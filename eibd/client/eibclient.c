/*
    EIBD client library
    Copyright (C) 2005-2007 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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

#include "eibclient.h"
#include "eibclient-int.h"

int
EIB_Poll_Complete (EIBConnection * con)
{
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  if (_EIB_CheckRequest (con, 0) == -1)
    return -1;
  return (con->readlen >= 2 && con->readlen >= con->size + 2) ? 1 : 0;
}

int
EIB_Poll_FD (EIBConnection * con)
{
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  return con->fd;
}

int
EIBComplete (EIBConnection * con)
{
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  return con->complete (con);
}

static int
OpenBusmonitor_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) == EIB_CONNECTION_INUSE)
    {
      errno = EBUSY;
      return -1;
    }
  if (EIBTYPE (con) != EIB_OPEN_BUSMONITOR)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenBusmonitor_async (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_BUSMONITOR);
  i = _EIB_SendRequest (con, 2, head);
  if (i == -1)
    return -1;

  con->complete = OpenBusmonitor_complete;
  return 0;
}

int
EIBOpenBusmonitor (EIBConnection * con)
{
  if (EIBOpenBusmonitor_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenBusmonitorText_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) == EIB_CONNECTION_INUSE)
    {
      errno = EBUSY;
      return -1;
    }
  if (EIBTYPE (con) != EIB_OPEN_BUSMONITOR_TEXT)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenBusmonitorText_async (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_BUSMONITOR_TEXT);
  i = _EIB_SendRequest (con, 2, head);
  if (i == -1)
    return -1;
  con->complete = OpenBusmonitorText_complete;
  return 0;
}

int
EIBOpenBusmonitorText (EIBConnection * con)
{
  if (EIBOpenBusmonitorText_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenVBusmonitor_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) == EIB_CONNECTION_INUSE)
    {
      errno = EBUSY;
      return -1;
    }
  if (EIBTYPE (con) != EIB_OPEN_VBUSMONITOR)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenVBusmonitor_async (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_VBUSMONITOR);
  i = _EIB_SendRequest (con, 2, head);
  if (i == -1)
    return -1;
  con->complete = OpenVBusmonitor_complete;
  return 0;
}

int
EIBOpenVBusmonitor (EIBConnection * con)
{
  if (EIBOpenVBusmonitor_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenVBusmonitorText_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) == EIB_CONNECTION_INUSE)
    {
      errno = EBUSY;
      return -1;
    }
  if (EIBTYPE (con) != EIB_OPEN_VBUSMONITOR_TEXT)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpenVBusmonitorText_async (EIBConnection * con)
{
  uchar head[2];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_VBUSMONITOR_TEXT);
  i = _EIB_SendRequest (con, 2, head);
  if (i == -1)
    return -1;
  con->complete = OpenVBusmonitorText_complete;
  return 0;
}

int
EIBOpenVBusmonitorText (EIBConnection * con)
{
  if (EIBOpenVBusmonitorText_async (con) == -1)
    return -1;
  return EIBComplete (con);
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

  i = _EIB_GetRequest (con);
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

static int
OpenT_Connection_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIBOpenT_Connection_async (EIBConnection * con, eibaddr_t dest)
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
  i = _EIB_SendRequest (con, 5, head);
  if (i == -1)
    return -1;
  con->complete = OpenT_Connection_complete;
  return 0;
}

int
EIBOpenT_Connection (EIBConnection * con, eibaddr_t dest)
{
  if (EIBOpenT_Connection_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenT_TPDU_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIBOpenT_TPDU_async (EIBConnection * con, eibaddr_t src)
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
  i = _EIB_SendRequest (con, 5, head);
  if (i == -1)
    return -1;
  con->complete = OpenT_TPDU_complete;
  return 0;
}

int
EIBOpenT_TPDU (EIBConnection * con, eibaddr_t src)
{
  if (EIBOpenT_TPDU_async (con, src) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenT_Individual_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIBOpenT_Individual_async (EIBConnection * con, eibaddr_t dest,
			   int write_only)
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
  i = _EIB_SendRequest (con, 5, head);
  if (i == -1)
    return -1;
  con->complete = OpenT_Individual_complete;
  return 0;
}

int
EIBOpenT_Individual (EIBConnection * con, eibaddr_t dest, int write_only)
{
  if (EIBOpenT_Individual_async (con, dest, write_only) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenT_Group_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIBOpenT_Group_async (EIBConnection * con, eibaddr_t dest, int write_only)
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
  i = _EIB_SendRequest (con, 5, head);
  if (i == -1)
    return -1;
  con->complete = OpenT_Group_complete;
  return 0;
}

int
EIBOpenT_Group (EIBConnection * con, eibaddr_t dest, int write_only)
{
  if (EIBOpenT_Group_async (con, dest, write_only) == -1)
    return -1;
  return EIBComplete (con);
}

static int
OpenT_Broadcast_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIBOpenT_Broadcast_async (EIBConnection * con, int write_only)
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
  i = _EIB_SendRequest (con, 5, head);
  if (i == -1)
    return -1;
  con->complete = OpenT_Broadcast_complete;
  return 0;
}

int
EIBOpenT_Broadcast (EIBConnection * con, int write_only)
{
  if (EIBOpenT_Broadcast_async (con, write_only) == -1)
    return -1;
  return EIBComplete (con);
}

int
EIBSendTPDU (EIBConnection * con, eibaddr_t dest, int len, uint8_t * data)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
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
  i = _EIB_SendRequest (con, len + 4, ibuf);
  free (ibuf);
  return i;
}

int
EIBSendAPDU (EIBConnection * con, int len, uint8_t * data)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
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
  i = _EIB_SendRequest (con, len + 2, ibuf);
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

  i = _EIB_GetRequest (con);
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

  i = _EIB_GetRequest (con);
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

static int
Open_GroupSocket_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_OPEN_GROUPCON)
    {
      errno = ECONNRESET;
      return -1;
    }
  return 0;
}

int
EIBOpen_GroupSocket_async (EIBConnection * con, int write_only)
{
  uchar head[5];
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_OPEN_GROUPCON);
  head[4] = (write_only ? 0xff : 0);
  i = _EIB_SendRequest (con, 5, head);
  if (i == -1)
    return -1;
  con->complete = Open_GroupSocket_complete;
  return 0;
}

int
EIBOpen_GroupSocket (EIBConnection * con, int write_only)
{
  if (EIBOpen_GroupSocket_async (con, write_only) == -1)
    return -1;
  return EIBComplete (con);
}

int
EIBGetGroup_Src (EIBConnection * con, int maxlen, uint8_t * buf,
		 eibaddr_t * src, eibaddr_t * dest)
{
  int i;
  if (!con || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_GROUP_PACKET || con->size < 6)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 6;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 6, i);
  if (src)
    *src = (con->buf[2] << 8) | (con->buf[3]);
  if (dest)
    *dest = (con->buf[4] << 8) | (con->buf[5]);
  return i;
}

int
EIBSendGroup (EIBConnection * con, eibaddr_t dest, int len, uint8_t * data)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
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
  EIBSETTYPE (ibuf, EIB_GROUP_PACKET);
  EIBSETADDR (ibuf + 2, dest);
  memcpy (ibuf + 4, data, len);
  i = _EIB_SendRequest (con, len + 4, ibuf);
  free (ibuf);
  return i;
}

static int
M_ReadIndividualAddresses_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_M_INDIVIDUAL_ADDRESS_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > con->req.len)
    i = con->req.len;
  memcpy (con->req.buf, con->buf + 2, i);
  return i;
}

int
EIB_M_ReadIndividualAddresses_async (EIBConnection * con, int maxlen,
				     uint8_t * buf)
{
  uchar head[2];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = maxlen;
  con->req.buf = buf;
  EIBSETTYPE (head, EIB_M_INDIVIDUAL_ADDRESS_READ);
  if (_EIB_SendRequest (con, 2, head) == -1)
    return -1;
  con->complete = M_ReadIndividualAddresses_complete;
  return 0;
}

int
EIB_M_ReadIndividualAddresses (EIBConnection * con, int maxlen, uint8_t * buf)
{
  if (EIB_M_ReadIndividualAddresses_async (con, maxlen, buf) == -1)
    return -1;
  return EIBComplete (con);
}

static int
M_Progmode_On_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_M_Progmode_On_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 1;
  if (_EIB_SendRequest (con, 5, head) == -1)
    return -1;
  con->complete = M_Progmode_On_complete;
  return 0;
}

int
EIB_M_Progmode_On (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_Progmode_On_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
M_Progmode_Off_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_M_Progmode_Off_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 0;
  if (_EIB_SendRequest (con, 5, head) == -1)
    return -1;
  con->complete = M_Progmode_Off_complete;
  return 0;
}

int
EIB_M_Progmode_Off (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_Progmode_Off_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
M_Progmode_Toggle_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_M_Progmode_Toggle_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 2;
  if (_EIB_SendRequest (con, 5, head) == -1)
    return -1;
  con->complete = M_Progmode_Toggle_complete;
  return 0;
}

int
EIB_M_Progmode_Toggle (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_Progmode_Toggle (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
M_Progmode_Status_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_M_Progmode_Status_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[5];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_PROG_MODE);
  EIBSETADDR (head + 2, dest);
  head[4] = 3;
  if (_EIB_SendRequest (con, 5, head) == -1)
    return -1;
  con->complete = M_Progmode_Status_complete;
  return 0;
}

int
EIB_M_Progmode_Status (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_Progmode_Status (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
M_GetMaskVersion_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_M_GetMaskVersion_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[4];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MASK_VERSION);
  EIBSETADDR (head + 2, dest);
  if (_EIB_SendRequest (con, 4, head) == -1)
    return -1;
  con->complete = M_GetMaskVersion_complete;
  return 0;
}

int
EIB_M_GetMaskVersion (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_GetMaskVersion_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
M_WriteIndividualAddress_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_M_WriteIndividualAddress_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[4];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_M_INDIVIDUAL_ADDRESS_WRITE);
  EIBSETADDR (head + 2, dest);
  if (_EIB_SendRequest (con, 4, head) == -1)
    return -1;
  con->complete = M_WriteIndividualAddress_complete;
  return 0;
}

int
EIB_M_WriteIndividualAddress (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_WriteIndividualAddress_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Connect_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_Connect_async (EIBConnection * con, eibaddr_t dest)
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
  i = _EIB_SendRequest (con, 4, head);
  if (i == -1)
    return -1;

  con->complete = MC_Connect_complete;
  return 0;
}

int
EIB_MC_Connect (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_MC_Connect_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Progmode_On_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_Progmode_On_async (EIBConnection * con)
{
  uchar head[3];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 1;
  if (_EIB_SendRequest (con, 3, head) == -1)
    return -1;
  con->complete = MC_Progmode_On_complete;
  return 0;
}

int
EIB_MC_Progmode_On (EIBConnection * con)
{
  if (EIB_MC_Progmode_On_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Progmode_Off_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_Progmode_Off_async (EIBConnection * con)
{
  uchar head[3];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 0;
  if (_EIB_SendRequest (con, 3, head) == -1)
    return -1;
  con->complete = MC_Progmode_Off_complete;
  return 0;
}

int
EIB_MC_Progmode_Off (EIBConnection * con)
{
  if (EIB_MC_Progmode_Off_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Progmode_Toggle_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_Progmode_Toggle_async (EIBConnection * con)
{
  uchar head[3];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 2;
  if (_EIB_SendRequest (con, 3, head) == -1)
    return -1;
  con->complete = MC_Progmode_Toggle_complete;
  return 0;
}

int
EIB_MC_Progmode_Toggle (EIBConnection * con)
{
  if (EIB_MC_Progmode_Toggle_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Progmode_Status_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_Progmode_Status_async (EIBConnection * con)
{
  uchar head[3];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PROG_MODE);
  head[2] = 3;
  if (_EIB_SendRequest (con, 3, head) == -1)
    return -1;
  con->complete = MC_Progmode_Status_complete;
  return 0;
}

int
EIB_MC_Progmode_Status (EIBConnection * con)
{
  if (EIB_MC_Progmode_Status_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_GetMaskVersion_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_GetMaskVersion_async (EIBConnection * con)
{
  uchar head[2];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_MASK_VERSION);
  if (_EIB_SendRequest (con, 2, head) == -1)
    return -1;
  con->complete = MC_GetMaskVersion_complete;
  return 0;
}

int
EIB_MC_GetMaskVersion (EIBConnection * con)
{
  if (EIB_MC_GetMaskVersion_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_GetPEIType_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_GetPEIType_async (EIBConnection * con)
{
  uchar head[2];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PEI_TYPE);
  if (_EIB_SendRequest (con, 2, head) == -1)
    return -1;
  con->complete = MC_GetPEIType_complete;
  return 0;
}

int
EIB_MC_GetPEIType (EIBConnection * con)
{
  if (EIB_MC_GetPEIType_async (con) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_ReadADC_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_ADC_READ || con->size < 4)
    {
      errno = ECONNRESET;
      return -1;
    }
  if (con->req.ptr1)
    *con->req.ptr1 = (con->buf[2] << 8) | (con->buf[3]);
  return 0;
}

int
EIB_MC_ReadADC_async (EIBConnection * con, uint8_t channel, uint8_t count,
		      int16_t * val)
{
  uchar head[4];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.ptr1 = val;
  EIBSETTYPE (head, EIB_MC_ADC_READ);
  head[2] = channel;
  head[3] = count;
  if (_EIB_SendRequest (con, 4, head) == -1)
    return -1;
  con->complete = MC_ReadADC_complete;
  return 0;
}

int
EIB_MC_ReadADC (EIBConnection * con, uint8_t channel, uint8_t count,
		int16_t * val)
{
  if (EIB_MC_ReadADC_async (con, channel, count, val) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_PropertyRead_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > con->req.len)
    i = con->req.len;
  memcpy (con->req.buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_PropertyRead_async (EIBConnection * con, uint8_t obj, uint8_t property,
			   uint16_t start, uint8_t nr_of_elem, int max_len,
			   uint8_t * buf)
{
  uchar head[7];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.buf = buf;
  con->req.len = max_len;
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
  if (_EIB_SendRequest (con, 7, head) == -1)
    return -1;
  con->complete = MC_PropertyRead_complete;
  return 0;
}

int
EIB_MC_PropertyRead (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint16_t start, uint8_t nr_of_elem, int max_len,
		     uint8_t * buf)
{
  if (EIB_MC_PropertyRead_async
      (con, obj, property, start, nr_of_elem, max_len, buf) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Read_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > con->req.len)
    i = con->req.len;
  memcpy (con->req.buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_Read_async (EIBConnection * con, uint16_t addr, int len, uint8_t * buf)
{
  uchar head[6];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = len;
  con->req.buf = buf;
  EIBSETTYPE (head, EIB_MC_READ);
  head[2] = (addr >> 8) & 0xff;
  head[3] = (addr) & 0xff;
  head[4] = (len >> 8) & 0xff;
  head[5] = (len) & 0xff;
  if (_EIB_SendRequest (con, 6, head) == -1)
    return -1;
  con->complete = MC_Read_complete;
  return 0;
}

int
EIB_MC_Read (EIBConnection * con, uint16_t addr, int len, uint8_t * buf)
{
  if (EIB_MC_Read_async (con, addr, len, buf) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_PropertyWrite_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_WRITE)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > con->req.len)
    i = con->req.len;
  memcpy (con->req.buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_PropertyWrite_async (EIBConnection * con, uint8_t obj,
			    uint8_t property, uint16_t start,
			    uint8_t nr_of_elem, int len, const uint8_t * buf,
			    int max_len, uint8_t * res)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  if (!buf || !res)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = max_len;
  con->req.buf = res;
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
  i = _EIB_SendRequest (con, len + 7, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  con->complete = MC_PropertyWrite_complete;
  return 0;
}

int
EIB_MC_PropertyWrite (EIBConnection * con, uint8_t obj, uint8_t property,
		      uint16_t start, uint8_t nr_of_elem, int len,
		      const uint8_t * buf, int max_len, uint8_t * res)
{
  if (EIB_MC_PropertyWrite_async
      (con, obj, property, start, nr_of_elem, len, buf, max_len, res) == -1)
    return -1;
  return EIBComplete (con);
}
static int
MC_Write_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) == EIB_ERROR_VERIFY)
    {
      errno = EIO;
      return -1;
    }
  if (EIBTYPE (con) != EIB_MC_WRITE)
    {
      errno = ECONNRESET;
      return -1;
    }
  return con->req.len;
}

int
EIB_MC_Write_async (EIBConnection * con, uint16_t addr, int len,
		    const uint8_t * buf)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = len;
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
  i = _EIB_SendRequest (con, len + 6, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  con->complete = MC_Write_complete;
  return 0;
}

int
EIB_MC_Write (EIBConnection * con, uint16_t addr, int len,
	      const uint8_t * buf)
{
  if (EIB_MC_Write_async (con, addr, len, buf) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_PropertyDesc_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_DESC || con->size < 6)
    {
      errno = ECONNRESET;
      return -1;
    }
  /* Type */
  if (con->req.ptr2)
    *con->req.ptr2 = con->buf[2];
  /* max_nr_of_elem */
  if (con->req.ptr4)
    *con->req.ptr4 = (con->buf[3] << 8) | (con->buf[4]);
  /* access */
  if (con->req.ptr3)
    *con->req.ptr3 = con->buf[5];
  return 0;
}

