/*
    EIBD eib bus access and management daemon
    Copyright (C) 2013 Ondrej Hosek <ondra.hosek@gmail.com>

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

#include "flagpole.h"

#include <cassert>

Flagpole::Flagpole ()
  : m_raisedflags (0), m_mutex (), m_condvar ()
{
}

bool
Flagpole::raised (size_t flag)
{
  assert (flag < 64);

  std::lock_guard<decltype (m_mutex)> guard (m_mutex);
  return (m_raisedflags & (1 << flag)) != 0;
}

void
Flagpole::raise (size_t flag)
{
  assert (flag < 64);

  {
    std::lock_guard<decltype (m_mutex)> guard (m_mutex);
    m_raisedflags |= (1 << flag);
  }
  m_condvar.notify_all ();
}

void
Flagpole::drop (size_t flag)
{
  assert (flag < 64);

  {
    std::lock_guard<decltype (m_mutex)> guard (m_mutex);
    m_raisedflags &= ~(1 << flag);
  }
  m_condvar.notify_all ();
}

void
Flagpole::wait ()
{
  std::unique_lock<decltype (m_mutex)> guard (m_mutex);
#if 0
  auto curVal = m_raisedflags;
  do
    {
      m_condvar.wait (guard);
    }
  while (curVal == m_raisedflags);
#else
  m_condvar.wait (guard);
#endif
}

template <class Rep, class Period>
std::cv_status
Flagpole::wait_for (const std::chrono::duration<Rep, Period>& rel_time)
{
  return m_condvar.wait (rel_time);
}

template <class Clock, class Duration>
std::cv_status
Flagpole::wait_until (const std::chrono::time_point<Clock, Duration>& timeout_time)
{
  return m_condvar.wait_until (timeout_time);
}
