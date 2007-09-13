/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#include "groupcache.h"
#include "tpdu.h"
#include "apdu.h"

GroupCache::GroupCache (Layer3 * l3, Trace * t)
{
  TRACEPRINTF (t, 4, this, "GroupCacheInit");
  this->t = t;
  this->layer3 = l3;
  this->enable = 0;
  pth_mutex_init (&mutex);
  pth_cond_init (&cond);
}

GroupCache::~GroupCache ()
{
  TRACEPRINTF (t, 4, this, "GroupCacheDestroy");
  if (enable)
    layer3->deregisterGroupCallBack (this, 0);
  Clear ();
}

GroupCacheEntry *
GroupCache::find (eibaddr_t dst)
{
  int l = 0, r = cache ();
  while (l < r)
    {
      int p = (l + r) / 2;
      if (cache[p]->dst == dst)
	return cache[p];
      if (dst > cache[p]->dst)
	l = p + 1;
      else
	r = p - 1;
    }
  return 0;
}

void
GroupCache::remove (eibaddr_t addr)
{
  TRACEPRINTF (t, 4, this, "GroupCacheRemove %d/%d/%d", (addr >> 11) & 0x1f,
	       (addr >> 8) & 0x07, (addr) & 0xff);

  int l = 0, r = cache ();
  while (l < r)
    {
      int p = (l + r) / 2;
      if (cache[p]->dst == addr)
	{
	  delete cache[p];
	  cache.deletepart (p, 1);
	  return;
	}
      if (addr > cache[p]->dst)
	l = p + 1;
      else
	r = p - 1;
    }
  return;
}

void
GroupCache::add (GroupCacheEntry * entry)
{
  unsigned p;
  cache.resize (cache () + 1);
  p = cache () - 1;
  while (p > 0 && cache[p - 1]->dst > entry->dst)
    {
      cache[p] = cache[p - 1];
      p--;
    }
  cache[p] = entry;
}


void
GroupCache::Get_L_Data (L_Data_PDU * l)
{
  GroupCacheEntry *c;
  if (enable)
    {
      TPDU *t = TPDU::fromPacket (l->data);
      if (t->getType () == T_DATA_XXX_REQ)
	{
	  T_DATA_XXX_REQ_PDU *t1 = (T_DATA_XXX_REQ_PDU *) t;
	  if (t1->data () >= 2 && !(t1->data[0] & 0x3) &&
	      ((t1->data[1] & 0xC0) == 0x40 || (t1->data[1] & 0xC0) == 0x80))
	    {
	      c = find (l->dest);
	      if (c)
		{
		  c->data = t1->data;
		  c->src = l->source;
		  c->dst = l->dest;
		}
	      else
		{
		  c = new GroupCacheEntry;
		  c->data = t1->data;
		  c->src = l->source;
		  c->dst = l->dest;
		  add (c);
		  pth_cond_notify (&cond, 1);
		}
	    }
	}
      delete t;
    }
  delete l;
}

bool
GroupCache::Start ()
{
  TRACEPRINTF (t, 4, this, "GroupCacheEnable");
  if (!layer3->registerGroupCallBack (this, 0))
    return false;
  enable = 1;
  return true;
}

void
GroupCache::Clear ()
{
  int i;
  TRACEPRINTF (t, 4, this, "GroupCacheClear");
  for (i = 0; i < cache (); i++)
    delete cache[i];
  cache.resize (0);
}

void
GroupCache::Stop ()
{
  Clear ();
  TRACEPRINTF (t, 4, this, "GroupCacheStop");
  if (enable)
    layer3->deregisterGroupCallBack (this, 0);
  enable = 0;
}

GroupCacheEntry
GroupCache::Read (eibaddr_t addr, unsigned Timeout)
{
  TRACEPRINTF (t, 4, this, "GroupCacheRead %d/%d/%d %d", (addr >> 11) & 0x1f,
	       (addr >> 8) & 0x07, (addr) & 0xff, Timeout);
  GroupCacheEntry *c;
  if (!enable)
    {
      GroupCacheEntry f;
      f.src = 0;
      f.dst = 0;
      TRACEPRINTF (t, 4, this, "GroupCache not enabled");
      return f;
    }

  c = find (addr);
  if (c)
    {
      TRACEPRINTF (t, 4, this, "GroupCache found: %d.%d.%d",
		   (c->src >> 12) & 0xf, (c->src >> 8) & 0xf,
		   (c->src) & 0xff);
      return *c;
    }

  if (!Timeout)
    {
      GroupCacheEntry f;
      f.src = 0;
      f.dst = addr;
      TRACEPRINTF (t, 4, this, "GroupCache no entry");
      return f;
    }

  A_GroupValue_Read_PDU apdu;
  T_DATA_XXX_REQ_PDU tpdu;
  L_Data_PDU *l;
  pth_event_t timeout = pth_event (PTH_EVENT_TIME, pth_timeout (Timeout, 0));;

  tpdu.data = apdu.ToPacket ();
  l = new L_Data_PDU;
  l->data = tpdu.ToPacket ();
  l->source = 0;
  l->dest = addr;
  l->AddrType = GroupAddress;
  layer3->send_L_Data (l);

  do
    {
      c = find (addr);
      if (c)
	{
	  TRACEPRINTF (t, 4, this, "GroupCache found: %d.%d.%d",
		       (c->src >> 12) & 0xf, (c->src >> 8) & 0xf,
		       (c->src) & 0xff);
	  return *c;
	}

      pth_mutex_acquire (&mutex, 0, 0);
      pth_cond_await (&cond, &mutex, timeout);
      if (pth_event_status (timeout) == PTH_STATUS_OCCURRED)
	{
	  c = new GroupCacheEntry;
	  c->src = 0;
	  c->dst = addr;
	  add (c);
	  TRACEPRINTF (t, 4, this, "GroupCache timeout");
	  return *c;
	}
      pth_mutex_release (&mutex);
    }
  while (1);
}
