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

#ifndef GROUPCACHECLIENT_H
#define GROUPCACHECLIENT_H

#include "layer3.h"

class ClientConnection;

bool CreateGroupCache (Layer3 * l3, Trace * t, bool enable);
void DeleteGroupCache ();

void GroupCacheRequest (Layer3 * l3, Trace * t, ClientConnection * c,
			pth_event_t stop);

#endif
