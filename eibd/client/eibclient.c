/*
    EIBD client library
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License, 
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any 
    restriction coming from the use of this file. (The General Public 
    License restrictions do apply in other respects; for example, they 
    cover modification of the file, and distribution when not linked into 
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "eibclient.h"
#include "eibtypes.h"

/** unsigned char */
typedef uint8_t uchar;

/** EIB Connection internal */
struct _EIBConnection
{
  /** file descriptor */
  int fd;
  /** buffer */
  uchar *buf;
  /** buffer size */
  unsigned buflen;
  /** used buffer */
  unsigned size;
};

/** extracts TYPE code of an eibd packet */
#define EIBTYPE(con) (((con)->buf[0]<<8)|((con)->buf[1]))
/** sets TYPE code for an eibd packet*/
#define EIBSETTYPE(buf,type) do{(buf)[0]=(type>>8)&0xff;(buf)[1]=(type)&0xff;}while(0)

/** set EIB address */
#define EIBSETADDR(buf,type) do{(buf)[0]=(type>>8)&0xff;(buf)[1]=(type)&0xff;}while(0)

/** resolve host name */
static int
GetHostIP (struct sockaddr_in *sock, const char *Name)
{
  struct hostent *h;
  if (!Name)
    return 0;
  memset (sock, 0, sizeof (*sock));
  h = gethostbyname (Name);
  if (!h)
    return 0;
  sock->sin_family = h->h_addrtype;
  sock->sin_addr.s_addr = (*((unsigned long *) h->h_addr_list[0]));
  return 1;
}

int
EIBClose (EIBConnection * con)
{
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  close (con->fd);
  if (con->buf)
    free (con->buf);
  free (con);
  return 0;
}

EIBConnection *
EIBSocketLocal (const char *path)
{
  EIBConnection *con = (EIBConnection *) malloc (sizeof (EIBConnection));
  struct sockaddr_un addr;
  if (!con)
    {
      errno = ENOMEM;
      return 0;
    }
  addr.sun_family = AF_LOCAL;
  strncpy (addr.sun_path, path, sizeof (addr.sun_path));
  addr.sun_path[sizeof (addr.sun_path) - 1] = 0;

  con->fd = socket (AF_LOCAL, SOCK_STREAM, 0);
  if (con->fd == -1)
    {
      free (con);
      return 0;
    }

  if (connect (con->fd, (struct sockaddr *) &addr, sizeof (addr)) == -1)
    {
      int saveerr = errno;
      close (con->fd);
      free (con);
      errno = saveerr;
      return 0;
    }
  con->buflen = 0;
  con->buf = 0;

  return con;
}

EIBConnection *
EIBSocketRemote (const char *host, int port)
{
  EIBConnection *con = (EIBConnection *) malloc (sizeof (EIBConnection));
  struct sockaddr_in addr;
  if (!con)
    {
      errno = ENOMEM;
      return 0;
    }

  if (!GetHostIP (&addr, host))
    {
      free (con);
      errno = ECONNREFUSED;
      return 0;
    }
  addr.sin_port = htons (port);

  con->fd = socket (addr.sin_family, SOCK_STREAM, 0);
  if (con->fd == -1)
    {
      free (con);
      return 0;
    }

  if (connect (con->fd, (struct sockaddr *) &addr, sizeof (addr)) == -1)
    {
      int saveerr = errno;
      close (con->fd);
      free (con);
      errno = saveerr;
      return 0;
    }
  con->buflen = 0;
  con->buf = 0;

  return con;
}

EIBConnection *
EIBSocketURL (const char *url)
{
  if (!url)
    {
      errno = EINVAL;
      return 0;
    }
  if (!strncmp (url, "local:", 6))
    {
      return EIBSocketLocal (url + 6);
    }
  if (!strncmp (url, "ip:", 3))
    {
      char *a = strdup (url + 3);
      char *b;
      int port;
      EIBConnection *c;
      if (!a)
	{
	  errno = ENOMEM;
	  return 0;
	}
      for (b = a; *b; b++)
	if (*b == ':')
	  break;
      if (*b == ':')
	{
	  *b = 0;
	  port = atoi (b + 1);
	}
      else
	port = 6720;
      c = EIBSocketRemote (a, port);
      free (a);
      return c;
    }
  errno = EINVAL;
  return 0;
}

/** send a request to eibd */
static int
SendRequest (EIBConnection * con, unsigned int size, uchar * data)
{
  uchar head[2];
  int i, start;

  if (size > 0xffff || size < 2)
    {
      errno = EINVAL;
      return -1;
    }
  head[0] = (size >> 8) & 0xff;
  head[1] = (size) & 0xff;

lp1:
  i = write (con->fd, &head, 2);
  if (i == -1 && errno == EINTR)
    goto lp1;
  if (i == -1)
    return -1;
  if (i != 2)
    {
      errno = ECONNRESET;
      return -1;
    }
  start = 0;
lp2:
  i = write (con->fd, data + start, size - start);
  if (i == -1 && errno == EINTR)
    goto lp2;
  if (i == -1)
    return -1;
  if (i == 0)
    {
      errno = ECONNRESET;
      return -1;
    }
  start += i;
  if (start < size)
    goto lp2;
  return 0;
}

