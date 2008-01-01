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

#ifndef GROUPCACHE_H
#define GROUPCACHE_H

#include <time.h>

#include "layer3.h"

typedef struct
{
  /** Layer 4 data */
  CArray data;
  /** source address */
  eibaddr_t src;
  /** destination address */
  eibaddr_t dst;
  /** receive time */
  time_t recvtime;
} GroupCacheEntry;

class GroupCache:public L_Data_CallBack
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** debug output */
  Trace *t;
  /** output queue */
    Array < GroupCacheEntry * >cache;
  bool enable;
  pth_mutex_t mutex;
  pth_cond_t cond;

  GroupCacheEntry *find (eibaddr_t dst);
  void add (GroupCacheEntry * entry);

public:
    GroupCache (Layer3 * l3, Trace * t);
    virtual ~ GroupCache ();

  void Get_L_Data (L_Data_PDU * l);

  bool Start ();
  void Clear ();
  void Stop ();

  GroupCacheEntry Read (eibaddr_t addr, unsigned timeout, uint16_t age);
  void remove (eibaddr_t addr);
};

#endif
