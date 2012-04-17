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

#ifndef C_USB_H
#define C_USB_H

#include "eibusb.h"
#include "usbif.h"
#include "usb.h"

#define USB_URL "usb:[bus[:device[:config[:interface]]]]\n"
#define USB_DOC "usb connects over a KNX USB interface\n\n"
#define USB_PREFIX "usb"
#define USB_CREATE Usb_Create
#define USB_CLEANUP USBEnd

inline Layer2Interface *
Usb_Create (const char *dev, int flags, Trace * t)
{
  if (!USBInit (t))
    return 0;
  return new USBLayer2Interface (new USBLowLevelDriver (dev, t), t, flags);
}

#endif
