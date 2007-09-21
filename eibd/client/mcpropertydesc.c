/*
    EIBD client library
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License, 
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any 
    restriction coming from the use of this file. (The General Public 
    License restrictions do apply in other respects; for example, they 
    cover modification of the file, and distribution when not linked into 
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "eibclient.h"
#include "eibclient-int.h"

EIBC_COMPLETE (EIB_MC_PropertyDesc,
  EIBC_GETREQUEST
  EIBC_CHECKRESULT (EIB_MC_PROP_DESC, 6)
  EIBC_RETURN_PTR2 (2)
  EIBC_RETURN_PTR4 (3)
  EIBC_RETURN_PTR3 (5)
  EIBC_RETURN_OK
)

int
EIB_MC_PropertyDesc_async (EIBConnection * con, uint8_t obj, uint8_t property,
			   uint8_t * type, uint16_t * max_nr_of_elem,
			   uint8_t * access)
{
  EIBC_INIT_SEND (4)
  EIBC_PTR2 (type)
  EIBC_PTR4 (max_nr_of_elem)
  EIBC_PTR3 (access)
  EIBC_UINT8 (obj, 2)
  EIBC_UINT8 (property, 3)
  EIBC_SEND (EIB_MC_PROP_DESC)
  EIBC_INIT_COMPLETE (EIB_MC_PropertyDesc)
}

int
EIB_MC_PropertyDesc (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint8_t * type, uint16_t * max_nr_of_elem,
		     uint8_t * access)
{
  if (EIB_MC_PropertyDesc_async
      (con, obj, property, type, max_nr_of_elem, access) == -1)
    return -1;
  return EIBComplete (con);
}
