/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2007 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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

#ifndef C_PEI16_H
#define C_PEI16_H

#include "emi1.h"
#include "bcu1.h"

#define PEI16_URL "bcu1:/dev/eib\n"
#define PEI16_DOC "bcu1 connects using the PEI16 Protocoll over a BCU to the bus (using a kernel driver)\n\n"

#define PEI16_PREFIX "bcu1"
#define PEI16_CREATE PEI16_Create

inline Layer2Interface *
PEI16_Create (const char *dev, Trace * t)
{
  return new EMI1Layer2Interface (new BCU1DriverLowLevelDriver (dev, t), t);
}

#endif
