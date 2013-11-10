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

#ifndef EIBD_NONBLOCKIO_H
#define EIBD_NONBLOCKIO_H

#include "threads.h"

#include <sys/socket.h>

/** Flagpole-timeout I/O functions. */
class FlagpoleIO
{
public:
  /** A guard to make a file descriptor temporarily non-blocking. */
  class NonBlockGuard
  {
  private:
    int m_fd;  /**< The file descriptor. */
    int m_oldflags;  /**< The flags before the guard came into effect. */
  public:
    NonBlockGuard (int fd);
    ~NonBlockGuard ();
  };

  static int accept (FlagpolePtr fp, int sockfd, struct sockaddr *addr, socklen_t *addrlen);
  static ssize_t read (FlagpolePtr fp, int fd, void *buf, size_t count);
  static ssize_t write (FlagpolePtr fp, int fd, const void *buf, size_t count);
};

#endif
