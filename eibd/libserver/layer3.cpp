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

#include "layer3.h"

Layer3::Layer3 (Layer2Interface * l2, Trace * tr)
{
  layer2 = l2;
  t = tr;
  t->TracePrintf (3, this, "Open");
  l2->Open ();
  mode = 0;
  Start ();
}

Layer3::~Layer3 ()
{
  t->TracePrintf (3, this, "Close");
  Stop ();
  if (mode)
    layer2->leaveBusmonitor ();
  else
    layer2->Close ();
  while (vbusmonitor ())
    deregisterVBusmonitor (vbusmonitor[0].cb);
  while (group ())
    deregisterGroupCallBack (group[0].cb, group[0].dest);
  while (individual ())
    deregisterIndividualCallBack (individual[0].cb, individual[0].src,
				  individual[0].dest);
  delete layer2;
}

void
Layer3::send_L_Data (L_Data_PDU * l)
{
  t->TracePrintf (3, this, "Send %s", l->Decode ()());
  if (l->source == 0)
    l->source = layer2->getDefaultAddr ();
  layer2->Send_L_Data (l);
}

bool
Layer3::deregisterBusmonitor (L_Busmonitor_CallBack * c)
{
  unsigned i;
  for (i = 0; i < busmonitor (); i++)
    if (busmonitor[i].cb == c)
      {
	busmonitor[i] = busmonitor[busmonitor () - 1];
	busmonitor.resize (busmonitor () - 1);
	if (busmonitor () == 0)
	  {
	    mode = 0;
	    layer2->leaveBusmonitor ();
	    layer2->Open ();
	  }
	t->TracePrintf (3, this, "deregisterBusmonitor %08X = 1", c);
	return 1;
      }
  t->TracePrintf (3, this, "deregisterBusmonitor %08X = 0", c);
  return 0;
}

bool
Layer3::deregisterVBusmonitor (L_Busmonitor_CallBack * c)
{
  unsigned i;
  for (i = 0; i < vbusmonitor (); i++)
    if (vbusmonitor[i].cb == c)
      {
	vbusmonitor[i] = vbusmonitor[vbusmonitor () - 1];
	vbusmonitor.resize (vbusmonitor () - 1);
	if (vbusmonitor () == 0)
	  {
	    layer2->closeVBusmonitor ();
	  }
	t->TracePrintf (3, this, "deregisterVBusmonitor %08X = 1", c);
	return 1;
      }
  t->TracePrintf (3, this, "deregisterVBusmonitor %08X = 0", c);
  return 0;
}

bool
Layer3::deregisterBroadcastCallBack (L_Data_CallBack * c)
{
  unsigned i;
  for (i = 0; i < broadcast (); i++)
    if (broadcast[i].cb == c)
      {
	broadcast[i] = broadcast[broadcast () - 1];
	broadcast.resize (broadcast () - 1);
	t->TracePrintf (3, this, "deregisterBroadcast %08X = 1", c);
	return 1;
      }
  t->TracePrintf (3, this, "deregisterBroadcast %08X = 0", c);
  return 0;

}

bool
Layer3::deregisterGroupCallBack (L_Data_CallBack * c, eibaddr_t addr)
{
  unsigned i;
  for (i = 0; i < group (); i++)
    if (group[i].cb == c && group[i].dest == addr)
      {
	group[i] = group[group () - 1];
	group.resize (group () - 1);
	t->TracePrintf (3, this, "deregisterGroupCallBack %08X = 1", c);
	for (i = 0; i < group (); i++)
	  {
	    if (group[i].dest == addr)
	      return 1;
	  }
	layer2->removeGroupAddress (addr);
	return 1;
      }
  t->TracePrintf (3, this, "deregisterGroupCallBack %08X = 0", c);
  return 0;

}

bool
  Layer3::deregisterIndividualCallBack (L_Data_CallBack * c, eibaddr_t src,
					eibaddr_t dest)
{
  unsigned i;
  for (i = 0; i < individual (); i++)
    if (individual[i].cb == c && individual[i].src == src
	&& individual[i].dest == dest)
      {
	individual[i] = individual[individual () - 1];
	individual.resize (individual () - 1);
	t->TracePrintf (3, this, "deregisterIndividual %08X = 1", c);
	for (i = 0; i < individual (); i++)
	  {
	    if (individual[i].dest == dest)
	      return 1;
	  }
	if (dest)
	  layer2->removeAddress (dest);
	return 1;
      }
  t->TracePrintf (3, this, "deregisterIndividual %08X = 0", c);
  return 0;
}