/** receive packet from eibd */
static int
GetRequest (EIBConnection * con)
{
  uchar head[2];
  int i, start, size;

lp1:
  i = read (con->fd, &head, 2);
  if (i == -1 && errno == EINTR)
    goto lp1;
  if (i == -1)
    return -1;
  if (i != 2)
    {
      errno = ECONNRESET;
      return -1;
    }

  size = (head[0] << 8) | (head[1]);
  if (size < 2)
    {
      errno = ECONNRESET;
      return -1;
    }

  if (size > con->buflen)
    {
      con->buf = (uchar *) realloc (con->buf, size);
      if (con->buf == 0)
	{
	  con->buflen = 0;
	  errno = ENOMEM;
	  return -1;
	}
      con->buflen = size;
    }

  start = 0;
lp2:
  i = read (con->fd, con->buf + start, size - start);
  if (i == -1 && errno == EINTR)
    goto lp2;
  if (i == -1)
    return -1;
  start += i;
  if (start < size)
    goto lp2;

  con->size = size;
  return 0;
}

int
EIBOpenBusmonitor (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_BUSMONITOR);
  i = SendRequest (con, 2, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_BUSMONITOR)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenBusmonitorText (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_BUSMONITOR_TEXT);
  i = SendRequest (con, 2, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_BUSMONITOR_TEXT)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenVBusmonitor (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_VBUSMONITOR);
  i = SendRequest (con, 2, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_VBUSMONITOR)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenVBusmonitorText (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_VBUSMONITOR_TEXT);
  i = SendRequest (con, 2, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_VBUSMONITOR_TEXT)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBGetBusmonitorPacket (EIBConnection * con, int maxlen, uint8_t * buf)
{
  int i;
  if (!con || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_BUSMONITOR_PACKET)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 2, i);
  return i;
}

int
EIBOpenT_Connection (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_T_CONNECTION);
  EIBSETADDR (head + 2, dest);
  i = SendRequest (con, 5, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_T_CONNECTION)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenT_TPDU (EIBConnection * con, eibaddr_t src)
{
  uchar head[5];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_T_TPDU);
  EIBSETADDR (head + 2, src);
  i = SendRequest (con, 5, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_T_TPDU)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenT_Individual (EIBConnection * con, eibaddr_t dest, int write_only)
{
  uchar head[5];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_T_INDIVIDUAL);
  EIBSETADDR (head + 2, dest);
  head[4] = (write_only ? 0xff : 0);
  i = SendRequest (con, 5, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_T_INDIVIDUAL)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenT_Group (EIBConnection * con, eibaddr_t dest, int write_only)
{
  uchar head[5];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_T_GROUP);
  EIBSETADDR (head + 2, dest);
  head[4] = (write_only ? 0xff : 0);
  i = SendRequest (con, 5, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_T_GROUP)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenT_Broadcast (EIBConnection * con, int write_only)
{
  uchar head[5];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_T_BROADCAST);
  head[4] = (write_only ? 0xff : 0);
  i = SendRequest (con, 5, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_T_BROADCAST)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBSendTPDU (EIBConnection * con, eibaddr_t dest, int len, uint8_t * data)
{
  uchar *ibuf;
  int i;
  if (len < 2 || !data)
    {
      errno = EINVAL;
      return -1;
    }
  ibuf = (uchar *) malloc (len + 4);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_APDU_PACKET);
  EIBSETADDR (ibuf + 2, dest);
  memcpy (ibuf + 4, data, len);
  i = SendRequest (con, len + 4, ibuf);
  free (ibuf);
  return i;
}

int
EIBSendAPDU (EIBConnection * con, int len, uint8_t * data)
{
  uchar *ibuf;
  int i;
  if (len < 2 || !data)
    {
      errno = EINVAL;
      return -1;
    }
  ibuf = (uchar *) malloc (len + 2);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_APDU_PACKET);
  memcpy (ibuf + 2, data, len);
  i = SendRequest (con, len + 2, ibuf);
  free (ibuf);
  return i;
}

int
EIBGetAPDU (EIBConnection * con, int maxlen, uint8_t * buf)
{
  int i;
  if (!con || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_APDU_PACKET)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 2, i);
  return i;
}

int
EIBGetAPDU_Src (EIBConnection * con, int maxlen, uint8_t * buf,
		eibaddr_t * src)
{
  int i;
  if (!con || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_APDU_PACKET || con->size < 4)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 4;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 4, i);
  if (src)
    *src = (con->buf[2] << 8) | (con->buf[3]);
  return i;
}

