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

#include "threads.h"

void *
Thread::ThreadWrapper (void *arg)
{
  ((Thread *) arg)->Run (&((Thread *) arg)->should_stop);
  pth_exit (0);
  return 0;
}

Thread::Thread (Runable * o, THREADENTRY t)
{
  obj = o;
  entry = t;
  pth_sem_init (&should_stop);
  tid = 0;
}

Thread::~Thread ()
{
  Stop ();
}

void
Thread::Stop ()
{
  if (!tid)
    return;
  pth_sem_inc (&should_stop, TRUE);
  pth_join (tid, 0);
  tid = 0;
}

void
Thread::Start ()
{
  if (tid)
    {
      pth_attr_t a = pth_attr_of (tid);
      int state;
      pth_attr_get (a, PTH_ATTR_STATE, &state);
      pth_attr_destroy (a);
      if (state != PTH_STATE_DEAD)
	return;
      Stop ();
    }
  tid = pth_spawn (PTH_ATTR_DEFAULT, &ThreadWrapper, this);
}

void
Thread::Run (pth_sem_t * stop)
{
  (obj->*entry) (stop);
}
