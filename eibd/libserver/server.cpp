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

#include <unistd.h>
#include "server.h"
#include "client.h"
#include "flagpole.h"
#include "nonblockio.h"

Server::~Server ()
{
  TRACEPRINTF (t, 8, this, "StopServer");
  Stop ();
  for (int i = 0; i < connections (); i++)
    connections[i]->StopDelete ();
  while (connections () != 0)
    std::this_thread::yield ();

  if (fd != -1)
    close (fd);
  TRACEPRINTF (t, 8, this, "Server ended");
}

bool
Server::deregister (ClientConnection * con)
{
  for (unsigned i = 0; i < connections (); i++)
    if (connections[i] == con)
      {
	connections.deletepart (i, 1);
	return 1;
      }
  return 0;
}

Server::Server (Layer3 * layer3, Trace * tr)
{
  t = tr;
  l3 = layer3;
  fd = -1;
}

void
Server::Run (FlagpolePtr pole)
{
  while (!pole->raised (Flag_Stop))
    {
      int cfd;
      cfd = FlagpoleIO::accept (pole, fd, 0, 0);
      if (cfd != -1)
	{
	  TRACEPRINTF (t, 8, this, "New Connection");
	  setupConnection (cfd);
	  ClientConnection *c = new ClientConnection (this, l3, t, cfd);
	  connections.setpart (&c, connections (), 1);
	  c->Start ();
	}
    }
}

void
Server::setupConnection (int cfd)
{
}
