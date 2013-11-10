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

#include "nonblockio.h"
#include "flagpole.h"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

FlagpoleIO::NonBlockGuard::NonBlockGuard (int fd)
  : m_fd (fd), m_oldflags (-1)
{
  m_oldflags = fcntl (fd, F_GETFD);
  if (m_oldflags < 0)
    {
      // nope
      return;
    }
  if (fcntl (fd, F_SETFD, m_oldflags | O_NONBLOCK) < 0)
    {
      // nooope
      m_oldflags = -1;
    }
}

FlagpoleIO::NonBlockGuard::~NonBlockGuard ()
{
  if (m_oldflags < 0)
    {
      // didn't work out
      return;
    }
  fcntl (m_fd, F_SETFD, m_oldflags);
}

int
FlagpoleIO::accept (FlagpolePtr fp, int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  NonBlockGuard guard (sockfd);

  while (!fp->raised (Flag_Stop))
    {
      int ret = ::accept (sockfd, addr, addrlen);
      if (ret >= 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
        {
          return ret;
        }
      std::this_thread::yield ();
    }

  // timeout
  errno = EINTR;
  return -1;
}

ssize_t
FlagpoleIO::read (FlagpolePtr fp, int fd, void *buf, size_t count)
{
  NonBlockGuard guard (fd);

  while (!fp->raised (Flag_Stop))
    {
      ssize_t ret = ::read (fd, buf, count);
      if (ret >= 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
        {
          return ret;
        }
      std::this_thread::yield ();
    }

  // timeout
  errno = EINTR;
  return -1;
}

ssize_t
FlagpoleIO::write (FlagpolePtr fp, int fd, const void *buf, size_t count)
{
  NonBlockGuard guard (fd);

  while (!fp->raised (Flag_Stop))
    {
      ssize_t ret = ::write (fd, buf, count);
      if (ret >= 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
        {
          return ret;
        }
      std::this_thread::yield ();
    }

  // timeout
  errno = EINTR;
  return -1;
}
