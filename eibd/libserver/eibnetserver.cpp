/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2008 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#include "eibnetserver.h"
#include "emi.h"
#include "config.h"

#define NAME "eibd"

EIBnetServer::EIBnetServer (const char *multicastaddr, int port, bool Tunnel,
			    bool Route, bool Discover, Layer3 * layer3,
			    Trace * tr)
{
  struct sockaddr_in baddr;
  struct ip_mreq mcfg;
  t = tr;
  l3 = layer3;

  TRACEPRINTF (t, 8, this, "Open");
  memset (&baddr, 0, sizeof (baddr));
#ifdef HAVE_SOCKADDR_IN_LEN
  baddr.sin_len = sizeof (baddr);
#endif
  baddr.sin_family = AF_INET;
  baddr.sin_port = htons (port);
  baddr.sin_addr.s_addr = htonl (INADDR_ANY);

  if (GetHostIP (&maddr, multicastaddr) == 0)
    {
      sock = 0;
      return;
    }
  maddr.sin_port = htons (port);

  sock = new EIBNetIPSocket (baddr, 1, t);
  if (!sock->init ())
    {
      delete sock;
      sock = 0;
      return;
    }
  mcfg.imr_multiaddr = maddr.sin_addr;
  mcfg.imr_interface.s_addr = htonl (INADDR_ANY);
  if (!sock->SetMulticast (mcfg))
    {
      delete sock;
      sock = 0;
      return;
    }
  sock->recvall = 2;
  if (!GetSourceAddress (&maddr, &sock->localaddr))
    {
      delete sock;
      sock = 0;
      return;
    }
  sock->localaddr.sin_port = htons (port);
  tunnel = Tunnel;
  route = Route;
  discover = Discover;
  Port = htons (port);
  if (route || tunnel)
    {
      if (!l3->registerBroadcastCallBack (this))
	{
	  delete sock;
	  sock = 0;
	  return;
	}
      if (!l3->registerGroupCallBack (this, 0))
	{
	  delete sock;
	  sock = 0;
	  return;
	}
      if (!l3->registerIndividualCallBack (this, Individual_Lock_None, 0, 0))
	{
	  delete sock;
	  sock = 0;
	  return;
	}
    }
  Start ();
  TRACEPRINTF (t, 8, this, "Opened");
}


EIBnetServer::~EIBnetServer ()
{
  TRACEPRINTF (t, 8, this, "Close");
  if (route || tunnel)
    {
      l3->deregisterBroadcastCallBack (this);
      l3->deregisterGroupCallBack (this, 0);
      l3->deregisterIndividualCallBack (this, 0, 0);
    }
  Stop ();
  if (sock)
    delete sock;
}

bool
EIBnetServer::init ()
{
  return sock != 0;
}

void
EIBnetServer::Get_L_Data (L_Data_PDU * l)
{
  if (route)
    {
      TRACEPRINTF (t, 8, this, "Send_Route %s", l->Decode ()());
      sock->sendaddr = maddr;
      EIBNetIPPacket p;
      p.service = ROUTING_INDICATION;
      p.data = L_Data_ToCEMI (0x29, *l);
      sock->Send (p);
    }
  for (int i = 0; i < state (); i++)
    {
      state[i].out.put (L_Data_ToCEMI (0x29, *l));
      pth_sem_inc (state[i].outsignal, 0);
    }
  delete l;
}

