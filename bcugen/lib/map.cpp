/*
    BCU SDK bcu development enviroment
    Copyright (C) 2005 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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

#include "map.h"
#include "scanner.h"

#define MAP(A,B) if(s==#B) return GO_##B;

GroupType
Map_GroupType (const String & s)
{
#include "GroupObjectType.lst"
  parserError (_("unkown GroupType %s"), s ());
}

#undef MAP
#define MAP(A,B) if(s==#B) return BCU_##B;

BCUType
Map_BCUType (const String & s)
{
#include "BCUType.lst"
  parserError (_("unkown BCUType %s"), s ());
}

#undef MAP
#define MAP(A,B) if(s==#B) return B;

PropertyType
Map_PropertyType (const String & s)
{
#include "PropertyType.lst"
  parserError (_("unkown Property Type %s"), s ());
}


#undef MAP
#define MAP(A,B) if(s==#B) return A;

itype
Map_ProfileID (const String & s)
{

  parserError (_("unkown Profile ID %s"), s ());
}

ftype
Map_DPTType (const String & s)
{
#include "DPT.lst"
  parserError (_("unkown DPT Type %s"), s ());
}

itype
Map_PropertyID (const String & s)
{
#include "PropertyID.lst"
  parserError (_("unkown Property ID %s"), s ());
}

itype
Map_ObjectType (const String & s)
{

  parserError (_("unkown ObjectType %s"), s ());
}

#undef MAP
#define MAP(A,B) if(s==A) return #B;


const char *
unMap_GroupType (GroupType s)
{

#include "GroupObjectType.lst"
  die (_("unknown value %d"), s);
}

#undef MAP
#define MAP(A,B) if(s==BCU_##B) return #B;

const char *
unMap_BCUType (BCUType s)
{

#include "BCUType.lst"
  die (_("unknown value %d"), s);
}

#undef MAP
#define MAP(A,B) if(s==A) return #B;

const char *
unMap_PropertyType (PropertyType s)
{
#include "PropertyType.lst"
  die (_("unknown value %d"), s);
}


String
escapeString (const String & s)
{
  const char *a = s ();
  String d;
  char buf[2] = { 0, 0 };
  while (*a)
    {
      switch (*a)
	{
	case '"':
	  d += "\\\"";
	  break;
	case '\\':
	  d += "\\\\";
	case '\n':
	  d += "\\n";
	  break;
	case '\r':
	  d += "\\r";
	  break;
	case '\t':
	  d += "\\t";
	  break;
	case '\f':
	  d += "\\f";
	  break;
	default:
	  buf[0] = *a;
	  d += buf;
	}
      a++;
    }
  return d;
}