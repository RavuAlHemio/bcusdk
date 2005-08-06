/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb.h"

void
check_device (usb_device_id_t cdev)
{
  struct usb_device_desc desc;
  struct usb_config_desc cfg;
  struct usb_interface_desc intf;
  struct usb_endpoint_desc ep;
  int in, out, outint;
  usb_dev_handle_t *h;
  char vendor[512];
  char product[512];
  int j, k, l;

  if (!cdev)
    return;

  usb_get_device_desc (cdev, &desc);
  for (j = 0; j < desc.bNumConfigurations; j++)
    {
      usb_get_config_desc (cdev, j, &cfg);
      for (k = 0; k < cfg.bNumInterfaces; k++)
	{
	  usb_get_interface_desc (cdev, j, k, &intf);
	  if (intf.bInterfaceClass != USB_CLASS_HID)
	    continue;

	  in = 0;
	  out = 0;
	  outint = 0;

	  for (l = 0; l < intf.bNumEndpoints; l++)
	    {
	      usb_get_endpoint_desc (cdev, j, k, l, &ep);
	      if (ep.wMaxPacketSize == 64)
		{
		  if (ep.bEndpointAddress & 0x80)
		    {
		      if ((ep.bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
			  USB_ENDPOINT_TYPE_INTERRUPT)
			in = ep.bEndpointAddress;
		    }
		  else
		    {
		      if (((ep.bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
			   USB_ENDPOINT_TYPE_CONTROL && !outint && 0)
			  || (ep.bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
			  USB_ENDPOINT_TYPE_INTERRUPT)
			{
			  out = ep.bEndpointAddress;
			  outint =
			    (ep.bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
			    USB_ENDPOINT_TYPE_INTERRUPT;
			}

		    }
		}
	    }
	  if (!in || !out)
	    continue;
	  if (usb_open (cdev, &h) >= 0)
	    {
	      memset (vendor, 0, sizeof (vendor));
	      memset (product, 0, sizeof (product));
	      if (usb_get_string_simple
		  (h, desc.iManufacturer, vendor, sizeof (vendor) - 1) < 0)
		strcpy (vendor, "<Unreadable>");
	      if (usb_get_string_simple
		  (h, desc.iProduct, product, sizeof (product) - 1) < 0)
		strcpy (product, "<Unreadable>");
	      printf ("device %d:%d:%d:%d (%s:%s)\n",
		      usb_get_busnum (usb_get_device_bus_id (cdev)),
		      usb_get_devnum (cdev), cfg.bConfigurationValue,
		      intf.bInterfaceNumber, vendor, product);
	      usb_close (h);
	    }
	}
    }
}

void
check_devlist (usb_device_id_t dev)
{
  usb_device_id_t cdev;
  int i;

  check_device (dev);
  for (i = 0; i < usb_get_child_count (dev); i++)
    {
      cdev = usb_get_child_device_id (dev, i + 1);
      check_devlist (cdev);
    }

}

int
main ()
{

  usb_bus_id_t bus;
  usb_device_id_t dev;
  usb_device_id_t cdev;
  int i, j, k, l;

  printf ("Possible addresses for KNX USB devices:\n");
  usb_set_debug (0);
  usb_init (0);
  for (bus = usb_get_first_bus_id (); bus; bus = usb_get_next_bus_id (bus))
    check_devlist (usb_get_root_device_id (bus));
}
