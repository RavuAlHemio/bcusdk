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

#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <asm/types.h>
#include "eibnetip.h"
#if 0
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif

int
GetHostIP (struct sockaddr_in *sock, const char *Name)
{
  struct hostent *h;
  if (!Name)
    return 0;
  memset (sock, 0, sizeof (*sock));
  h = gethostbyname (Name);
  if (!h)
    return 0;
  sock->sin_family = h->h_addrtype;
  sock->sin_addr.s_addr = (*((unsigned long *) h->h_addr_list[0]));
  return 1;
}

#if 0
typedef struct
{
  struct nlmsghdr n;
  struct rtmsg r;
  char data[1000];
} r_req;

int
GetSourceAddress (const struct sockaddr_in *dest, struct sockaddr_in *src)
{
  int s;
  int l;
  r_req req;
  struct rtattr *a;
  memset (src, 0, sizeof (*src));
  s = socket (PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
  if (s == -1)
    return 0;
  req.n.nlmsg_len =
    NLMSG_SPACE (sizeof (req.r)) + RTA_LENGTH (sizeof (*dest));
  req.n.nlmsg_flags = NLM_F_REQUEST;
  req.n.nlmsg_type = RTM_GETROUTE;
  req.r.rtm_family = AF_INET;
  req.r.rtm_dst_len = 32;
  a = (rtattr *) ((char *) &req + NLMSG_SPACE (sizeof (req.r)));
  a->rta_type = RTA_DST;
  a->rta_len = RTA_LENGTH (sizeof (dest->sin_addr.s_addr));
  memcpy (RTA_DATA (a), &dest->sin_addr.s_addr,
	  sizeof (dest->sin_addr.s_addr));
  if (write (s, &req, req.n.nlmsg_len) < 0)
    return 0;
  if (read (s, &req, sizeof (req)) < 0)
    return 0;
  close (s);
  if (req.n.nlmsg_type == NLMSG_ERROR)
    return 0;
  l = ((struct nlmsghdr *) &req)->nlmsg_len;
  while (RTA_OK (a, l))
    {
      if (a->rta_type == RTA_PREFSRC
	  && RTA_PAYLOAD (a) == sizeof (src->sin_addr.s_addr))
	{
	  src->sin_family = AF_INET;
	  memcpy (&src->sin_addr.s_addr, RTA_DATA (a), RTA_PAYLOAD (a));
	  return 1;
	}
      a = RTA_NEXT (a, l);
    }
  return 0;
}
#endif

CArray
L_Data_ToCEMI (uchar code, const L_Data_PDU & l1)
{
  uchar c;
  CArray pdu;
  assert (l1.data () >= 1);
  assert (l1.data () < 0xff);
  assert ((l1.hopcount & 0xf8) == 0);

  switch (l1.prio)
    {
    case PRIO_LOW:
      c = 0x3;
      break;
    case PRIO_NORMAL:
      c = 0x1;
      break;
    case PRIO_URGENT:
      c = 0x02;
      break;
    case PRIO_SYSTEM:
      c = 0x00;
      break;
    }
  pdu.resize (l1.data () + 9);
  pdu[0] = code;
  pdu[1] = 0x00;
  pdu[2] = 0x30 | (c << 2) | (l1.data () - 1 <= 0x0f ? 0x80 : 0x00);
  pdu[3] =
    (l1.AddrType ==
     GroupAddress ? 0x80 : 0x00) | ((l1.hopcount & 0x7) << 4) | 0x0;
  pdu[4] = (l1.source >> 8) & 0xff;
  pdu[5] = (l1.source) & 0xff;
  pdu[6] = (l1.dest >> 8) & 0xff;
  pdu[7] = (l1.dest) & 0xff;
  pdu[8] = l1.data () - 1;
  pdu.setpart (l1.data.array (), 9, l1.data ());
  return pdu;
}

L_Data_PDU *
CEMI_to_L_Data (const CArray & data)
{
  L_Data_PDU c;
  if (data () < 2)
    return 0;
  unsigned start = data[1] + 2;
  if (data () < 7 + start)
    return 0;
  if (data () != 7 + start + data[6 + start] + 1)
    return 0;
  c.source = (data[start + 2] << 8) | (data[start + 3]);
  c.dest = (data[start + 4] << 8) | (data[start + 5]);
  c.data.set (data.array () + start + 7, data[6 + start] + 1);
  c.repeated = (data[start] & 0x20) ? 0 : 1;
  switch ((data[start] >> 2) & 0x3)
    {
    case 0:
      c.prio = PRIO_SYSTEM;
      break;
    case 1:
      c.prio = PRIO_URGENT;
      break;
    case 2:
      c.prio = PRIO_NORMAL;
      break;
    case 3:
      c.prio = PRIO_LOW;
      break;
    }
  c.hopcount = (data[start + 1] >> 4) & 0x07;
  c.AddrType = (data[start + 1] & 0x80) ? GroupAddress : IndividualAddress;
  if (!data[start] & 0x80 && data[start + 1] & 0x0f)
    return 0;
  return new L_Data_PDU (c);
}

EIBNetIPPacket::EIBNetIPPacket ()
{
  service = 0;
}

EIBNetIPPacket *
EIBNetIPPacket::fromPacket (const CArray & c)
{
  EIBNetIPPacket *p;
  unsigned len;
  if (c () < 6)
    return 0;
  if (c[0] != 0x6 || c[1] != 0x10)
    return 0;
  len = (c[4] << 8) | c[5];
  if (len != c ())
    return 0;
  p = new EIBNetIPPacket;
  p->service = (c[2] << 8) | c[3];
  p->data.set (c.array () + 6, len - 6);
  return p;
}

CArray
EIBNetIPPacket::ToPacket () const
{
  CArray c;
  c.resize (6 + data ());
  c[0] = 0x06;
  c[1] = 0x10;
  c[2] = (service >> 8) & 0xff;
  c[3] = (service) & 0xff;
  c[4] = ((data () + 6) >> 8) & 0xff;
  c[5] = ((data () + 6)) & 0xff;
  c.setpart (data, 6);
  return c;
}

CArray
IPtoEIBNetIP (const struct sockaddr_in * a)
{
  CArray buf;
  buf.resize (8);
  buf[0] = 0x08;
  buf[1] = 0x01;
  buf[2] = (ntohl (a->sin_addr.s_addr) >> 24) & 0xff;
  buf[3] = (ntohl (a->sin_addr.s_addr) >> 16) & 0xff;
  buf[4] = (ntohl (a->sin_addr.s_addr) >> 8) & 0xff;
  buf[5] = (ntohl (a->sin_addr.s_addr) >> 0) & 0xff;
  buf[6] = (ntohs (a->sin_port) >> 8) & 0xff;
  buf[7] = (ntohs (a->sin_port) >> 0) & 0xff;
  return buf;
}

int
EIBnettoIP (const CArray & buf, struct sockaddr_in *a)
{
  int ip, port;
  memset (a, 0, sizeof (*a));
  if (buf[0] != 0x8 || buf[1] != 0x1)
    return 1;
  ip = (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | (buf[5]);
  port = (buf[6] << 8) | (buf[7]);
  a->sin_family = AF_INET;
  a->sin_port = htons (port);
  a->sin_addr.s_addr = htonl (ip);
  return 0;
}

EIBNetIPSocket::EIBNetIPSocket (struct sockaddr_in bindaddr, bool reuseaddr,
				Trace * tr)
{
  int i;
  t = tr;
  t->TracePrintf (0, this, "Open");
  multicast = 0;
  pth_sem_init (&insignal);
  pth_sem_init (&outsignal);
  getwait = pth_event (PTH_EVENT_SEM, &outsignal);
  memset (&maddr, 0, sizeof (maddr));
  memset (&sendaddr, 0, sizeof (sendaddr));
  memset (&recvaddr, 0, sizeof (recvaddr));
  recvall = 0;

  fd = socket (AF_INET, SOCK_DGRAM, 0);
  if (fd == -1)
    throw Exception (DEV_OPEN_FAIL);

  if (reuseaddr)
    {
      i = 1;
      if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i)) == -1)
	throw Exception (DEV_OPEN_FAIL);
    }
  if (bind (fd, (struct sockaddr *) &bindaddr, sizeof (bindaddr)) == -1)
    throw Exception (DEV_OPEN_FAIL);

  Start ();
  t->TracePrintf (0, this, "Openend");
}

