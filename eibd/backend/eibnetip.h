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

#ifndef EIBNETIP_H
#define EIBNETIP_H

#include <netinet/in.h>
#include "common.h"
#include "lpdu.h"

#define CONNECTION_REQUEST 0x0205
#define CONNECTION_RESPONSE 0x0206
#define CONNECTIONSTATE_REQUEST 0x0207
#define CONNECTIONSTATE_RESPONSE 0x0208
#define DISCONNECT_REQUEST 0x0209
#define DISCONNECT_RESPONSE 0x020A

#define TUNNEL_REQUEST 0x0420
#define TUNNEL_RESPONSE 0x0421

#define DEVICE_CONFIGURATION_REQUEST 0x0310
#define DEVICE_CONFIGURATION_RESPONSE 0x0311

#define ROUTING_INDICATION 0x0530

/** resolve host name */
int GetHostIP (struct sockaddr_in *sock, const char *Name);
/** gets source address for a route */
int GetSourceAddress (const struct sockaddr_in *dest,
		      struct sockaddr_in *src);
/** convert a to EIBnet/IP format */
CArray IPtoEIBNetIP (const struct sockaddr_in *a);
/** convert EIBnet/IP IP Address to a */
int EIBnettoIP (const CArray & buf, struct sockaddr_in *a);
/** convert L_Data_PDU to CEMI frame */
CArray L_Data_ToCEMI (uchar code, const L_Data_PDU & p);
/** create L_Data_PDU out of a CEMI frame */
L_Data_PDU *CEMI_to_L_Data (const CArray & data);

/** represents a EIBnet/IP packet */
class EIBNetIPPacket
{

public:
  /** service code*/
  int service;
  /** payload */
  CArray data;

    EIBNetIPPacket ();
    /** create from character array */
  static EIBNetIPPacket *fromPacket (const CArray & c);
  /** convert to character array */
  CArray ToPacket () const;
    virtual ~ EIBNetIPPacket ()
  {
  }
};

/** represents a EIBnet/IP packet to send*/
struct _EIBNetIP_Send
{
  /** packat */
  EIBNetIPPacket data;
  /** destination address */
  struct sockaddr_in addr;
};

/** EIBnet/IP socket */
class EIBNetIPSocket:private Thread
{
  /** debug output */
  Trace *t;
  /** input queue */
    Queue < struct _EIBNetIP_Send >inqueue;
    /** output queue */
    Queue < EIBNetIPPacket > outqueue;
    /** semaphore for inqueue */
  pth_sem_t insignal;
  /** semaphore for outqueue */
  pth_sem_t outsignal;
  /** event to wait for outqueue */
  pth_event_t getwait;
  /** multicast address */
  struct ip_mreq maddr;
  /** file descriptor */
  int fd;
  /** multicast in use */
  int multicast;

  void Run (pth_sem_t * stop);
public:
    EIBNetIPSocket (struct sockaddr_in bindaddr, bool reuseaddr, Trace * tr);
    virtual ~ EIBNetIPSocket ();

    /** enables multicast */
  void SetMulticast (struct ip_mreq multicastaddr);
  /** sends a packet */
  void Send (EIBNetIPPacket p);
  /** waits for an packet; aborts if stop occurs */
  EIBNetIPPacket *Get (pth_event_t stop);

  /** default send address */
  struct sockaddr_in sendaddr;
  /** addres to accept packets from */
  struct sockaddr_in recvaddr;
  /** accept all packets*/
  bool recvall;
};

#endif
