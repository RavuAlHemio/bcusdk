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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <asm/types.h>
#include <linux/hiddev.h>
#include <sys/ioctl.h>
#include "usbif.h"

USBEndpoint
parseUSBEndpoint (const char *addr)
{
  USBEndpoint e;
  e.bus = -1;
  e.device = -1;
  e.config = -1;
  e.interface = -1;
  if (!*addr)
    return e;
  if (!isdigit (*addr))
    return e;
  e.bus = atoi (addr);
  while (isdigit (*addr))
    addr++;
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit (*addr))
    return e;
  e.device = atoi (addr);
  while (isdigit (*addr))
    addr++;
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit (*addr))
    return e;
  e.config = atoi (addr);
  while (isdigit (*addr))
    addr++;
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit (*addr))
    return e;
  e.interface = atoi (addr);
  return e;
}

bool
check_device (libusb_device_id_t cdev, USBEndpoint e, USBDevice & e2)
{
  struct usb_device_desc desc;
  struct usb_config_desc cfg;
  struct usb_interface_desc intf;
  struct usb_endpoint_desc ep;
  int in, out, outint;
  libusb_bus_id_t bus;
  unsigned char devnum;
  libusb_dev_handle_t h;
  int j, k, l;

  if (!cdev)
    return false;

  libusb_get_devnum (cdev, &devnum);
  libusb_get_bus_id (cdev, &bus);

  if (libusb_get_busnum (bus) != e.bus && e.bus != -1)
    return false;
  if (devnum != e.device && e.device != -1)
    return false;

  libusb_get_device_desc (cdev, &desc);
  for (j = 0; j < desc.bNumConfigurations; j++)
    {
      libusb_get_config_desc (cdev, j, &cfg);
      if (cfg.bConfigurationValue != e.config && e.config != -1)
	continue;
      for (k = 0; k < cfg.bNumInterfaces; k++)
	{
	  libusb_get_interface_desc (cdev, j, k, &intf);
	  if (intf.bInterfaceNumber != e.interface && e.interface != -1)
	    continue;
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
	      USBDevice e1;
	      e1.dev = cdev;
	      e1.config = cfg.bConfigurationValue;
	      e1.interface = intf.bInterfaceNumber;
	      e1.sendep = out;
	      e1.recvep = in;
	      libusb_close (h);
	      e2 = e1;
	      return true;
	    }
	}
    }
  return false;
}


bool
check_devlist (libusb_device_id_t dev, USBEndpoint e, USBDevice & e2)
{
  libusb_device_id_t cdev;
  unsigned char count;
  int i;

  if (check_device (dev, e, e2))
    return true;
  libusb_get_child_count (dev, &count);
  for (i = 0; i < count; i++)
    {
      if (libusb_get_child_device_id (dev, i + 1, &cdev) < 0)
	continue;
      if (check_devlist (cdev, e, e2))
	return true;
    }
  return false;
}

USBDevice
detectUSBEndpoint (USBEndpoint e)
{
  libusb_bus_id_t bus;
  libusb_device_id_t dev;
  USBDevice e2;
  e2.dev = -1;
  for (libusb_get_first_bus_id (&bus); bus;)
    {
      libusb_get_first_device_id (bus, &dev);
      if (check_devlist (dev, e, e2))
	break;

      if (libusb_get_next_bus_id (&bus) < 0)
	break;
    }
  return e2;
}


USBLowLevelDriver::USBLowLevelDriver (const char *Dev, Trace * tr)
{
  libusb_bus_id_t bus;
  unsigned char devnum;

  t = tr;
  pth_sem_init (&in_signal);
  pth_sem_init (&out_signal);
  pth_sem_init (&send_empty);
  pth_sem_set_value (&send_empty, 1);
  getwait = pth_event (PTH_EVENT_SEM, &out_signal);

  TRACEPRINTF (t, 1, this, "Detect");
  USBEndpoint e = parseUSBEndpoint (Dev);
  d = detectUSBEndpoint (e);
  state = 0;
  if (d.dev == -1)
    return;
  libusb_get_devnum (d.dev, &devnum);
  libusb_get_bus_id (d.dev, &bus);
  TRACEPRINTF (t, 1, this, "Using %d (%d:%d:%d:%d) (%d:%d)", d.dev,
	       libusb_get_busnum (bus),
	       devnum, d.config, d.interface, d.sendep, d.recvep);
  if (libusb_open (d.dev, &dev) < 0)
    return;
  state = 1;
  TRACEPRINTF (t, 1, this, "Open");
  libusb_detach_kernel_driver_np (dev, d.interface);
  if (libusb_set_configuration (dev, d.config) < 0)
    return;
  if (libusb_claim_interface (dev, d.interface) < 0)
    return;
  TRACEPRINTF (t, 1, this, "Claimed");
  state = 2;

  Start ();
  TRACEPRINTF (t, 1, this, "Opened");
}

USBLowLevelDriver::~USBLowLevelDriver ()
{
  TRACEPRINTF (t, 1, this, "Close");
  Stop ();
  pth_event_free (getwait, PTH_FREE_THIS);

  TRACEPRINTF (t, 1, this, "Release");
  if (state > 0)
    {
      libusb_release_interface (dev, d.interface);
      libusb_attach_kernel_driver_np (dev, d.interface);
    }
  TRACEPRINTF (t, 1, this, "Close");
  if (state > 0)
    libusb_close (dev);
}

bool
USBLowLevelDriver::init ()
{
  return state == 2;
}

bool
USBLowLevelDriver::Connection_Lost ()
{
  return 0;
}

