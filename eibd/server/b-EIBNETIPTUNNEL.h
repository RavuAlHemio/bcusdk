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

#ifndef C_EIBNETIPTUNNEL_H
#define C_EIBNETIPTUNNEL_H

#include <stdlib.h>
#include "eibnettunnel.h"

#define EIBNETIPTUNNEL_URL "ipt:router-ip[:dest-port[:src-port[:nat-ip[:data-port]]]]]\n"
#define EIBNETIPTUNNEL_DOC "ipt connects with the EIBnet/IP Tunneling protocol over an EIBnet/IP gateway. The gateway must be so configured, that it routes the necessary addresses\n\n"

#define EIBNETIPTUNNEL_PREFIX "ipt"
#define EIBNETIPTUNNEL_CREATE eibnetiptunnel_Create

inline Layer2Interface *
eibnetiptunnel_Create (const char *dev, Trace * t)
{
  char *a = strdup (dev);
  char *b;
  char *c;
  char *d = 0;
  char *e;
  int dport;
  int dataport = -1;
  int sport;
  Layer2Interface *iface;
  if (!a)
    die ("out of memory");
  for (b = a; *b; b++)
    if (*b == ':')
      break;
  sport = 3672;
  if (*b == ':')
    {
      *b = 0;
      for (c = b + 1; *c; c++)
	if (*c == ':')
	  break;
      if (*c == ':')
	{
	  *c = 0;
	  for (d = c + 1; *d; d++)
	    if (*d == ':')
	      break;
	  if (*d == ':')
	    {
	      for (e = d + 1; *e; e++)
		if (*e == ':')
		  break;
	      if (*e == ':')
		dataport = atoi (e + 1);
	      *e = 0;
	      d++;
	    }
	  else
	    d = 0;

	  sport = atoi (c + 1);
	}
      dport = atoi (b + 1);
    }
  else
    dport = 3671;

  iface = new EIBNetIPTunnel (a, dport, sport, d, dataport, t);
  free (a);
  return iface;
}


#endif