int
EIB_MC_PropertyDesc_async (EIBConnection * con, uint8_t obj, uint8_t property,
			   uint8_t * type, uint16_t * max_nr_of_elem,
			   uint8_t * access)
{
  uchar head[5];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.ptr2 = type;
  con->req.ptr4 = max_nr_of_elem;
  con->req.ptr3 = access;
  EIBSETTYPE (head, EIB_MC_PROP_DESC);
  head[2] = obj;
  head[3] = property;
  if (_EIB_SendRequest (con, 4, head) == -1)
    return -1;
  con->complete = MC_PropertyDesc_complete;
  return 0;
}

int
EIB_MC_PropertyDesc (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint8_t * type, uint16_t * max_nr_of_elem,
		     uint8_t * access)
{
  if (EIB_MC_PropertyDesc_async
      (con, obj, property, type, max_nr_of_elem, access) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_Authorize_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_Authorize_async (EIBConnection * con, uint8_t key[4])
{
  uchar head[6];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_AUTHORIZE);
  memcpy (head + 2, key, 4);
  if (_EIB_SendRequest (con, 6, head) == -1)
    return -1;
  con->complete = MC_Authorize_complete;
  return 0;
}

int
EIB_MC_Authorize (EIBConnection * con, uint8_t key[4])
{
  if (EIB_MC_Authorize_async (con, key) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_SetKey_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
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
EIB_MC_SetKey_async (EIBConnection * con, uint8_t key[4], uint8_t level)
{
  uchar head[7];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_KEY_WRITE);
  memcpy (head + 2, key, 4);
  head[6] = level;
  if (_EIB_SendRequest (con, 7, head) == -1)
    return -1;
  con->complete = MC_SetKey_complete;
  return 0;
}

int
EIB_MC_SetKey (EIBConnection * con, uint8_t key[4], uint8_t level)
{
  if (EIB_MC_SetKey_async (con, key, level) == -1)
    return -1;
  return EIBComplete (con);
}

static int
MC_PropertyScan_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_SCAN)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > con->req.len)
    i = con->req.len;
  memcpy (con->req.buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_PropertyScan_async (EIBConnection * con, int maxlen, uint8_t * buf)
{
  uchar head[2];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = maxlen;
  con->req.buf = buf;
  EIBSETTYPE (head, EIB_MC_PROP_SCAN);
  if (_EIB_SendRequest (con, 2, head) == -1)
    return -1;
  con->complete = MC_PropertyScan_complete;
  return 0;
}

int
EIB_MC_PropertyScan (EIBConnection * con, int maxlen, uint8_t * buf)
{
  if (EIB_MC_PropertyScan_async (con, maxlen, buf) == -1)
    return -1;
  return EIBComplete (con);
}

static int
LoadImage_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_LOAD_IMAGE || con->size < 4)
    {
      errno = ECONNRESET;
      return IMG_UNKNOWN_ERROR;
    }
  return (con->buf[2] << 8) | con->buf[3];
}

int
EIB_LoadImage_async (EIBConnection * con, const uint8_t * image, int len)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
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
  i = _EIB_SendRequest (con, len + 2, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  con->complete = LoadImage_complete;
  return 0;
}

BCU_LOAD_RESULT
EIB_LoadImage (EIBConnection * con, const uint8_t * image, int len)
{
  if (EIB_LoadImage_async (con, image, len) == -1)
    return -1;
  return EIBComplete (con);
}
