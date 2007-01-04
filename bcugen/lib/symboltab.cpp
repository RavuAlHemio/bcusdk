/*
    BCU SDK bcu development enviroment
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

#include <stdio.h>
#include "symboltab.h"

static Array < Symbol > syms;
static int SymId = 0;

const String &
NewSymbol (const String & name, int lineno)
{
  char buf[20];
  int i;
  for (i = 0; i < syms (); i++)
    if (syms[i].Name == name)
      die (_("Line %d: symbol %s already defined at line %d"), lineno,
	   name (), syms[i].lineno);
  i = syms ();
  syms.resize (i + 1);
  syms[i].Name = name;
  syms[i].lineno = lineno;
  sprintf (buf, "id%d", SymId++);
  syms[i].Id = buf;
  return syms[i].Id;
}

const String &
NewSymbolAn (int lineno)
{
  int i;
  char buf[20];
  i = syms ();
  syms.resize (i + 1);
  syms[i].Name = "";
  syms[i].lineno = lineno;
  sprintf (buf, "id%d", SymId++);
  syms[i].Id = buf;
  return syms[i].Id;
}

const Symbol &
FindSymbol (const String & name, int lineno)
{
  int i;
  for (i = 0; i < syms (); i++)
    if (syms[i].Name == name)
      return syms[i];
  die (_("Undefined symbol %s at line %d"), name (), lineno);
}
