/*
    BCU SDK bcu development enviroment
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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

#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"

String HexDump (CArray data);

typedef enum
{
  S_Invalid,
  S_Unknown,
  S_BCUType,
  S_Code,
  S_StringParameter,
  S_IntParameter,
  S_FloatParameter,
  S_ListParameter,
  S_GroupObject,
  S_BCU1Size,
  S_BCU2Start,
} STR_Type;

class STR_Stream
{
public:
  virtual ~ STR_Stream ()
  {
  }
  static STR_Stream *fromArray (const CArray & c);
  virtual CArray toArray () = 0;
  virtual STR_Type getType () = 0;
  virtual String decode () = 0;
};

class STR_Invalid:public STR_Stream
{
public:
  CArray data;

  STR_Invalid ();
  STR_Invalid (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_Invalid;
  }
  String decode ();
};

class STR_Unknown:public STR_Stream
{
public:
  uint16_t type;
  CArray data;

    STR_Unknown ();
    STR_Unknown (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_Unknown;
  }
  String decode ();
};

class STR_BCUType:public STR_Stream
{
public:
  uint16_t bcutype;

  STR_BCUType ();
  STR_BCUType (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_BCUType;
  }
  String decode ();
};
class STR_Code:public STR_Stream
{
public:
  CArray code;

  STR_Code ();
  STR_Code (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_Code;
  }
  String decode ();
};
class STR_StringParameter:public STR_Stream
{
public:
  uint16_t addr;
  uint16_t length;
  String name;

    STR_StringParameter ();
    STR_StringParameter (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_StringParameter;
  }
  String decode ();
};
class STR_ListParameter:public STR_Stream
{
public:
  uint16_t addr;
  String name;
    Array < String > elements;

    STR_ListParameter ();
    STR_ListParameter (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_ListParameter;
  }
  String decode ();
};
class STR_IntParameter:public STR_Stream
{
public:
  uint16_t addr;
  int8_t type;
  String name;

    STR_IntParameter ();
    STR_IntParameter (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_IntParameter;
  }
  String decode ();
};
class STR_FloatParameter:public STR_Stream
{
public:
  uint16_t addr;
  String name;

    STR_FloatParameter ();
    STR_FloatParameter (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_FloatParameter;
  }
  String decode ();
};
class STR_GroupObject:public STR_Stream
{
public:
  uchar no;
  String name;

    STR_GroupObject ();
    STR_GroupObject (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_GroupObject;
  }
  String decode ();
};
class STR_BCU1Size:public STR_Stream
{
public:
  uint16_t textsize;
  uint16_t stacksize;
  uint16_t datasize;
  uint16_t bsssize;

    STR_BCU1Size ();
    STR_BCU1Size (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_BCU1Size;
  }
  String decode ();
};
class STR_BCU2Start:public STR_Stream
{
public:
  uint16_t initaddr;
  uint16_t runaddr;
  uint16_t saveaddr;

    STR_BCU2Start ();
    STR_BCU2Start (const CArray & str);
  CArray toArray ();
  STR_Type getType ()
  {
    return S_BCU2Start;
  }
  String decode ();
};

class Image
{
public:
  Array < STR_Stream * >str;

  Image ();
  virtual ~ Image ();

  static Image *fromArray (CArray c);
  CArray toArray ();
  String decode ();
  bool isValid ();

  int findStreamNumber (STR_Type t);
  STR_Stream *findStream (STR_Type t);
};

#endif
