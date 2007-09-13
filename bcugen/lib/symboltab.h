/*
    BCU SDK bcu development enviroment
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#ifndef SYMBOL_TAB_H
#define SYMBOL_TAB_H

#include "common.h"

typedef struct
{
  String Name;
  String Id;
  int lineno;
} Symbol;

const String & NewSymbol (const String & name, int lineno);
const String & NewSymbolAn (int lineno);
const Symbol & FindSymbol (const String & name, int lineno);

#endif
