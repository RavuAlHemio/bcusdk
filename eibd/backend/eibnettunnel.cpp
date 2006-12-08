/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2006 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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

#include "eibnettunnel.h"
#include "emi.h"

bool
EIBNetIPTunnel::addAddress (eibaddr_t addr)
{
  return 0;
}

bool
EIBNetIPTunnel::removeAddress (eibaddr_t addr)
{
  return 0;
}

bool
EIBNetIPTunnel::addGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPTunnel::removeGroupAddress (eibaddr_t addr)
{
  return 1;
}

eibaddr_t
EIBNetIPTunnel::getDefaultAddr ()
{
  return 0;
}

EIBNetIPTunnel::EIBNetIPTunnel (const char *dest, int port, int sport,
				Trace * tr)
{
  t = tr;
  TRACEPRINTF (t, 2, this, "Open");
  if (!GetHostIP (&caddr, dest))
    throw Exception (DEV_OPEN_FAIL);
  caddr.sin_port = htons (port);
  if (!GetSourceAddress (&caddr, &saddr))
    throw Exception (DEV_OPEN_FAIL);
  saddr.sin_port = htons (sport);
  sock = new EIBNetIPSocket (saddr, 0, t);
  sock->sendaddr = caddr;
  sock->recvaddr = caddr;
  pth_sem_init (&insignal);
  pth_sem_init (&outsignal);
  getwait = pth_event (PTH_EVENT_SEM, &outsignal);
  mode = 0;
  vmode = 0;
  Start ();
  TRACEPRINTF (t, 2, this, "Opened");
}

EIBNetIPTunnel::~EIBNetIPTunnel ()
{
  TRACEPRINTF (t, 2, this, "Close");
  Stop ();
  while (!outqueue.isempty ())
    delete outqueue.get ();
  pth_event_free (getwait, PTH_FREE_THIS);
  delete sock;
}

void
EIBNetIPTunnel::Send_L_Data (LPDU * l)
{
  TRACEPRINTF (t, 2, this, "Send %s", l->Decode ()());
  if (l->getType () != L_Data)
    {
      delete l;
      return;
    }
  L_Data_PDU *l1 = (L_Data_PDU *) l;
  inqueue.put (L_Data_ToCEMI (0x11, *l1));
  pth_sem_inc (&insignal, 1);
  if (vmode)
    {
      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
      l2->pdu.set (l->ToPacket ());
      outqueue.put (l2);
      pth_sem_inc (&outsignal, 1);
    }
  outqueue.put (l);
  pth_sem_inc (&outsignal, 1);
}

LPDU *
EIBNetIPTunnel::Get_L_Data (pth_event_t stop)
{
  if (stop != NULL)
    pth_event_concat (getwait, stop, NULL);

  pth_wait (getwait);

  if (stop)
    pth_event_isolate (getwait);

  if (pth_event_status (getwait) == PTH_STATUS_OCCURRED)
    {
      pth_sem_dec (&outsignal);
      LPDU *c = outqueue.get ();
      if (c)
	TRACEPRINTF (t, 2, this, "Recv %s", c->Decode ()());
      return c;
    }
  else
    return 0;
}

bool
EIBNetIPTunnel::Send_Queue_Empty ()
{
  return inqueue.isempty ();
}

bool
EIBNetIPTunnel::openVBusmonitor ()
{
  vmode = 1;
  return 1;
}

bool
EIBNetIPTunnel::closeVBusmonitor ()
{
  vmode = 0;
  return 1;
}

bool
EIBNetIPTunnel::enterBusmonitor ()
{
  mode = 1;
  return 1;
}

bool
EIBNetIPTunnel::leaveBusmonitor ()
{
  mode = 0;
  return 1;
}

bool
EIBNetIPTunnel::Open ()
{
  return 1;
}

bool
EIBNetIPTunnel::Close ()
{
  return 1;
}

bool
EIBNetIPTunnel::Connection_Lost ()
{
  return 0;
}