int
EIB_M_ReadIndividualAddresses (EIBConnection * con, int maxlen, uint8_t * buf)
{
  uchar head[2];
  int i;
  EIBSETTYPE (head, EIB_M_INDIVIDUAL_ADDRESS_READ);
  if (SendRequest (con, 2, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_M_INDIVIDUAL_ADDRESS_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 2, i);
  return i;
}

int
EIB_M_Progmode_On (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  int i;
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 1;
  if (SendRequest (con, 5, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_PROG_MODE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_M_Progmode_Off (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  int i;
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 0;
  if (SendRequest (con, 5, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_PROG_MODE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_M_Progmode_Toggle (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  int i;
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 2;
  if (SendRequest (con, 5, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_PROG_MODE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_M_Progmode_Status (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  int i;
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 3;
  if (SendRequest (con, 5, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_PROG_MODE || con->size < 3)
    {
      errno = ECONNRESET;
      return -1;
    }
  return con->buf[2];
}

int
EIB_M_GetMaskVersion (EIBConnection * con, eibaddr_t dest)
{
  uchar head[4];
  int i;
  EIBSETTYPE (head, EIB_MASK_VERSION);
  EIBSETADDR (head + 2, dest);
  if (SendRequest (con, 4, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MASK_VERSION || con->size < 4)
    {
      errno = ECONNRESET;
      return -1;
    }
  return (con->buf[2] << 8) | (con->buf[3]);
}

int
EIB_M_WriteIndividualAddress (EIBConnection * con, eibaddr_t dest)
{
  uchar head[4];
  int i;
  EIBSETTYPE (head, EIB_M_INDIVIDUAL_ADDRESS_WRITE);
  EIBSETADDR (head + 2, dest);
  if (SendRequest (con, 4, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) == EIB_ERROR_ADDR_EXISTS)
    {
      errno = EADDRINUSE;
      return -1;
    }
  if (EIBTYPE (con) == EIB_M_INDIVIDUAL_ADDRESS_WRITE)
    {
      return 0;
    }
  if (EIBTYPE (con) == EIB_ERROR_TIMEOUT)
    {
      errno = ETIMEDOUT;
      return -1;
    }
  if (EIBTYPE (con) == EIB_ERROR_MORE_DEVICE)
    {
      errno = EADDRNOTAVAIL;
      return -1;
    }
  errno = ECONNRESET;
  return -1;
}


int
EIB_MC_Connect (EIBConnection * con, eibaddr_t dest)
{
  uchar head[4];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_CONNECTION);
  EIBSETADDR (head + 2, dest);
  i = SendRequest (con, 4, head);
  if (i == -1)
    return -1;

  i = GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_MC_CONNECTION)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_MC_Progmode_On (EIBConnection * con)
{
  uchar head[3];
  int i;
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 1;
  if (SendRequest (con, 3, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROG_MODE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_MC_Progmode_Off (EIBConnection * con)
{
  uchar head[3];
  int i;
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 0;
  if (SendRequest (con, 3, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROG_MODE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_MC_Progmode_Toggle (EIBConnection * con)
{
  uchar head[3];
  int i;
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 2;
  if (SendRequest (con, 3, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROG_MODE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_MC_Progmode_Status (EIBConnection * con)
{
  uchar head[3];
  int i;
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 3;
  if (SendRequest (con, 3, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROG_MODE || con->size < 3)
    {
      errno = ECONNRESET;
      return -1;
    }
  return con->buf[2];
}

int
EIB_MC_GetMaskVersion (EIBConnection * con)
{
  uchar head[2];
  int i;
  EIBSETTYPE (head, EIB_MC_MASK_VERSION);
  if (SendRequest (con, 2, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_MASK_VERSION || con->size < 4)
    {
      errno = ECONNRESET;
      return -1;
    }
  return (con->buf[2] << 8) | (con->buf[3]);
}

int
EIB_MC_GetPEIType (EIBConnection * con)
{
  uchar head[2];
  int i;
  EIBSETTYPE (head, EIB_MC_PEI_TYPE);
  if (SendRequest (con, 2, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PEI_TYPE || con->size < 4)
    {
      errno = ECONNRESET;
      return -1;
    }
  return (con->buf[2] << 8) | (con->buf[3]);
}

int
EIB_MC_ReadADC (EIBConnection * con, uint8_t channel, uint8_t count,
		int16_t * val)
{
  uchar head[4];
  int i;
  EIBSETTYPE (head, EIB_MC_ADC_READ);
  head[2] = channel;
  head[3] = count;
  if (SendRequest (con, 4, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_ADC_READ || con->size < 4)
    {
      errno = ECONNRESET;
      return -1;
    }
  if (val)
    *val = (con->buf[2] << 8) | (con->buf[3]);
  return 0;
}

int
EIB_MC_PropertyRead (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint16_t start, uint8_t nr_of_elem, int max_len,
		     uint8_t * buf)
{
  uchar head[7];
  int i;
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PROP_READ);
  head[2] = obj;
  head[3] = property;
  head[4] = (start >> 8) & 0xff;
  head[5] = (start) & 0xff;
  head[6] = nr_of_elem;
  if (SendRequest (con, 7, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > max_len)
    i = max_len;
  memcpy (buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_Read (EIBConnection * con, uint16_t addr, int len, uint8_t * buf)
{
  uchar head[6];
  int i;
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_READ);
  head[2] = (addr >> 8) & 0xff;
  head[3] = (addr) & 0xff;
  head[4] = (len >> 8) & 0xff;
  head[5] = (len) & 0xff;
  if (SendRequest (con, 6, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > len)
    i = len;
  memcpy (buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_PropertyWrite (EIBConnection * con, uint8_t obj, uint8_t property,
		      uint16_t start, uint8_t nr_of_elem, int len,
		      const uint8_t * buf, int max_len, uint8_t * res)
{
  uchar *ibuf;
  int i;
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  ibuf = (uchar *) malloc (len + 7);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_MC_PROP_WRITE);
  ibuf[2] = obj;
  ibuf[3] = property;
  ibuf[4] = (start >> 8) & 0xff;
  ibuf[5] = (start) & 0xff;
  ibuf[6] = nr_of_elem;
  memcpy (ibuf + 7, buf, len);
  i = SendRequest (con, len + 7, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_WRITE)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > max_len)
    i = max_len;
  memcpy (res, con->buf + 2, i);
  return i;
}

int
EIB_MC_Write (EIBConnection * con, uint16_t addr, int len,
	      const uint8_t * buf)
{
  uchar *ibuf;
  int i;
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  ibuf = (uchar *) malloc (len + 6);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_MC_WRITE);
  ibuf[2] = (addr >> 8) & 0xff;
  ibuf[3] = (addr) & 0xff;
  ibuf[4] = (len >> 8) & 0xff;
  ibuf[5] = (len) & 0xff;
  memcpy (ibuf + 6, buf, len);
  i = SendRequest (con, len + 6, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_WRITE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return len;
}


int
EIB_MC_PropertyDesc (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint8_t * type, uint16_t * max_nr_of_elem,
		     uint8_t * access)
{
  uchar head[5];
  int i;
  EIBSETTYPE (head, EIB_MC_PROP_DESC);
  head[2] = obj;
  head[3] = property;
  if (SendRequest (con, 4, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_DESC || con->size < 6)
    {
      errno = ECONNRESET;
      return -1;
    }
  if (type)
    *type = con->buf[2];
  if (max_nr_of_elem)
    *max_nr_of_elem = (con->buf[3] << 8) | (con->buf[4]);
  if (access)
    *access = con->buf[5];
  return 0;
}

int
EIB_MC_Authorize (EIBConnection * con, uint8_t key[4])
{
  uchar head[6];
  int i;
  EIBSETTYPE (head, EIB_MC_AUTHORIZE);
  memcpy (head + 2, key, 4);
  if (SendRequest (con, 6, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_AUTHORIZE || con->size < 3)
    {
      errno = ECONNRESET;
      return -1;
    }
  return con->buf[2];
}

int
EIB_MC_SetKey (EIBConnection * con, uint8_t key[4], uint8_t level)
{
  uchar head[7];
  int i;
  EIBSETTYPE (head, EIB_MC_KEY_WRITE);
  memcpy (head + 2, key, 4);
  head[6] = level;
  if (SendRequest (con, 7, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) == EIB_PROCESSING_ERROR)
    {
      errno = EPERM;
      return -1;
    }
  if (EIBTYPE (con) != EIB_MC_KEY_WRITE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIB_MC_PropertyScan (EIBConnection * con, int maxlen, uint8_t * buf)
{
  uchar head[2];
  int i;
  EIBSETTYPE (head, EIB_MC_PROP_SCAN);
  if (SendRequest (con, 2, head) == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_SCAN)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 2, i);
  return i;
}

BCU_LOAD_RESULT
EIB_LoadImage (EIBConnection * con, const uint8_t * image, int len)
{
  uchar *ibuf;
  int i;
  if (!image)
    {
      errno = EINVAL;
      return -1;
    }
  ibuf = (uchar *) malloc (len + 2);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_LOAD_IMAGE);
  memcpy (ibuf + 2, image, len);
  i = SendRequest (con, len + 2, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  i = GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_LOAD_IMAGE || con->size < 4)
    {
      errno = ECONNRESET;
      return IMG_UNKNOWN_ERROR;
    }
  return (con->buf[2] << 8) | con->buf[3];
}
