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

#ifndef LAYER2_H
#define LAYER2_H

#include "lpdu.h"

/** interface for an Layer 2 driver */
class Layer2Interface
{
public:
  virtual ~ Layer2Interface ()
  {
  }
  virtual bool init () = 0;

  /** sends a Layer 2 frame asynchronouse */
  virtual void Send_L_Data (LPDU * l) = 0;
  /** waits for the next frame
   * @param stop return NULL, if stop occurs
   * @return returns frame or NULL
   */
  virtual LPDU *Get_L_Data (pth_event_t stop) = 0;

  /** try to add the individual address addr to the device, return true if successful */
  virtual bool addAddress (eibaddr_t addr) = 0;
  /** try to add the group address addr to the device, return true if successful */
  virtual bool addGroupAddress (eibaddr_t addr) = 0;
  /** try to remove the individual address addr to the device, return true if successful */
  virtual bool removeAddress (eibaddr_t addr) = 0;
  /** try to remove the group address addr to the device, return true if successful */
  virtual bool removeGroupAddress (eibaddr_t addr) = 0;

  /** try to enter the busmonitor mode, return true if successful */
  virtual bool enterBusmonitor () = 0;
  /** try to leave the busmonitor mode, return true if successful */
  virtual bool leaveBusmonitor () = 0;

  /** try to enter the vbusmonitor mode, return true if successful */
  virtual bool openVBusmonitor () = 0;
  /** try to leave the vbusmonitor mode, return true if successful */
  virtual bool closeVBusmonitor () = 0;

  /** try to enter the normal operation mode, return true if successful */
  virtual bool Open () = 0;
  /** try to leave the normal operation mode, return true if successful */
  virtual bool Close () = 0;
  /** returns the default individual address of the device */
  virtual eibaddr_t getDefaultAddr () = 0;
  /** return true, if the connection is broken */
  virtual bool Connection_Lost () = 0;
  /** return true, if all frames have been sent */
  virtual bool Send_Queue_Empty () = 0;
};

/** interface for callback for Layer 2 frames */
class LPDU_CallBack
{
public:
  /** callback: a Layer 2 frame has been received */
  virtual void Get_LPDU (LPDU * l) = 0;
};

/** interface for callback for L_Data frames */
class L_Data_CallBack
{
public:
  /** callback: a L_Data frame has been received */
  virtual void Get_L_Data (L_Data_PDU * l) = 0;
};

/** interface for callback for busmonitor frames */
class L_Busmonitor_CallBack
{
public:
  /** callback: a bus monitor frame has been received */
  virtual void Get_L_Busmonitor (L_Busmonitor_PDU * l) = 0;
};

/** pointer to a functions, which creates a Layer 2 interface
 * @exception Exception in the case of an error
 * @param conf string, which contain configuration
 * @param t trace output
 * @return new Layer 2 interface
 */
typedef Layer2Interface *(*Layer2_Create_Func) (const char *conf, int flags,
						Trace * t);

#define FLAG_B_TUNNEL_NOQUEUE (1<<0)
#define FLAG_B_TPUARTS_ACKGROUP (1<<1)
#define FLAG_B_TPUARTS_ACKINDIVIDUAL (1<<2)
#define FLAG_B_TPUARTS_DISCH_RESET (1<<3)
#define FLAG_B_EMI_NOQUEUE (1<<4)

#endif
