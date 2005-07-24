/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "eibnetrouter.h"
#include "emi.h"

EIBNetIPRouter::EIBNetIPRouter (const char *multicastaddr, int port,
				eibaddr_t a, Trace * tr)
{
  struct sockaddr_in baddr;
  struct ip_mreq mcfg;
  t = tr;
  t->TracePrintf (2, this, "Open");
  addr = a;
  mode = 0;
  vmode = 0;
  memset (&baddr, 0, sizeof (baddr));
  baddr.sin_family = AF_INET;
  baddr.sin_port = htons (port);
  baddr.sin_addr.s_addr = htonl (INADDR_ANY);
  sock = new EIBNetIPSocket (baddr, 1, t);
  sock->recvall = 1;
  if (GetHostIP (&sock->sendaddr, multicastaddr) == 0)
    throw Exception (DEV_OPEN_FAIL);
  sock->sendaddr.sin_port = htons (port);

  mcfg.imr_multiaddr = sock->sendaddr.sin_addr;
  mcfg.imr_interface.s_addr = htonl (INADDR_ANY);
  sock->SetMulticast (mcfg);
  pth_sem_init (&out_signal);
  getwait = pth_event (PTH_EVENT_SEM, &out_signal);
  Start ();
  t->TracePrintf (2, this, "Opened");
}

EIBNetIPRouter::~EIBNetIPRouter ()
{
  t->TracePrintf (2, this, "Destroy");
  Stop ();
  pth_event_free (getwait, PTH_FREE_THIS);
  while (!outqueue.isempty ())
    delete outqueue.get ();
  delete sock;
}

void
EIBNetIPRouter::Send_L_Data (LPDU * l)
{
  t->TracePrintf (2, this, "Send %s", l->Decode ()());
  if (l->getType () != L_Data)
    {
      delete l;
      return;
    }
  L_Data_PDU *l1 = (L_Data_PDU *) l;
  EIBNetIPPacket p;
  p.data = L_Data_ToCEMI (0x29, *l1);
  p.service = ROUTING_INDICATION;
  sock->Send (p);
  delete l;
}

LPDU *
EIBNetIPRouter::Get_L_Data (pth_event_t stop)
{
  if (stop != NULL)
    pth_event_concat (getwait, stop, NULL);

  pth_wait (getwait);

  if (stop)
    pth_event_isolate (getwait);

  if (pth_event_status (getwait) == PTH_STATUS_OCCURRED)
    {
      pth_sem_dec (&out_signal);
      LPDU *l = outqueue.get ();
      t->TracePrintf (2, this, "Recv %s", l->Decode ()());
      return l;
    }
  else
    return 0;
}


void
EIBNetIPRouter::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      EIBNetIPPacket *p = sock->Get (stop);
      if (p)
	{
	  if (p->service != ROUTING_INDICATION)
	    {
	      delete p;
	      continue;
	    }
	  if (p->data () < 2 || p->data[0] != 0x29)
	    {
	      delete p;
	      continue;
	    }
	  const CArray data = p->data;
	  delete p;
	  L_Data_PDU *c = CEMI_to_L_Data (data);
	  if (c)
	    {
	      t->TracePrintf (2, this, "Recv %s", c->Decode ()());
	      if (mode == 0)
		{
		  if (vmode)
		    {
		      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
		      l2->pdu.set (c->ToPacket ());
		      outqueue.put (l2);
		      pth_sem_inc (&out_signal, 1);
		    }
		  outqueue.put (c);
		  pth_sem_inc (&out_signal, 1);
		  continue;
		}
	      L_Busmonitor_PDU *p1 = new L_Busmonitor_PDU;
	      p1->pdu = c->ToPacket ();
	      delete c;
	      outqueue.put (p1);
	      pth_sem_inc (&out_signal, 1);
	      continue;
	    }
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}

bool
EIBNetIPRouter::addAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::addGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::removeAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::removeGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::openVBusmonitor ()
{
  vmode = 1;
  return 1;
}

bool
EIBNetIPRouter::closeVBusmonitor ()
{
  vmode = 0;
  return 1;
}

bool
EIBNetIPRouter::enterBusmonitor ()
{
  mode = 1;
  return 1;
}

bool
EIBNetIPRouter::leaveBusmonitor ()
{
  mode = 0;
  return 1;
}

bool
EIBNetIPRouter::Open ()
{
  mode = 0;
  return 1;
}

bool
EIBNetIPRouter::Close ()
{
  return 1;
}

eibaddr_t
EIBNetIPRouter::getDefaultAddr ()
{
  return addr;
}

bool
EIBNetIPRouter::Connection_Lost ()
{
  return 0;
}

bool
EIBNetIPRouter::Send_Queue_Empty ()
{
  return 1;
}
