EIBC_LICENSE(
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
)

EIBC_COMPLETE (EIB_LoadImage,
  EIBC_GETREQUEST
  EIBC_CHECKRESULT (EIB_LOAD_IMAGE, 4)
  EIBC_RETURN_UINT16 (2)
)

EIBC_ASYNC (EIB_LoadImage, ARG_INBUF (image, ARG_NONE),
  EIBC_INIT_SEND (2)
  EIBC_SEND_BUF (image)
  EIBC_SEND (EIB_LOAD_IMAGE)
  EIBC_INIT_COMPLETE (EIB_LoadImage)
)