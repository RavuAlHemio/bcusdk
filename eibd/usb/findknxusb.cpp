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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libusb.h"

void
check_device (libusb_device_id_t cdev)
{
  struct usb_device_desc desc;
  struct usb_config_desc cfg;
  struct usb_interface_desc intf;
  struct usb_endpoint_desc ep;
  libusb_bus_id_t bus;
  int in, out, outint;
  libusb_dev_handle_t h;
  char vendor[512];
  char product[512];
  int j, k, l;
  unsigned char devnum;

  if (!cdev)
    return;

  libusb_get_device_desc (cdev, &desc);
  for (j = 0; j < desc.bNumConfigurations; j++)
    {
      libusb_get_config_desc (cdev, j, &cfg);
      for (k = 0; k < cfg.bNumInterfaces; k++)
	{
	  libusb_get_interface_desc (cdev, j, k, &intf);
	  if (intf.bInterfaceClass != USB_CLASS_HID)
	    continue;

	  in = 0;
	  out = 0;
	  outint = 0;

	  for (l = 0; l < intf.bNumEndpoints; l++)
	    {
	      libusb_get_endpoint_desc (cdev, j, k, l, &ep);
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
	  if (libusb_open (cdev, &h) >= 0)
	    {
	      memset (vendor, 0, sizeof (vendor));
	      memset (product, 0, sizeof (product));
	      if (libusb_get_string_simple
		  (h, desc.iManufacturer, (unsigned char *) vendor,
		   sizeof (vendor) - 1) < 0)
		strcpy (vendor, "<Unreadable>");
	      if (libusb_get_string_simple
		  (h, desc.iProduct, (unsigned char *) product,
		   sizeof (product) - 1) < 0)
		strcpy (product, "<Unreadable>");
	      libusb_get_devnum (cdev, &devnum);
	      libusb_get_bus_id (cdev, &bus);
	      printf ("device %d:%d:%d:%d (%s:%s)\n",
		      libusb_get_busnum (bus),
		      devnum, cfg.bConfigurationValue,
		      intf.bInterfaceNumber, vendor, product);
	      libusb_close (h);
	    }
	}
    }
}

void
check_devlist (libusb_device_id_t dev)
{
  libusb_device_id_t cdev;
  unsigned char count;
  int i;

  check_device (dev);
  libusb_get_child_count (dev, &count);
  for (i = 0; i < count; i++)
    {
      if (libusb_get_child_device_id (dev, i + 1, &cdev) < 0)
	continue;
      check_devlist (cdev);
    }

}

int
main ()
{

  libusb_bus_id_t bus;
  libusb_device_id_t dev;

  printf ("Possible addresses for KNX USB devices:\n");
  libusb_set_debug (0);
  libusb_init ();
  for (libusb_get_first_bus_id (&bus); bus;)
    {
      libusb_get_first_device_id (bus, &dev);
      check_devlist (dev);
      if (libusb_get_next_bus_id (&bus) < 0)
	break;
    }
}