void
EIBNetIPTunnel::Run (pth_sem_t * stop1)
{
  int channel = -1;
  int mod = 0;
  int rno = 0;
  int sno = 0;
  int retry = 0;
  int heartbeat = 0;
  eibaddr_t myaddr;
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  pth_event_t input = pth_event (PTH_EVENT_SEM, &insignal);
  pth_event_t timeout = pth_event (PTH_EVENT_TIME, pth_timeout (0, 0));
  pth_event_t timeout1 = pth_event (PTH_EVENT_TIME, pth_timeout (10, 0));
  L_Data_PDU *c;

  EIBNetIPPacket p;
  EIBNetIPPacket *p1;
  EIBnet_ConnectRequest creq;
  EIBnet_ConnectResponse cresp;
  EIBnet_ConnectionStateRequest csreq;
  EIBnet_ConnectionStateResponse csresp;
  EIBnet_TunnelRequest treq;
  EIBnet_TunnelACK tresp;
  EIBnet_DisconnectRequest dreq;
  EIBnet_DisconnectResponse dresp;
  creq.caddr = saddr;
  creq.daddr = saddr;
  creq.CRI.resize (3);
  creq.CRI[0] = 0x04;
  creq.CRI[1] = 0x02;
  creq.CRI[2] = 0x00;
  p = creq.ToPacket ();
  sock->Send (p);

  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      if (mod == 1)
	pth_event_concat (stop, input, NULL);
      if (mod == 2)
	pth_event_concat (stop, timeout, NULL);

      pth_event_concat (stop, timeout1, NULL);

      p1 = sock->Get (stop);
      pth_event_isolate (stop);
      pth_event_isolate (timeout);
      pth_event_isolate (timeout1);
      if (p1)
	{
	  switch (p1->service)
	    {
	    case CONNECTION_RESPONSE:
	      if (mod)
		goto err;
	      if (parseEIBnet_ConnectResponse (*p1, cresp))
		{
		  TRACEPRINTF (t, 1, this, "Recv wrong connection response");
		  break;
		}
	      if (cresp.status != 0)
		{
		  TRACEPRINTF (t, 1, this, "Connect failed with error %02X",
			       cresp.status);
		  break;
		}
	      if (cresp.CRD () != 3)
		{
		  TRACEPRINTF (t, 1, this, "Recv wrong connection response");
		  break;
		}
	      myaddr = (cresp.CRD[1] << 8) | cresp.CRD[2];
	      daddr = cresp.daddr;
	      channel = cresp.channel;
	      mod = 1;
	      sock->recvaddr = daddr;
	      sock->sendaddr = daddr;
	      pth_event (PTH_EVENT_TIME | PTH_MODE_REUSE, timeout1,
			 pth_timeout (30, 0));
	      heartbeat = 0;
	      break;

	    case TUNNEL_REQUEST:
	      if (mod == 0)
		{
		  TRACEPRINTF (t, 1, this, "Not connected");
		  goto err;
		}
	      if (parseEIBnet_TunnelRequest (*p1, treq))
		{
		  TRACEPRINTF (t, 1, this, "Invalid request");
		  break;
		}
	      if (treq.channel != channel)
		{
		  TRACEPRINTF (t, 1, this, "Not for us");
		  break;
		}
	      if (treq.seqno != rno)
		{
		  TRACEPRINTF (t, 1, this, "Wrong sequence %d<->%d",
			       treq.seqno, rno);
		  break;
		}
	      rno++;
	      if (rno > 0xff)
		rno = 0;
	      tresp.status = 0;
	      tresp.channel = channel;
	      tresp.seqno = treq.seqno;
	      p = tresp.ToPacket ();
	      sock->Send (p);
	      //Confirmation
	      if (treq.CEMI[0] == 0x2E)
		break;
	      if (treq.CEMI[0] != 0x29)
		{
		  TRACEPRINTF (t, 1, this, "Unexpected CEMI Type %02X",
			       treq.CEMI[0]);
		  break;
		}
	      c = CEMI_to_L_Data (treq.CEMI);
	      if (c)
		{

		  TRACEPRINTF (t, 1, this, "Recv %s", c->Decode ()());
		  if (mode == 0)
		    {
		      if (vmode)
			{
			  L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
			  l2->pdu.set (c->ToPacket ());
			  outqueue.put (l2);
			  pth_sem_inc (&outsignal, 1);
			}
		      if (c->AddrType == IndividualAddress
			  && c->dest == myaddr)
			c->dest = 0;
		      outqueue.put (c);
		      pth_sem_inc (&outsignal, 1);
		      break;
		    }
		  L_Busmonitor_PDU *p1 = new L_Busmonitor_PDU;
		  p1->pdu = c->ToPacket ();
		  delete c;
		  outqueue.put (p1);
		  pth_sem_inc (&outsignal, 1);
		  break;
		}
	      TRACEPRINTF (t, 1, this, "Unknown CEMI");
	      break;
	    case TUNNEL_RESPONSE:
	      if (mod == 0)
		{
		  TRACEPRINTF (t, 1, this, "Not connected");
		  goto err;
		}
	      if (parseEIBnet_TunnelACK (*p1, tresp))
		{
		  TRACEPRINTF (t, 1, this, "Invalid response");
		  break;
		}
	      if (tresp.channel != channel)
		{
		  TRACEPRINTF (t, 1, this, "Not for us");
		  break;
		}
	      if (tresp.seqno != sno)
		{
		  TRACEPRINTF (t, 1, this, "Wrong sequence %d<->%d",
			       tresp.seqno, sno);
		  break;
		}
	      if (tresp.status)
		{
		  TRACEPRINTF (t, 1, this, "Error in ACK %d", tresp.status);
		  break;
		}
	      if (mod == 2)
		{
		  sno++;
		  if (sno > 0xff)
		    sno = 0;
		  pth_sem_dec (&insignal);
		  inqueue.get ();
		  mod = 1;
		  retry = 0;
		}
	      else
		TRACEPRINTF (t, 1, this, "Unexpected ACK");
	      break;
	    case CONNECTIONSTATE_RESPONSE:
	      if (parseEIBnet_ConnectionStateResponse (*p1, csresp))
		{
		  TRACEPRINTF (t, 1, this, "Invalid response");
		  break;
		}
	      if (csresp.channel != channel)
		{
		  TRACEPRINTF (t, 1, this, "Not for us");
		  break;
		}
	      if (csresp.status == 0)
		{
		  if (heartbeat > 0)
		    heartbeat--;
		  else
		    TRACEPRINTF (t, 1, this,
				 "Duplicate Connection State Response");
		}
	      else
		TRACEPRINTF (t, 1, this,
			     "Connection State Response Error %02x",
			     csresp.status);
	      break;
	    case DISCONNECT_REQUEST:
	      if (mod == 0)
		{
		  TRACEPRINTF (t, 1, this, "Not connected");
		  goto err;
		}
	      if (parseEIBnet_DisconnectRequest (*p1, dreq))
		{
		  TRACEPRINTF (t, 1, this, "Invalid request");
		  break;
		}
	      if (dreq.channel != channel)
		{
		  TRACEPRINTF (t, 1, this, "Not for us");
		  break;
		}
	      dresp.channel = channel;
	      dresp.status = 0;
	      p = treq.ToPacket ();
	      t->TracePacket (1, this, "SendDis", p.data);
	      sock->sendaddr = caddr;
	      sock->Send (p);
	      sock->sendaddr = daddr;
	      mod = 0;
	      break;
	    default:
	    err:
	      TRACEPRINTF (t, 1, this, "Recv unexpected service %04X",
			   p1->service);
	    }
	  delete p1;
	}
      if (mod == 2 && pth_event_status (timeout) == PTH_STATUS_OCCURRED)
	{
	  mod = 1;
	  retry++;
	}
      if (mod != 0 && pth_event_status (timeout1) == PTH_STATUS_OCCURRED)
	{
	  pth_event (PTH_EVENT_TIME | PTH_MODE_REUSE, timeout1,
		     pth_timeout (30, 0));
	  if (heartbeat < 5)
	    {
	      csreq.caddr = saddr;
	      csreq.channel = channel;
	      p = csreq.ToPacket ();
	      sock->sendaddr = caddr;
	      TRACEPRINTF (t, 1, this, "Heartbeat");
	      sock->Send (p);
	      sock->sendaddr = daddr;
	      heartbeat++;
	    }
	  else
	    {
	      TRACEPRINTF (t, 1, this, "Disconnection because of errors");
	      dreq.caddr = saddr;
	      dreq.channel = channel;
	      p = dreq.ToPacket ();
	      if (channel != -1)
		sock->Send (p);
	      mod = 0;
	    }
	}
      if (mod == 0 && pth_event_status (timeout1) == PTH_STATUS_OCCURRED)
	{
	  pth_event (PTH_EVENT_TIME | PTH_MODE_REUSE, timeout1,
		     pth_timeout (10, 0));
	  p = creq.ToPacket ();
	  TRACEPRINTF (t, 1, this, "Connectretry");
	  sock->sendaddr = caddr;
	  sock->Send (p);
	  sock->sendaddr = daddr;
	}

      if (!inqueue.isempty () && mod == 1)
	{
	  treq.channel = channel;
	  treq.seqno = sno;
	  treq.CEMI = inqueue.top ();
	  p = treq.ToPacket ();
	  t->TracePacket (1, this, "SendTunnel", p.data);
	  sock->Send (p);
	  mod = 2;
	  pth_event (PTH_EVENT_TIME | PTH_MODE_REUSE, timeout,
		     pth_timeout (1, 0));
	}
    }
out:
  dreq.caddr = saddr;
  dreq.channel = channel;
  p = dreq.ToPacket ();
  if (channel != -1)
    sock->Send (p);

  pth_event_free (stop, PTH_FREE_THIS);
  pth_event_free (input, PTH_FREE_THIS);
  pth_event_free (timeout, PTH_FREE_THIS);
  pth_event_free (timeout1, PTH_FREE_THIS);
}