void
USBLowLevelDriver::Send_Packet (CArray l)
{
  CArray pdu;
  t->TracePacket (1, this, "Send", l);

  inqueue.put (l);
  pth_sem_set_value (&send_empty, 0);
  pth_sem_inc (&in_signal, TRUE);
}

void
USBLowLevelDriver::SendReset ()
{
}

bool
USBLowLevelDriver::Send_Queue_Empty ()
{
  return inqueue.isempty ();
}

pth_sem_t *
USBLowLevelDriver::Send_Queue_Empty_Cond ()
{
  return &send_empty;
}

CArray *
USBLowLevelDriver::Get_Packet (pth_event_t stop)
{
  if (stop != NULL)
    pth_event_concat (getwait, stop, NULL);

  pth_wait (getwait);

  if (stop)
    pth_event_isolate (getwait);

  if (pth_event_status (getwait) == PTH_STATUS_OCCURRED)
    {
      pth_sem_dec (&out_signal);
      CArray *c = outqueue.get ();
      t->TracePacket (1, this, "Recv", *c);
      return c;
    }
  else
    return 0;
}

LowLevelDriverInterface::EMIVer USBLowLevelDriver::getEMIVer ()
{
  return vRaw;
}


void
USBLowLevelDriver::Run (pth_sem_t * stop1)
{
  int i, j;
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  pth_event_t input = pth_event (PTH_EVENT_SEM, &in_signal);
  uchar recvbuf[64];
  uchar sendbuf[64];
  libusb_io_handle_t *sendh = 0;
  libusb_io_handle_t *recvh = 0;
  pth_event_t sende = pth_event (PTH_EVENT_SEM, &in_signal);;
  pth_event_t recve = pth_event (PTH_EVENT_SEM, &in_signal);;

  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      if (!recvh)
	{
	  recvh =
	    libusb_submit_interrupt_read (dev, d.recvep, recvbuf,
					  sizeof (recvbuf), 1000, 0);
	  if (!recvh)
	    {
	      TRACEPRINTF (t, 0, this, "Error StartRecv");
	      break;
	    }
	  TRACEPRINTF (t, 0, this, "StartRecv");
	  pth_event (PTH_EVENT_FD | PTH_MODE_REUSE | PTH_UNTIL_FD_READABLE |
		     PTH_UNTIL_FD_WRITEABLE, recve,
		     libusb_io_wait_handle (recvh));
	}

      if (recvh && libusb_is_io_completed (recvh))
	{
	  if (libusb_io_comp_status (recvh) < 0)
	    TRACEPRINTF (t, 0, this, "RecvError %d",
			 libusb_io_comp_status (recvh));
	  else
	    {
	      TRACEPRINTF (t, 0, this, "RecvComplete %d",
			   libusb_io_xfer_size (recvh));
	      CArray res;
	      res.set (recvbuf, sizeof (recvbuf));
	      t->TracePacket (0, this, "RecvUSB", res);
	      outqueue.put (new CArray (res));
	      pth_sem_inc (&out_signal, 1);
	    }
	  libusb_io_free (recvh);
	  recvh = 0;
	  continue;
	}

      if (sendh && libusb_is_io_completed (sendh))
	{
	  if (libusb_io_comp_status (sendh) < 0)
	    TRACEPRINTF (t, 0, this, "SendError %d",
			 libusb_io_comp_status (sendh));
	  else
	    {
	      TRACEPRINTF (t, 0, this, "SendComplete %d",
			   libusb_io_xfer_size (sendh));
	      pth_sem_dec (&in_signal);
	      inqueue.get ();
	      if (inqueue.isempty ())
		pth_sem_set_value (&send_empty, 1);
	    }
	  libusb_io_free (sendh);
	  sendh = 0;
	  continue;
	}
      if (!sendh && !inqueue.isempty ())
	{
	  const CArray & c = inqueue.top ();
	  t->TracePacket (0, this, "Send", c);
	  memset (sendbuf, 0, sizeof (sendbuf));
	  memcpy (sendbuf, c.array (),
		  (c () > sizeof (sendbuf) ? sizeof (sendbuf) : c ()));

	  sendh =
	    libusb_submit_interrupt_write (dev, d.sendep, sendbuf,
					   sizeof (sendbuf), 1000, 0);
	  if (!sendh)
	    {
	      TRACEPRINTF (t, 0, this, "Error StartSend");
	      break;
	    }
	  TRACEPRINTF (t, 0, this, "StartSend");
	  pth_event (PTH_EVENT_FD | PTH_MODE_REUSE | PTH_UNTIL_FD_READABLE |
		     PTH_UNTIL_FD_WRITEABLE, sende,
		     libusb_io_wait_handle (sendh));
	  continue;
	}

      if (recvh)
	pth_event_concat (stop, recve, NULL);
      if (sendh)
	pth_event_concat (stop, sende, NULL);
      else
	pth_event_concat (stop, input, NULL);

      pth_wait (stop);

      pth_event_isolate (sende);
      pth_event_isolate (recve);
      pth_event_isolate (input);

    }
  if (sendh)
    {
      libusb_io_cancel (sendh);
      libusb_io_free (sendh);
    }
  if (recvh)
    {
      libusb_io_cancel (recvh);
      libusb_io_free (recvh);
    }
  pth_event_free (stop, PTH_FREE_THIS);
  pth_event_free (input, PTH_FREE_THIS);
  pth_event_free (sende, PTH_FREE_THIS);
  pth_event_free (recve, PTH_FREE_THIS);
}
