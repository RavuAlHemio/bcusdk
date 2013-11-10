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

#ifndef THREADS_H
#define THREADS_H

#include <memory>
#include <atomic>
#include <thread>

class Flagpole;

typedef std::shared_ptr<Flagpole> FlagpolePtr;

enum
{
  Flag_Stop = 0
};

/** interface for a class started by a thread */
class Runable
{
public:
};

/** pointer to an thread entry point in Runable
 * the thread should exit, if stopcond can be deceremented
 */
typedef void (Runable::*THREADENTRY) (FlagpolePtr pole);

/** implements a Thread */
class Thread
{
  /** C entry point for the threads */
  static void *ThreadWrapper (void *arg);
  /** the thread */
  std::thread * thread;
  /** object to run */
  Runable *obj;
  /** entry point */
  THREADENTRY entry;
  /** is done */
  std::atomic<bool> is_done;

protected:
  /** main function of the thread
   * @param stop if stop has been triggered, the routine should exit
   */
  virtual void Run (FlagpolePtr pole);
  /** flagpole for this thread */
  FlagpolePtr flagpole;
public:
  /** create a thread
   * if o and t are not present, Run is runned, which has to be replaced
   * @param o Object to run
   * @param t Entry point
   */
  Thread (Runable * o = NULL, THREADENTRY t = NULL);
  virtual ~Thread ();

  /** starts the thread*/
  void Start ();
  /** stops the thread, if it is running */
  void Stop ();
  /** stops the thread and delete it asynchronous */
  void StopDelete ();
};


#endif
