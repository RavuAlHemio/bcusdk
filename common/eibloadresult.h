/*
    EIBD client library
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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

#ifndef EIB_LOAD_RESULT_H
#define EIB_LOAD_RESULT_H

typedef enum
{
  IMG_UNKNOWN_ERROR = 0,
  IMG_UNRECOG_FORMAT,
  IMG_INVALID_FORMAT,
  IMG_NO_BCUTYPE,
  IMG_UNKNOWN_BCUTYPE,
  IMG_NO_CODE,
  IMG_NO_SIZE,
  IMG_LODATA_OVERFLOW,
  IMG_HIDATA_OVERFLOW,
  IMG_TEXT_OVERFLOW,
  IMG_IMAGE_LOADABLE,
} BCU_LOAD_RESULT;

#endif
