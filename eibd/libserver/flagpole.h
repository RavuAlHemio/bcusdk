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

#ifndef EIBD_FLAGPOLE_H
#define EIBD_FLAGPOLE_H

#include <mutex>
#include <condition_variable>
#include <cstdint>

/** A pole for raising flags. */
class Flagpole
{
private:
  /** Bitfield of raised flags. */
  uint64_t m_raisedflags;

  /** The mutex synchronizing the access to this flagpole. */
  std::mutex m_mutex;

  /** The condition variable for waiting on the flagpole. */
  std::condition_variable m_condvar;

public:
  /** Constructs the flagpole with no flags raised. */
  Flagpole ();

  /** Returns whether the given flag is raised. */
  bool raised (size_t flag);

  /** Raises the given flag. */
  void raise (size_t flag);

  /** Drops the given flag. */
  void drop (size_t flag);

  /** Waits until a flag is raised or dropped. */
  void wait ();

  /** Waits until a flag is raised, dropped or the operation times out. */
  template <class Rep, class Period>
  std::cv_status wait_for (const std::chrono::duration<Rep, Period>& rel_time);

  /** Waits until a flag is raised, dropped or the operation times out. */
  template <class Clock, class Duration>
  std::cv_status wait_until (const std::chrono::time_point<Clock, Duration>& timeout_time);
};

#endif
