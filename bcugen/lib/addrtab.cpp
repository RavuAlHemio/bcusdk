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

#include "addrtab.h"

static int
hasAddress (Array < eibgaddr_t > &a, eibgaddr_t addr)
{
  int i;
  for (i = 0; i < a (); i++)
    if (a[i] == addr)
      return 1;
  return 0;
}

static int
addAddress (Array < eibgaddr_t > &a, eibgaddr_t addr)
{
  if (hasAddress (a, addr))
    return 0;
  a.resize (a () + 1);
  a[a () - 1] = addr;
  return 1;
}

void
BuildObjAddress (GroupObject & o, int &objno, BCUType b)
{
  int i, j;
  int first = 1;
  o.ObjAddress.resize (0);
  for (i = 0; i < o.ReceiveAddress (); i++)
    {
      if (o.ReceiveAddress[i] < 0 || o.ReceiveAddress[i] > 0xffff)
	die (_("line %d: invalid group address %X"), o.lineno,
	     o.ReceiveAddress[i]);
      if (addAddress (o.ObjAddress, o.ReceiveAddress[i]))
	if (!first)
	  warn (_
		("line %d: unsupported combination of group addresses, merging them"),
		o.lineno);
    }
  if (o.ObjAddress ())
    first = 0;
  for (i = 0; i < o.ReadAddress (); i++)
    {
      if (o.ReadAddress[i] < 0 || o.ReadAddress[i] > 0xffff)
	die (_("line %d: invalid group address %X"), o.lineno,
	     o.ReadAddress[i]);
      if (addAddress (o.ObjAddress, o.ReadAddress[i]))
	if (!first)
	  warn (_
		("line %d: unsupported combination of group addresses, merging them"),
		o.lineno);
    }
  if (o.ObjAddress ())
    first = 0;
  if (o.UpdateAddress () && b == BCU_bcu12)
    die (_("line %d: UpdateAddress not supported"), o.lineno);
  for (i = 0; i < o.UpdateAddress (); i++)
    {
      if (o.UpdateAddress[i] < 0 || o.UpdateAddress[i] > 0xffff)
	die (_("line %d: invalid group address %X"), o.lineno,
	     o.UpdateAddress[i]);
      if (addAddress (o.ObjAddress, o.UpdateAddress[i]))
	if (!first)
	  warn (_
		("line %d: unsupported combination of group addresses, merging them"),
		o.lineno);
    }
  if (o.ObjAddress ())
    first = 0;
  if (o.SendAddress_lineno)
    {
      if (o.SendAddress < 0 || o.SendAddress > 0xffff)
	die (_("line %d: invalid group address %X"), o.lineno, o.SendAddress);
      if (addAddress (o.ObjAddress, o.SendAddress))
	if (!first)
	  warn (_
		("line %d: unsupported combination of group addresses, merging them"),
		o.lineno);
    }
  if (o.ObjAddress ())
    o.ObjNo = objno++;
  else
    o.ObjNo = -1;
}

static int
AddrNo (Array < eibgaddr_t > a, eibgaddr_t ga)
{
  int i;
  for (i = 0; i < a (); i++)
    if (a[i] == ga)
      return i;
  die (_("internal error"));
}

