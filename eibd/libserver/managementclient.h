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

#ifndef MANAGEMENT_CLIENT_H
#define MANAGEMENT_CLIENT_H

#include "client.h"

/** reads all individual address of devices in the programming mode 
 * @param l3 Layer 3 interface
 * @param t debug output
 * @param c client connection
 * @param stop if occurs, function should abort
 */
void ReadIndividualAddresses (Layer3 * l3, Trace * t, ClientConnection * c,
			      pth_event_t stop);
/** change programming mode of a device
 * @param l3 Layer 3 interface
 * @param t debug output
 * @param c client connection
 * @param stop if occurs, function should abort
 */
void ChangeProgMode (Layer3 * l3, Trace * t, ClientConnection * c,
		     pth_event_t stop);

/** read the mask version of a device
 * @param l3 Layer 3 interface
 * @param t debug output
 * @param c client connection
 * @param stop if occurs, function should abort
 */
void GetMaskVersion (Layer3 * l3, Trace * t, ClientConnection * c,
		     pth_event_t stop);

/** write a individual address 
 * @param l3 Layer 3 interface
 * @param t debug output
 * @param c client connection
 * @param stop if occurs, function should abort
 */
void WriteIndividualAddress (Layer3 * l3, Trace * t, ClientConnection * c,
			     pth_event_t stop);

/** opens and handles a management connection
 * @param l3 Layer 3 interface
 * @param t debug output
 * @param c client connection
 * @param stop if occurs, function should abort
 */
void ManagementConnection (Layer3 * l3, Trace * t, ClientConnection * c,
			   pth_event_t stop);

/** Loads an image in a BCU
 * @param l3 Layer 3 interface
 * @param t debug output
 * @param c client connection
 * @param stop if occurs, function should abort
 */
void LoadImage (Layer3 * l3, Trace * t, ClientConnection * c,
		pth_event_t stop);

#endif