EIBNetIPSocket::~EIBNetIPSocket ()
{
  t->TracePrintf (0, this, "Close");
  Stop ();
  pth_event_free (getwait, PTH_FREE_THIS);
  if (fd != -1)
    {
      if (multicast)
	setsockopt (fd, SOL_IP, IP_DROP_MEMBERSHIP, &maddr, sizeof (maddr));
      close (fd);
    }
}

void
EIBNetIPSocket::SetMulticast (struct ip_mreq multicastaddr)
{
  if (multicast)
    throw Exception (DEV_OPEN_FAIL);
  maddr = multicastaddr;
  if (setsockopt (fd, SOL_IP, IP_ADD_MEMBERSHIP, &maddr, sizeof (maddr)) ==
      -1)
    throw Exception (DEV_OPEN_FAIL);
  multicast = 1;
}

void
EIBNetIPSocket::Send (EIBNetIPPacket p)
{
  struct _EIBNetIP_Send s;
  t->TracePacket (1, this, "Send", p.data);
  s.data = p;
  s.addr = sendaddr;
  inqueue.put (s);
  pth_sem_inc (&insignal, 1);
}

EIBNetIPPacket *
EIBNetIPSocket::Get (pth_event_t stop)
{
  if (stop != NULL)
    pth_event_concat (getwait, stop, NULL);

  pth_wait (getwait);

  if (stop)
    pth_event_isolate (getwait);

  if (pth_event_status (getwait) == PTH_STATUS_OCCURRED)
    {
      pth_sem_dec (&outsignal);
      t->TracePacket (1, this, "Recv", outqueue.top ().data);
      return new EIBNetIPPacket (outqueue.get ());
    }
  else
    return 0;
}

