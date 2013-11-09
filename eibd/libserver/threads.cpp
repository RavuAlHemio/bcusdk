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

#include "types.h"
#include "threads.h"
#include "event.h"

void *
Thread::ThreadWrapper (void *arg)
{
  Thread *thd = (Thread *) arg;
  thd->Run (thd->should_stop.get_future ().share ());
  thd->is_done.store (true);
  return NULL;
}

Thread::Thread (Runable * o, THREADENTRY t)
  : thread (NULL), obj (o), entry (t), should_stop (),
    is_done ()
{
}

Thread::~Thread ()
{
  Stop ();
}

void
Thread::Stop ()
{
  if (thread == NULL)
    return;
  should_stop.set_value ();

  if (thread->joinable ())
    thread->join ();
  delete thread;
  thread = NULL;
}

void
Thread::StopDelete ()
{
  should_stop.set_value ();

  if (thread == NULL)
    return;
  thread->detach ();
  delete thread;
  thread = NULL;
}

void
Thread::Start ()
{
  if (thread != NULL)
    {
      if (!is_done)
	return;
      Stop ();
    }
  thread = new std::thread (ThreadWrapper, this);
}

void
Thread::Run (std::shared_future<void> stop)
{
  (obj->*entry) (stop);
}
