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

#ifndef LOWLEVEL_H
#define LOWLEVEL_H

#include "common.h"

/** implements interface for a Driver to send packets for the EMI1/2 driver */
class LowLevelDriverInterface
{
public:
  virtual ~ LowLevelDriverInterface ()
  {
  }

  /** sends a EMI frame asynchronous */
  virtual void Send_Packet (CArray l) = 0;
  /** all frames sent ? */
  virtual bool Send_Queue_Empty () = 0;
  /** returns semaphore, which becomes 1, if all frames are sent */
  virtual pth_sem_t *Send_Queue_Empty_Cond () = 0;
  /** waits for the next EMI frame
   * @param stop return NULL, if stop occurs
   * @return returns EMI frame or NULL
   */
  virtual CArray *Get_Packet (pth_event_t stop) = 0;

  /** resets the connection */
  virtual void SendReset () = 0;
  /** indicate, if connections works */
  virtual bool Connection_Lost () = 0;
};

#endif