void
EIBnetServer::Run (pth_sem_t * stop1)
{
  EIBNetIPPacket *p1;
  EIBNetIPPacket p;
  int i;
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);

  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      for (i = 0; i < state (); i++)
	{
	  pth_event_concat (stop, state[i].timeout, NULL);
	  if (state[i].state)
	    pth_event_concat (stop, state[i].sendtimeout, NULL);
	  else
	    pth_event_concat (stop, state[i].outwait, NULL);

	}
      p1 = sock->Get (stop);
      for (i = 0; i < state (); i++)
	{
	  pth_event_isolate (state[i].timeout);
	  pth_event_isolate (state[i].sendtimeout);
	  pth_event_isolate (state[i].outwait);
	}
      if (p1)
	{
	  if (p1->service == SEARCH_REQUEST && discover)
	    {
	      EIBnet_SearchRequest r1;
	      EIBnet_SearchResponse r2;
	      DIB_service_Entry d;
	      if (parseEIBnet_SearchRequest (*p1, r1))
		goto out;
	      TRACEPRINTF (t, 8, this, "SEARCH");
	      r2.KNXmedium = 2;
	      r2.devicestatus = 0;
	      r2.individual_addr = 0;
	      r2.installid = 0;
	      r2.multicastaddr = maddr.sin_addr;
	      strcpy ((char *) r2.name, NAME);
	      d.version = 1;
	      d.family = 2;
	      if (discover)
		r2.services.add (d);
	      d.family = 4;
	      if (tunnel)
		r2.services.add (d);
	      d.family = 5;
	      if (route)
		r2.services.add (d);
	      if (!GetSourceAddress (&r1.caddr, &r2.caddr))
		goto out;
	      r2.caddr.sin_port = Port;
	      sock->sendaddr = r1.caddr;
	      sock->Send (r2.ToPacket ());
	    }
	  if (p1->service == DESCRIPTION_REQUEST && discover)
	    {
	      EIBnet_DescriptionRequest r1;
	      EIBnet_DescriptionResponse r2;
	      DIB_service_Entry d;
	      if (parseEIBnet_DescriptionRequest (*p1, r1))
		goto out;
	      TRACEPRINTF (t, 8, this, "DESCRIBE");
	      r2.KNXmedium = 2;
	      r2.devicestatus = 0;
	      r2.individual_addr = 0;
	      r2.installid = 0;
	      r2.multicastaddr = maddr.sin_addr;
	      strcpy ((char *) r2.name, NAME);
	      d.version = 1;
	      d.family = 2;
	      if (discover)
		r2.services.add (d);
	      d.family = 4;
	      if (tunnel)
		r2.services.add (d);
	      d.family = 5;
	      if (route)
		r2.services.add (d);
	      sock->sendaddr = r1.caddr;
	      sock->Send (r2.ToPacket ());
	    }
	  if (p1->service == ROUTING_INDICATION && route)
	    {
	      if (p1->data () < 2 || p1->data[0] != 0x29)
		goto out;
	      const CArray data = p1->data;
	      L_Data_PDU *c = CEMI_to_L_Data (data);
	      if (c)
		{
		  TRACEPRINTF (t, 8, this, "Recv_Route %s", c->Decode ()());
		  l3->send_L_Data (c);
		}
	    }
	  if (p1->service == CONNECTIONSTATE_REQUEST && tunnel)
	    {
	      uchar res = 21;
	      EIBnet_ConnectionStateRequest r1;
	      EIBnet_ConnectionStateResponse r2;
	      if (parseEIBnet_ConnectionStateRequest (*p1, r1))
		goto out;
	      for (i = 0; i < state (); i++)
		if (state[i].channel = r1.channel)
		  {
		    res = 0;
		    pth_event (PTH_EVENT_TIME | PTH_MODE_REUSE,
			       state[i].timeout, pth_timeout (120, 0));
		  }
	      r2.channel = r1.channel;
	      r2.status = res;
	      sock->sendaddr = r1.caddr;
	      sock->Send (r2.ToPacket ());
	    }
	  if (p1->service == DISCONNECT_REQUEST && tunnel)
	    {
	      uchar res = 0x21;
	      EIBnet_DisconnectRequest r1;
	      EIBnet_DisconnectResponse r2;
	      if (parseEIBnet_DisconnectRequest (*p1, r1))
		goto out;
	      for (i = 0; i < state (); i++)
		if (state[i].channel = r1.channel)
		  {
		    res = 0;
		    pth_event_free (state[i].timeout, PTH_FREE_THIS);
		    pth_event_free (state[i].sendtimeout, PTH_FREE_THIS);
		    pth_event_free (state[i].outwait, PTH_FREE_THIS);
		    delete state[i].outsignal;
		    state.deletepart (i, 1);
		    break;
		  }
	      r2.channel = r1.channel;
	      r2.status = res;
	      sock->sendaddr = r1.caddr;
	      sock->Send (r2.ToPacket ());
	    }
	  if (p1->service == CONNECTION_REQUEST && tunnel)
	    {
	      EIBnet_ConnectRequest r1;
	      EIBnet_ConnectResponse r2;
	      if (parseEIBnet_ConnectRequest (*p1, r1))
		goto out;
	      r2.CRD.resize (3);
	      r2.CRD[0] = 0x04;
	      r2.CRD[1] = 0x00;
	      r2.CRD[2] = 0x00;
	      r2.status = 0x22;
	      if (r1.CRI () == 3 && r1.CRI[0] == 4 && r1.CRI[1] == 2)
		{
		  int id = 1;
		rt:
		  for (i = 0; i < state (); i++)
		    if (state[i].channel == id)
		      {
			id++;
			goto rt;
		      }
		  if (id <= 0xff)
		    {
		      int pos = state ();
		      state.resize (state () + 1);
		      state[pos].timeout =
			pth_event (PTH_EVENT_TIME, pth_timeout (120, 0));
		      state[pos].outsignal = new pth_sem_t;
		      pth_sem_init (state[pos].outsignal);
		      state[pos].outwait =
			pth_event (PTH_EVENT_SEM, state[pos].outsignal);
		      state[pos].sendtimeout =
			pth_event (PTH_EVENT_TIME, pth_timeout (1, 0));
		      state[pos].channel = id;
		      state[pos].daddr = r1.daddr;
		      state[pos].caddr = r1.caddr;
		      state[pos].state = 0;
		      state[pos].sno = 0;
		      state[pos].rno = 0;
		      r2.channel = id;
		      r2.status = 0;
		    }
		}
	      if (!GetSourceAddress (&r1.caddr, &r2.daddr))
		goto out;
	      r2.daddr.sin_port = Port;
	      sock->sendaddr = r1.caddr;
	      sock->Send (r2.ToPacket ());
	    }
	  if (p1->service == TUNNEL_REQUEST && tunnel)
	    {
	      EIBnet_TunnelRequest r1;
	      EIBnet_TunnelACK r2;
	      if (parseEIBnet_TunnelRequest (*p1, r1))
		goto out;
	      TRACEPRINTF (t, 8, this, "TUNNEL_REQ");
	      for (i = 0; i < state (); i++)
		if (state[i].channel == r1.channel)
		  goto reqf;
	      goto out;
	    reqf:
	      if (state[i].rno == (r1.seqno + 1) & 0xff)
		{
		  r2.channel = r1.channel;
		  r2.seqno = r1.seqno;
		  sock->sendaddr = state[i].daddr;
		  sock->Send (r2.ToPacket ());
		  goto out;
		}
	      if (state[i].rno != r1.seqno)
		{
		  TRACEPRINTF (t, 8, this, "Wrong sequence %d<->%d",
			       r1.seqno, state[i].rno);
		  goto out;
		}
	      r2.channel = r1.channel;
	      r2.seqno = r1.seqno;
	      L_Data_PDU *c = CEMI_to_L_Data (r1.CEMI);
	      if (c)
		{
		  r2.status = 0;
		  if (r1.CEMI[0] == 0x11)
		    {
		      state[i].out.put (L_Data_ToCEMI (0x2E, *c));
		      pth_sem_inc (state[i].outsignal, 0);
		    }
		  if (r1.CEMI[0] == 0x11 || r1.CEMI[0] == 0x29)
		    l3->send_L_Data (c);
		  else
		    delete c;

		}
	      else
		r2.status = 0x29;
	      state[i].rno++;
	      if (state[i].rno > 0xff)
		state[i].rno = 0;
	      sock->sendaddr = state[i].daddr;
	      sock->Send (r2.ToPacket ());
	    }
	  if (p1->service == TUNNEL_RESPONSE && tunnel)
	    {
	      EIBnet_TunnelACK r1;
	      if (parseEIBnet_TunnelACK (*p1, r1))
		goto out;
	      TRACEPRINTF (t, 8, this, "TUNNEL_ACK");
	      for (i = 0; i < state (); i++)
		if (state[i].channel == r1.channel)
		  goto reqf1;
	      goto out;
	    reqf1:
	      if (state[i].sno != r1.seqno)
		{
		  TRACEPRINTF (t, 8, this, "Wrong sequence %d<->%d",
			       r1.seqno, state[i].sno);
		  goto out;
		}
	      if (r1.status != 0)
		{
		  TRACEPRINTF (t, 8, this, "Wrong status %d", r1.status);
		  goto out;
		}
	      if (!state[i].state)
		{
		  TRACEPRINTF (t, 8, this, "Unexpected ACK");
		  goto out;
		}
	      state[i].sno++;
	      if (state[i].sno > 0xff)
		state[i].sno = 0;
	      state[i].state = 0;
	      state[i].out.get ();
	      pth_sem_dec (state[i].outsignal);
	    }
	out:
	  delete p1;
	}
      for (i = 0; i < state (); i++)
	if (pth_event_status (state[i].timeout) == PTH_STATUS_OCCURRED)
	  {
	    pth_event_free (state[i].timeout, PTH_FREE_THIS);
	    pth_event_free (state[i].sendtimeout, PTH_FREE_THIS);
	    pth_event_free (state[i].outwait, PTH_FREE_THIS);
	    delete state[i].outsignal;
	    state.deletepart (i, 1);
	    break;
	  }
      for (i = 0; i < state (); i++)
	{
	  if ((state[i].state
	       && pth_event_status (state[i].sendtimeout) ==
	       PTH_STATUS_OCCURRED) || (!state[i].state
					&& !state[i].out.isempty ()))
	    {
	      TRACEPRINTF (t, 8, this, "TunnelSend %d", state[i].channel);
	      state[i].state++;
	      if (state[i].state > 10)
		{
		  state[i].out.get ();
		  pth_sem_dec (state[i].outsignal);
		  state[i].state = 0;
		  continue;
		}
	      EIBnet_TunnelRequest r;
	      r.channel = state[i].channel;
	      r.seqno = state[i].sno;
	      r.CEMI = state[i].out.top ();
	      pth_event (PTH_EVENT_TIME | PTH_MODE_REUSE,
			 state[i].sendtimeout, pth_timeout (1, 0));
	      sock->sendaddr = state[i].daddr;
	      sock->Send (r.ToPacket ());
	    }

	}
    }
  for (i = 0; i < state (); i++)
    {
      EIBnet_DisconnectRequest r;
      r.channel = state[i].channel;
      if (!GetSourceAddress (&state[i].caddr, &r.caddr))
	continue;
      r.caddr.sin_port = Port;
      sock->sendaddr = state[i].caddr;
      sock->Send (r.ToPacket ());
      pth_event_free (state[i].timeout, PTH_FREE_THIS);
      pth_event_free (state[i].sendtimeout, PTH_FREE_THIS);
      pth_event_free (state[i].outwait, PTH_FREE_THIS);
      delete state[i].outsignal;
    }
  pth_event_free (stop, PTH_FREE_THIS);
}
