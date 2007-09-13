/*
    EIBD client library - internals
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

#ifndef EIBCLIENT_INT_H
#define EIBCLIENT_INT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "eibtypes.h"

/** unsigned char */
typedef uint8_t uchar;

/** EIB Connection internal */
struct _EIBConnection
{
  int (*complete) (EIBConnection *);
  /** file descriptor */
  int fd;
  unsigned readlen;
  /** buffer */
  uchar *buf;
  /** buffer size */
  unsigned buflen;
  /** used buffer */
  unsigned size;
  struct
  {
    int len;
    uint8_t *buf;
    int16_t *ptr1;
    uint8_t *ptr2;
    uint8_t *ptr3;
    uint16_t *ptr4;
    eibaddr_t *ptr5;
  } req;
};

/** extracts TYPE code of an eibd packet */
#define EIBTYPE(con) (((con)->buf[0]<<8)|((con)->buf[1]))
/** sets TYPE code for an eibd packet*/
#define EIBSETTYPE(buf,type) do{(buf)[0]=(type>>8)&0xff;(buf)[1]=(type)&0xff;}while(0)

/** set EIB address */
#define EIBSETADDR(buf,type) do{(buf)[0]=(type>>8)&0xff;(buf)[1]=(type)&0xff;}while(0)

int _EIB_SendRequest (EIBConnection * con, unsigned int size, uchar * data);
int _EIB_CheckRequest (EIBConnection * con, int block);
int _EIB_GetRequest (EIBConnection * con);

#endif
