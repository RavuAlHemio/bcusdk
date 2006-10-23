/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2006 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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

#ifndef EIB_USB_H
#define EIB_USB_H

#include "lowlevel.h"
#include "libusb.h"

typedef struct
{
  int bus;
  int device;
  int config;
  int interface;
} USBEndpoint;

typedef struct
{
  libusb_device_id_t dev;
  int config;
  int interface;
  int sendep;
  int recvep;
} USBDevice;

USBEndpoint parseUSBEndpoint (const char *addr);
USBDevice detectUSBEndpoint (USBEndpoint e);

class USBLowLevelDriver:public LowLevelDriverInterface, private Thread
{
  libusb_dev_handle_t dev;
  USBDevice d;
  /** debug output */
  Trace *t;
  /** semaphore for inqueue */
  pth_sem_t in_signal;
  /** semaphore for outqueue */
  pth_sem_t out_signal;
  /** input queue */
    Queue < CArray > inqueue;
    /** output queue */
    Queue < CArray * >outqueue;
    /** event to wait for outqueue */
  pth_event_t getwait;
  /** semaphore to signal empty sendqueue */
  pth_sem_t send_empty;

  void Run (pth_sem_t * stop);

public:
    USBLowLevelDriver (const char *device, Trace * tr);
   ~USBLowLevelDriver ();

  void Send_Packet (CArray l);
  bool Send_Queue_Empty ();
  pth_sem_t *Send_Queue_Empty_Cond ();
  CArray *Get_Packet (pth_event_t stop);
  void SendReset ();
  bool Connection_Lost ();
  EMIVer getEMIVer ();
};

#endif