void
printAddrTab (FILE * f, Device & d)
{
  int objno = 0, i, j, fn;
  Array < eibgaddr_t > addr;
  Array < bool > used;
  Array < int >ObjNo;
  Array < int >Addr;
  int maxs = 0;
  for (i = 0; i < d.GroupObjects (); i++)
    BuildObjAddress (d.GroupObjects[i], objno, d.BCU);
  for (i = 0; i < d.GroupObjects (); i++)
    for (j = 0; j < d.GroupObjects[i].ObjAddress (); j++)
      addAddress (addr, d.GroupObjects[i].ObjAddress[j]);
  addr.sort ();

  for (i = 0; i < d.GroupObjects (); i++)
    maxs += d.GroupObjects[i].ObjAddress ();
  used.resize (maxs);
  ObjNo.resize (maxs);
  Addr.resize (maxs);
  for (i = 0; i < maxs; i++)
    used[i] = 0;
  for (i = 0; i < d.GroupObjects (); i++)
    if (d.GroupObjects[i].SendAddress_lineno)
      {
	used[i] = 1;
	ObjNo[i] = d.GroupObjects[i].ObjNo;
	Addr[i] = AddrNo (addr, d.GroupObjects[i].SendAddress);
      }
  fn = 0;
  while (fn < maxs && used[fn])
    fn++;
  for (i = 0; i < d.GroupObjects (); i++)
    for (j = 0; j < d.GroupObjects[i].ObjAddress (); j++)
      if (!d.GroupObjects[i].SendAddress_lineno ||
	  d.GroupObjects[i].SendAddress != d.GroupObjects[i].ObjAddress[j])
	{
	  used[fn] = 1;
	  ObjNo[fn] = d.GroupObjects[i].ObjNo;
	  Addr[fn] = AddrNo (addr, d.GroupObjects[i].ObjAddress[j]);
	  fn++;
	  while (fn < maxs && used[fn])
	    fn++;
	}

  fprintf (f, "\t.section .addrtab\n");
  fprintf (f, "addrtab:\n");
  fprintf (f, "\t.byte %d\n", addr () + 1);
  fprintf (f, "\t.hword 0x%04X # physical address\n", d.PhysicalAddress);
  for (i = 0; i < addr (); i++)
    fprintf (f, "\t.hword 0x%04X\n", addr[i]);
  fprintf (f, "addrtab_end:\n");
  fprintf (f, "\t.section .assoctab\n");
  fprintf (f, "assoctab:\n");
  fprintf (f, "\t.byte %d\n", maxs);
  for (i = 0; i < maxs; i++)
    fprintf (f, "\t.byte %d, %d\n", Addr[i] + 1, ObjNo[i]);
  fprintf (f, "assoctab_end:\n");
  d.ObjCount = objno;
  if (d.ObjCount > 85)
    die (_("to many communication objectes"));
}

void
printPseudoAddrTab (FILE * f, Device & d)
{
  int i;
  if (!d.Test_Addr_Count_lineno)
    {
      d.Test_Addr_Count = d.GroupObjects ();
    }
  if (d.Test_Addr_Count < 0 || d.Test_Addr_Count > 0x7e)
    die (_("invalid value %d for Test_Addr_Count"), d.Test_Addr_Count);
  if (d.Test_Assoc_Count < 0 || d.Test_Assoc_Count > 0xff)
    die (_("invalid value %d for Test_Assoc_Count"), d.Test_Assoc_Count);
  if (!d.Test_Assoc_Count_lineno)
    {
      d.Test_Assoc_Count = d.GroupObjects ();
    }
  fprintf (f, "\t.section .addrtab\n");
  fprintf (f, "addrtab:\n");
  fprintf (f, "\t.byte %d\n", d.Test_Addr_Count + 1);
  fprintf (f, "\t.hword 0x0000 # physical address\n");
  for (i = 0; i < d.Test_Addr_Count; i++)
    fprintf (f, "\t.hword 0x0000\n");
  fprintf (f, "\t.section .assoctab\n");
  fprintf (f, "addrtab_end:\n");
  fprintf (f, "assoctab:\n");
  fprintf (f, "\t.byte %d\n", d.Test_Assoc_Count);
  for (i = 0; i < d.Test_Assoc_Count; i++)
    fprintf (f, "\t.byte 0, 0\n");
  fprintf (f, "assoctab_end:\n");

  for (i = 0; i < d.GroupObjects (); i++)
    d.GroupObjects[i].ObjNo = i;
  d.ObjCount = d.GroupObjects ();
  if (d.ObjCount > 85)
    die (_("to many communication objectes"));
}