bool
Layer3::registerBusmonitor (L_Busmonitor_CallBack * c)
{
  t->TracePrintf (3, this, "registerBusmontior %08X", c);
  if (individual ())
    return 0;
  if (group ())
    return 0;
  if (broadcast ())
    return 0;
  if (mode == 0)
    {
      layer2->Close ();
      if (!layer2->enterBusmonitor ())
	{
	  layer2->Open ();
	  return 0;
	}
    }
  mode = 1;
  busmonitor.resize (busmonitor () + 1);
  busmonitor[busmonitor () - 1].cb = c;
  t->TracePrintf (3, this, "registerBusmontior %08X = 1", c);
  return 1;
}

bool
Layer3::registerVBusmonitor (L_Busmonitor_CallBack * c)
{
  t->TracePrintf (3, this, "registerVBusmonitor %08X", c);
  if (!vbusmonitor () && !layer2->openVBusmonitor ())
    return 0;

  vbusmonitor.resize (vbusmonitor () + 1);
  vbusmonitor[vbusmonitor () - 1].cb = c;
  t->TracePrintf (3, this, "registerVBusmontior %08X = 1", c);
  return 1;
}

bool
Layer3::registerBroadcastCallBack (L_Data_CallBack * c)
{
  t->TracePrintf (3, this, "registerBroadcast %08X", c);
  if (mode == 1)
    return 0;
  broadcast.resize (broadcast () + 1);
  broadcast[broadcast () - 1].cb = c;
  t->TracePrintf (3, this, "registerBroadcast %08X = 1", c);
  return 1;
}

bool
Layer3::registerGroupCallBack (L_Data_CallBack * c, eibaddr_t addr)
{
  unsigned i;
  t->TracePrintf (3, this, "registerGroup %08X", c);
  if (mode == 1)
    return 0;
  for (i = 0; i < group (); i++)
    {
      if (group[i].dest == addr)
	break;
    }
  if (i == group ())
    if (addr)
      if (!layer2->addGroupAddress (addr))
	return 0;
  group.resize (group () + 1);
  group[group () - 1].cb = c;
  group[group () - 1].dest = addr;
  t->TracePrintf (3, this, "registerGroup %08X = 1", c);
  return 1;
}

bool
  Layer3::registerIndividualCallBack (L_Data_CallBack * c, eibaddr_t src,
				      eibaddr_t dest)
{
  unsigned i;
  t->TracePrintf (3, this, "registerIndividual %08X", c);
  if (mode == 1)
    return 0;
  for (i = 0; i < individual (); i++)
    {
      if (individual[i].dest == dest)
	break;
    }
  if (i == individual () && dest)
    if (!layer2->addAddress (dest))
      return 0;
  individual.resize (individual () + 1);
  individual[individual () - 1].cb = c;
  individual[individual () - 1].dest = dest;
  individual[individual () - 1].src = src;
  t->TracePrintf (3, this, "registerIndividual %08X = 1", c);
  return 1;
}

void
Layer3::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  unsigned i;

  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      LPDU *l = layer2->Get_L_Data (stop);
      if (!l)
	continue;
      if (l->getType () == L_Busmonitor)
	{
	  L_Busmonitor_PDU *l1, *l2;
	  l1 = (L_Busmonitor_PDU *) l;

	  t->TracePrintf (3, this, "Recv %s", l1->Decode ()());
	  for (i = 0; i < busmonitor (); i++)
	    {
	      l2 = new L_Busmonitor_PDU (*l1);
	      busmonitor[i].cb->Get_L_Busmonitor (l2);
	    }
	  for (i = 0; i < vbusmonitor (); i++)
	    {
	      l2 = new L_Busmonitor_PDU (*l1);
	      vbusmonitor[i].cb->Get_L_Busmonitor (l2);
	    }
	}
      if (l->getType () == L_Data)
	{
	  L_Data_PDU *l1;
	  l1 = (L_Data_PDU *) l;
	  if (l1->AddrType == IndividualAddress
	      && l1->dest == layer2->getDefaultAddr ())
	    l1->dest = 0;
	  t->TracePrintf (3, this, "Recv %s", l1->Decode ()());

	  if (l1->AddrType == GroupAddress && l1->dest == 0)
	    {
	      for (i = 0; i < broadcast (); i++)
		broadcast[i].cb->Get_L_Data (new L_Data_PDU (*l1));
	    }
	  if (l1->AddrType == GroupAddress && l1->dest != 0)
	    {
	      for (i = 0; i < group (); i++)
		if (group[i].dest == l1->dest || group[i].dest == 0)
		  group[i].cb->Get_L_Data (new L_Data_PDU (*l1));
	    }
	  if (l1->AddrType == IndividualAddress)
	    {
	      for (i = 0; i < individual (); i++)
		if (individual[i].dest == l1->dest)
		  if (individual[i].src == l1->source
		      || individual[i].src == 0)
		    individual[i].cb->Get_L_Data (new L_Data_PDU (*l1));
	    }
	}
      delete l;

    }
  pth_event_free (stop, PTH_FREE_THIS);
}