void
EIBNetIPSocket::Run (pth_sem_t * stop1)
{
  int i;
  uchar buf[255];
  socklen_t rl;
  sockaddr_in r;
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  pth_event_t input = pth_event (PTH_EVENT_SEM, &insignal);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      pth_event_concat (stop, input, NULL);
      rl = sizeof (r);
      memset (&r, 0, sizeof (r));
      i =
	pth_recvfrom_ev (fd, buf, sizeof (buf), 0, (struct sockaddr *) &r,
			 &rl, stop);
      if (i > 0 && rl == sizeof (r))
	{
	  if (recvall || !memcpy (&r, &recvaddr, sizeof (&r)))
	    {
	      t->TracePacket (0, this, "Recv", i, buf);
	      EIBNetIPPacket *p =
		EIBNetIPPacket::fromPacket (CArray (buf, i));
	      if (p)
		{
		  outqueue.put (*p);
		  delete p;
		  pth_sem_inc (&outsignal, 1);
		}
	    }
	}
      pth_event_isolate (stop);
      if (!inqueue.isempty ())
	{
	  const struct _EIBNetIP_Send s = inqueue.top ();
	  CArray p = s.data.ToPacket ();
	  t->TracePacket (0, this, "Send", p);
	  i =
	    pth_sendto_ev (fd, p.array (), p (), 0,
			   (const struct sockaddr *) &s.addr, sizeof (s.addr),
			   stop);
	  if (i > 0)
	    {
	      pth_sem_dec (&insignal);
	      inqueue.get ();
	    }
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
  pth_event_free (input, PTH_FREE_THIS);
}
