/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <errno.h>
#include <unistd.h>
#include "server.h"
#include "client.h"
#include "busmonitor.h"
#include "connection.h"
#include "managementclient.h"
#include "groupcacheclient.h"
#include "config.h"
#include "flagpole.h"
#include "nonblockio.h"

ClientConnection::ClientConnection (Server * s, Layer3 * l3, Trace * tr,
				    int fd)
{
  TRACEPRINTF (tr, 8, this, "ClientConnection Init");
  this->fd = fd;
  this->t = tr;
  this->l3 = l3;
  this->s = s;
  buf = 0;
  buflen = 0;
}

ClientConnection::~ClientConnection ()
{
  TRACEPRINTF (t, 8, this, "ClientConnection closed");
  s->deregister (this);
  if (buf)
    delete[]buf;
  close (fd);
}

void
ClientConnection::Run (FlagpolePtr pole)
{
  while (!pole->raised (Flag_Stop))
    {
      if (readmessage (pole) == -1)
	break;
      int msg = EIBTYPE (buf);
      switch (msg)
	{
	case EIB_OPEN_BUSMONITOR:
	  {
	    A_Busmonitor busmon (this, l3, t, false, false);
	    busmon.Do (pole);
	  }
	  break;

	case EIB_OPEN_BUSMONITOR_TEXT:
	  {
	    A_Text_Busmonitor busmon (this, l3, t, false);
	    busmon.Do (pole);
	  }
	  break;

	case EIB_OPEN_BUSMONITOR_TS:
	  {
	    A_Busmonitor busmon (this, l3, t, false, true);
	    busmon.Do (pole);
	  }
	  break;

	case EIB_OPEN_VBUSMONITOR:
	  {
	    A_Busmonitor busmon (this, l3, t, true, false);
	    busmon.Do (pole);
	  }
	  break;

	case EIB_OPEN_VBUSMONITOR_TEXT:
	  {
	    A_Text_Busmonitor busmon (this, l3, t, true);
	    busmon.Do (pole);
	  }
	  break;

	case EIB_OPEN_VBUSMONITOR_TS:
	  {
	    A_Busmonitor busmon (this, l3, t, true, true);
	    busmon.Do (pole);
	  }
	  break;

	case EIB_OPEN_T_BROADCAST:
	  {
	    A_Broadcast cl (l3, t, this);
	    cl.Do (pole);
	  }
	  break;

	case EIB_OPEN_T_GROUP:
	  {
	    A_Group cl (l3, t, this);
	    cl.Do (pole);
	  }
	  break;

	case EIB_OPEN_T_INDIVIDUAL:
	  {
	    A_Individual cl (l3, t, this);
	    cl.Do (pole);
	  }
	  break;

	case EIB_OPEN_T_TPDU:
	  {
	    A_TPDU cl (l3, t, this);
	    cl.Do (pole);
	  }
	  break;

	case EIB_OPEN_T_CONNECTION:
	  {
	    A_Connection cl (l3, t, this);
	    cl.Do (pole);
	  }
	  break;

	case EIB_OPEN_GROUPCON:
	  {
	    A_GroupSocket cl (l3, t, this);
	    cl.Do (pole);
	  }
	  break;

	case EIB_M_INDIVIDUAL_ADDRESS_READ:
	  ReadIndividualAddresses (l3, t, this, pole);
	  break;

	case EIB_PROG_MODE:
	  ChangeProgMode (l3, t, this, pole);
	  break;

	case EIB_MASK_VERSION:
	  GetMaskVersion (l3, t, this, pole);
	  break;

	case EIB_M_INDIVIDUAL_ADDRESS_WRITE:
	  WriteIndividualAddress (l3, t, this, pole);
	  break;

	case EIB_MC_CONNECTION:
	  ManagementConnection (l3, t, this, pole);
	  break;

	case EIB_MC_INDIVIDUAL:
	  ManagementIndividual (l3, t, this, pole);
	  break;

	case EIB_LOAD_IMAGE:
	  LoadImage (l3, t, this, pole);
	  break;

	case EIB_CACHE_ENABLE:
	case EIB_CACHE_DISABLE:
	case EIB_CACHE_CLEAR:
	case EIB_CACHE_REMOVE:
	case EIB_CACHE_READ:
	case EIB_CACHE_READ_NOWAIT:
	case EIB_CACHE_LAST_UPDATES:
#ifdef HAVE_GROUPCACHE
	  GroupCacheRequest (l3, t, this, pole);
#else
	  sendreject (pole);
#endif
	  break;

	case EIB_RESET_CONNECTION:
	  sendreject (pole, EIB_RESET_CONNECTION);
	  EIBSETTYPE (buf, EIB_INVALID_REQUEST);
	  break;

	default:
	  sendreject (pole);
	}
      if (EIBTYPE (buf) == EIB_RESET_CONNECTION)
	sendreject (pole, EIB_RESET_CONNECTION);
    }
  StopDelete ();
}

int
ClientConnection::sendreject (FlagpolePtr pole)
{
  uchar buf[2];
  EIBSETTYPE (buf, EIB_INVALID_REQUEST);
  return sendmessage (2, buf, pole);
}

int
ClientConnection::sendreject (FlagpolePtr pole, int type)
{
  uchar buf[2];
  EIBSETTYPE (buf, type);
  return sendmessage (2, buf, pole);
}

int
ClientConnection::sendmessage (int size, const uchar * msg, FlagpolePtr pole)
{
  int i;
  int start;
  uchar head[2];
  assert (size >= 2);

  t->TracePacket (8, this, "SendMessage", size, msg);
  head[0] = (size >> 8) & 0xff;
  head[1] = (size) & 0xff;

  i = FlagpoleIO::write (pole, fd, head, 2);
  if (i != 2)
    return -1;

  start = 0;
lp:
  i = FlagpoleIO::write (pole, fd, msg + start, size - start);
  if (i <= 0)
    return -1;
  start += i;
  if (start < size)
    goto lp;
  return 0;
}

int
ClientConnection::readmessage (FlagpolePtr pole)
{
  uchar head[2];
  int i;
  unsigned start;

  i = FlagpoleIO::read (pole, fd, &head, 2);
  if (i != 2)
    return -1;

  size = (head[0] << 8) | (head[1]);
  if (size < 2)
    return -1;

  if (size > buflen)
    {
      if (buf)
	delete[]buf;
      buf = new uchar[size];
      buflen = size;
    }

  start = 0;
lp:
  i = FlagpoleIO::read (pole, fd, buf + start, size - start);
  if (i <= 0)
    return -1;
  start += i;
  if (start < size)
    goto lp;

  t->TracePacket (8, this, "RecvMessage", size, buf);
  return 0;
}
