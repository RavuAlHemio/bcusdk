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

#include "gencode.h"
#include "map.h"
#include "addrtab.h"
#include "loadctl.h"

static const char *
inttype (int i)
{
  const char *utypes[] = { "unsigned char", "unsigned short", "unsigned long",
    "unsigned long long"
  };
  const char *stypes[] =
    { "signed char", "signed short", "signed long", "signed long long" };
  if (i > 0)
    return utypes[i - 1];
  return stypes[-i - 1];

}

void
GenListEnum (FILE * f, ListParameter & d)
{
  fprintf (f, "typedef enum { ");
  for (int i = 0; i < d.Elements (); i++)
    fprintf (f, "%s, ", d.Elements[i].Name ());
  fprintf (f, " } %s_t;\n", d.Name ());
}

void
GenGroupObjectASM (FILE * f, GroupObject & o, BCUType b)
{
  int flag = 0;
  if (o.ObjNo == -1)
    return;
  flag = GroupObjectFlag (o, b);
  fprintf (f, "\t.byte %s(%s),0x%02X,GROUPTYPE_%d\n",
	   (b == BCU_bcu12 ? "lo8" : "offset8"), o.Name (), flag, o.Type);
}

void
GenGroupObjectUpdate (FILE * f, GroupObject & o)
{
  if (o.ObjNo == -1)
    return;
  if (!o.on_update ())
    return;
  fprintf (f, "\tlda $%d\n", o.ObjNo);
  fprintf (f, "\tjsr U_testObj\n");
  fprintf (f, "\tbeq _NoChange%d\n", o.ObjNo);
  fprintf (f, "\tjsr %s\n", o.on_update ());
  fprintf (f, "_NoChange%d:\n", o.ObjNo);
}

void
GenGroupObjectHeader (FILE * f, GroupObject & o)
{
  if (o.ObjNo == -1)
    {
      fprintf (f, "static GROUP%d_T %s;\n", o.Type, o.Name ());
      if (o.on_update ())
	fprintf (f, "static void %s();\n", o.on_update ());
#ifdef PHASE1
      if (o.Sending)
	fprintf (f, "static void %s_transmit(){}\n", o.Name ());
#else
      if (o.Sending)
	fprintf (f, "static void %s_transmit(){}\n", o.Name ());
#endif
    }
  else
    {
      fprintf (f, "%sGROUP%d_T %s __attribute__ ((section (\"%s\")));\n",
	       o.eeprom ? "const " : "",
	       o.Type, o.Name (), o.eeprom ? ".eeprom" : ".ram");
      if (o.on_update ())
	fprintf (f, "void %s();\n", o.on_update ());
#ifdef PHASE1
      if (o.Sending)
	fprintf (f,
		 "static void inline %s_transmit(){transmit_groupobject(%d);}\n",
		 o.Name (), o.ObjNo);
#else
      if (o.SendAddress_lineno)
	fprintf (f,
		 "static void inline %s_transmit(){transmit_groupobject(%d);}\n",
		 o.Name (), o.ObjNo);
#endif
    }
}

void
GenEIBObject (FILE * f, Object & o)
{
  int i;
  fprintf (f, "%s:\n", o.Name ());
  fprintf (f, "\t.byte 0x%02X,%d\n", (o.RAccess << 4) | (o.WAccess),
	   o.PropCount + 1);
  fprintf (f, "\t.byte %d,%d\n", 0, 4);
  fprintf (f, "\t.hword 0x%04X\n", o.ObjectType);
  for (i = 0; i < o.Propertys (); i++)
    {
      if (o.Propertys[i].Disable)
	continue;
      fprintf (f, "\t.byte %d,%d\n", o.Propertys[i].PropertyID,
	       (o.Propertys[i].Type & 0x1f) | (!o.Propertys[i].
					       ReadOnly ? 0x80 : 0x00) | (o.
									  Propertys
									  [i].
									  MaxArrayLength
									  ==
									  1 ?
									  0x00
									  :
									  0x40)
	       | 0x20);
      fprintf (f, "\t.hword %s%s\n", o.Propertys[i].Name (),
	       o.Propertys[i].Function () != 0 ? "_stub+0x8000" : "");
    }
}

void
GenInclude (FILE * f, Device & d)
{
  int i;
  for (i = 0; i < d.include (); i++)
    fprintf (f, "#include \"%s\"\n", escapeString (d.include[i]) ());
}

void
GenCommonHeader (FILE * f, Device & d)
{
  if (d.on_init ())
    fprintf (f, "void %s();\n", d.on_init ());
  if (d.on_run ())
    fprintf (f, "void %s();\n", d.on_run ());
  if (d.on_save ())
    fprintf (f, "void %s();\n", d.on_save ());
}

void
GenTestHeader (FILE * f, Device & d)
{
  int i;
  fprintf (f, "#include <bcu_%04x.h>\n", d.BCU);
  for (i = 0; i < d.StringParameters (); i++)
    {
      StringParameter & s = d.StringParameters[i];
      fprintf (f, "extern const char %s[%d];\n", s.Name (), s.MaxLength + 1);
    }
  for (i = 0; i < d.FloatParameters (); i++)
    {
      FloatParameter & s = d.FloatParameters[i];
      fprintf (f, "extern const float %s;\n", s.Name ());
    }
  for (i = 0; i < d.IntParameters (); i++)
    {
      IntParameter & s = d.IntParameters[i];
      fprintf (f, "extern const %s %s;\n", inttype (s.SIZE), s.Name ());
    }
  for (i = 0; i < d.ListParameters (); i++)
    {
      ListParameter & s = d.ListParameters[i];
      GenListEnum (f, s);
      fprintf (f, "extern const %s_t %s;\n", s.Name (), s.Name ());
    }
  for (i = 0; i < d.GroupObjects (); i++)
    GenGroupObjectHeader (f, d.GroupObjects[i]);
  GenCommonHeader (f, d);
}

void
GenTestData (FILE * f, Device & d)
{
  int i;
  for (i = 0; i < d.StringParameters (); i++)
    {
      StringParameter & s = d.StringParameters[i];
      fprintf (f,
	       "const char %s[%d] __attribute__ ((section (\".parameter\")))=\"%s\";\n",
	       s.Name (), s.MaxLength + 1, escapeString (s.Default) ());
    }
  for (i = 0; i < d.FloatParameters (); i++)
    {
      FloatParameter & s = d.FloatParameters[i];
      fprintf (f,
	       "const float %s __attribute__ ((section (\".parameter\"))) =%f;\n",
	       s.Name (), s.Default);
    }
  for (i = 0; i < d.IntParameters (); i++)
    {
      IntParameter & s = d.IntParameters[i];
      fprintf (f,
	       "const %s %s __attribute__ ((section (\".parameter\"))) =%d;\n",
	       inttype (s.SIZE), s.Name (), s.Default);
    }
  for (i = 0; i < d.ListParameters (); i++)
    {
      ListParameter & s = d.ListParameters[i];
      GenListEnum (f, s);
      fprintf (f,
	       "const %s_t %s __attribute__ ((section (\".parameter\"))) = %s;\n",
	       s.Name (), s.Name (), s.Elements[s.def].Name ());
    }
}
void
printParameterInfo (FILE * f, Device & d)
{
  int i;
  fprintf (f, "\t.section .loadcontrol\n");
  for (i = 0; i < d.StringParameters (); i++)
    {
      StringParameter & s = d.StringParameters[i];
      if (!s.ID ())
	continue;
      fprintf (f, "\t.hword %d\n", strlen (s.ID ()) + 7);
      fprintf (f, "\t.hword %d\n", L_STRING_PAR);
      fprintf (f, "\t.hword %s\n", s.Name ());
      fprintf (f, "\t.hword %d\n", s.MaxLength + 1);
      fprintf (f, "\t.string \"%s\"\n", s.ID ());
    }
  for (i = 0; i < d.FloatParameters (); i++)
    {
      FloatParameter & s = d.FloatParameters[i];
      if (!s.ID ())
	continue;
      fprintf (f, "\t.hword %d\n", strlen (s.ID ()) + 5);
      fprintf (f, "\t.hword %d\n", L_FLOAT_PAR);
      fprintf (f, "\t.hword %s\n", s.Name ());
      fprintf (f, "\t.string \"%s\"\n", s.ID ());
    }
  for (i = 0; i < d.IntParameters (); i++)
    {
      IntParameter & s = d.IntParameters[i];
      if (!s.ID ())
	continue;
      fprintf (f, "\t.hword %d\n", strlen (s.ID ()) + 6);
      fprintf (f, "\t.hword %d\n", L_INT_PAR);
      fprintf (f, "\t.hword %s\n", s.Name ());
      fprintf (f, "\t.byte %d\n", s.SIZE);
      fprintf (f, "\t.string \"%s\"\n", s.ID ());
    }
  for (i = 0; i < d.ListParameters (); i++)
    {
      int j, l = 0;
      ListParameter & s = d.ListParameters[i];
      if (!s.ID ())
	continue;
      for (j = 0; j < s.Elements (); j++)
	l += strlen (s.Elements[j].Value ()) + 1;
      fprintf (f, "\t.hword %d\n", strlen (s.ID ()) + 7 + l);
      fprintf (f, "\t.hword %d\n", L_LIST_PAR);
      fprintf (f, "\t.hword %s\n", s.Name ());
      fprintf (f, "\t.hword %d\n", s.Elements ());
      fprintf (f, "\t.string \"%s\"\n", s.ID ());
      for (j = 0; j < s.Elements (); j++)
	fprintf (f, "\t.string \"%s\"\n", s.Elements[j].Value ());
    }
  for (i = 0; i < d.GroupObjects (); i++)
    {
      GroupObject & s = d.GroupObjects[i];
      if (!s.ID ())
	continue;
      fprintf (f, "\t.hword %d\n", strlen (s.ID ()) + 4);
      fprintf (f, "\t.hword %d\n", L_GROUP_OBJECT);
      fprintf (f, "\t.byte %d\n", i);
      fprintf (f, "\t.string \"%s\"\n", s.ID ());
    }
}


void
GenRealHeader (FILE * f, Device & d)
{
  int i;
  fprintf (f, "#include <bcu_%04x.h>\n", d.BCU);
  for (i = 0; i < d.StringParameters (); i++)
    {
      StringParameter & s = d.StringParameters[i];
      fprintf (f, "static const char %s[/*%d*/]=\"%s\";\n", s.Name (),
	       s.MaxLength + 1, escapeString (s.Value) ());
    }
  for (i = 0; i < d.FloatParameters (); i++)
    {
      FloatParameter & s = d.FloatParameters[i];
      fprintf (f, "static const float %s=%f;\n", s.Name (), s.Value);
    }
  for (i = 0; i < d.IntParameters (); i++)
    {
      IntParameter & s = d.IntParameters[i];
      fprintf (f, "static const %s %s=%d;\n", inttype (s.SIZE), s.Name (),
	       s.Value);
    }
  for (i = 0; i < d.ListParameters (); i++)
    {
      ListParameter & s = d.ListParameters[i];
      GenListEnum (f, s);
      fprintf (f, "static const %s_t %s= %s;\n", s.Name (), s.Name (),
	       s.Elements[s.def].Name ());
    }
  for (i = 0; i < d.GroupObjects (); i++)
    GenGroupObjectHeader (f, d.GroupObjects[i]);
  GenCommonHeader (f, d);
}

void
GenBCUHeader (FILE * f, Device & d)
{
  int i;
  fprintf (f, "\t.include \"bcu_%04x.inc\"\n", d.BCU);
  fprintf (f, "\t.section .loadcontrol\n");
  fprintf (f, "\t.hword %d\n", 4);
  fprintf (f, "\t.hword %d\n", L_BCU_TYPE);
  fprintf (f, "\t.hword %d\n", d.BCU);
  if (d.BCU != BCU_bcu12)
    {
      fprintf (f, "\t.hword %d\n", 42);
      fprintf (f, "\t.hword %d\n", L_BCU2_INIT);
      fprintf (f, "\t.hword addrtab\n");
      fprintf (f, "\t.hword addrtab_end-addrtab\n");
      fprintf (f, "\t.hword assoctab\n");
      fprintf (f, "\t.hword assoctab_end-assoctab\n");

      fprintf (f, "\t.hword readonly_start\n");
      fprintf (f, "\t.hword readonly_end\n");
      fprintf (f, "\t.hword param_start\n");
      fprintf (f, "\t.hword param_end\n");
      fprintf (f, "\t.hword eibobjects\n");
      fprintf (f, "\t.hword %d #ObjCount\n", d.Objects ());
      fprintf (f, "\t.hword AL_SAPcallback\n");
      fprintf (f, "\t.hword commobjs\n");
      fprintf (f, "\t.hword ram_start\n");
      fprintf (f, "\t.hword eeprom_start\n");
      fprintf (f, "\t.hword 0x0000 #SPHandler\n");

      fprintf (f, "\t.hword _UserInit\n");
      fprintf (f, "\t.hword _UserRun\n");
      fprintf (f, "\t.hword _UserSave\n");
      fprintf (f, "\t.hword eeprom_start\n");
      fprintf (f, "\t.hword eeprom_end\n");
    }
  fprintf (f, "\t.section .bcuconfig\n");
  if (d.BCU == BCU_bcu12)
    i = (d.BCU1_PROTECT ? 0x00 : 0x02) | (d.BCU1_SEC ? 0x00 : 0x01);
  else
    i = (d.BCU2_PROTECT ? 0x00 : 0x02) | (d.BCU2_WATCHDOG ? 0x01 : 0x00);

  fprintf (f, "\t.byte 0x%02X # OptionReg\n", 0xfc | i);
  fprintf (f, "\t.hword 0x%04X # manufacturer\n", d.ManufacturerCode);
  fprintf (f, "\t.hword 0x%04X # internal manufacturer\n",
	   d.InternalManufacturerCode);
  fprintf (f, "\t.hword 0x%04X # DevType\n", d.DeviceType);
  fprintf (f, "\t.byte 0x%02X # version\n", d.Version);
  if (d.BCU == BCU_bcu12)
    fprintf (f, "\t.byte _end_readonly-0x100 # CheckLim\n");
  else
    fprintf (f, "\t.byte 0x09 # CheckLim\n");
  fprintf (f, "\t.byte 0x%02X # PEI_Type\n", d.PEIType);
  fprintf (f, "\t.byte 0x%02X # SyncRate\n", d.SyncRate);
  fprintf (f, "\t.byte 0x%02X # PortCDDR\n", d.PortCDDR);
  fprintf (f, "\t.byte 0x%02X # PortADDR\n", d.PortADDR);
  fprintf (f, "\t.byte 0xFF # RunError\n");
  fprintf (f, "\t.byte 0x%02X # RouteCnt\n",
	   (d.RouteCount << 4) | (d.U_DELMSG ? 0x80 : 0x00));
  fprintf (f, "\t.byte 0x%02X # MaxRstCnt\n",
	   (d.BusyLimit << 5) | (d.INAKLimit << 0));

  if (d.BCU == BCU_bcu12)
    i = (d.AutoPLMA ? 0x00 : 0x01) | 0xA0;
  else
    i = (d.PLM_FAST ? 0x80 : 0x00) | 0x20;
  i |= (d.RateLimit_lineno ? 0x00 : 0x08);

  if (d.CPOL)
    i |= 0x04;
  if (d.CPHA)
    i |= 0x02;
  if (!d.A_Event)
    i |= 0x40;

  fprintf (f, "\t.byte 0x%02X # ConfigDes\n", i);

  if (d.BCU == BCU_bcu12)
    {
      fprintf (f, "\t.byte assoctab-0x100\n");
      fprintf (f, "\t.byte commobjs-0x100\n");
      fprintf (f, "\t.byte _UserInit-0x100\n");
      fprintf (f, "\t.byte _UserRun-0x100\n");
      fprintf (f, "\t.byte _UserSave-0x100\n");
    }
  else
    {
      fprintf (f, "\t.byte 0 #BCU1 pointer\n");
      fprintf (f, "\t.byte 0 #BCU1 pointer\n");
      fprintf (f, "\t.byte %s #RateLimit Pointer\n",
	       (d.RateLimit_lineno ? "ratelimit+1" : "0"));
      fprintf (f, "\t.byte %s\n", "0");
      fprintf (f, "\t.byte %d # BCU2 Mark\n", 0);
    }
  if (d.RateLimit_lineno)
    fprintf (f, "\t.section .ratelimit\nratelimit:\n\t.byte %d # RateLimit",
	     d.RateLimit);
}

void
GenTestAsm (FILE * f, Device & d)
{
  int i, j;
  GenBCUHeader (f, d);
  printPseudoAddrTab (f, d);
  fprintf (f, "\t.section .commobjs\n");
  fprintf (f, "commobjs:\n");
  fprintf (f, "\t.byte %d,ramflag_pointer\n", d.ObjCount);
  for (i = 0; i < d.GroupObjects (); i++)
    GenGroupObjectASM (f, d.GroupObjects[i], d.BCU);
  fprintf (f, "\t.section .ramflags\n");
  fprintf (f, "ramflag_pointer:\n");
  j = d.ObjCount;
  if (j % 2)
    j++;
  j = j >> 1;
  for (i = 0; i < j; i++)
    fprintf (f, "\t.byte 0\n");

  fprintf (f, "\t.section .eibobjects\n");
  fprintf (f, "eibobjects:\n");
  for (i = 0; i < d.Objects (); i++)
    fprintf (f, "\t.hword %s\n", d.Objects[i].Name ());
  for (i = 0; i < d.Objects (); i++)
    GenEIBObject (f, d.Objects[i]);

  fprintf (f, "\t.section .init\n");
  fprintf (f, "_UserInit:\n");
  fprintf (f, "\tjsr _initmem\n");
  if (d.on_init ())
    fprintf (f, "\tjmp %s\n", d.on_init ());
  else
    fprintf (f, "\trts\n");
  fprintf (f, "_UserSave:\n");
  fprintf (f, "\tjsr _initstack\n");
  if (d.on_save ())
    fprintf (f, "\tjmp %s\n", d.on_save ());
  else
    fprintf (f, "\trts\n");
  fprintf (f, "_UserRun:\n");
  fprintf (f, "\tjsr _initstack\n");

  for (i = 0; i < d.GroupObjects (); i++)
    GenGroupObjectUpdate (f, d.GroupObjects[i]);

  if (d.on_run ())
    fprintf (f, "\tjmp %s\n", d.on_run ());
  else
    fprintf (f, "\trts\n");

  printParameterInfo (f, d);

}

void
GenRealAsm (FILE * f, Device & d)
{
  int i, j;
  GenBCUHeader (f, d);
  printAddrTab (f, d);
  fprintf (f, "\t.section .commobjs\n");
  fprintf (f, "commobjs:\n");
  fprintf (f, "\t.byte %d,ramflag_pointer\n", d.ObjCount);
  for (i = 0; i < d.GroupObjects (); i++)
    GenGroupObjectASM (f, d.GroupObjects[i], d.BCU);
  fprintf (f, "\t.section .ramflags\n");
  fprintf (f, "ramflag_pointer:\n");
  j = d.ObjCount;
  if (j % 2)
    j++;
  j = j >> 1;
  for (i = 0; i < j; i++)
    fprintf (f, "\t.byte 0\n");

  fprintf (f, "\t.section .eibobjects\n");
  fprintf (f, "eibobjects:\n");
  for (i = 0; i < d.Objects (); i++)
    fprintf (f, "\t.hword %s\n", d.Objects[i].Name ());
  for (i = 0; i < d.Objects (); i++)
    GenEIBObject (f, d.Objects[i]);

  fprintf (f, "\t.section .init\n");
  fprintf (f, "_UserInit:\n");
  fprintf (f, "\tjsr _initmem\n");
  if (d.on_init ())
    fprintf (f, "\tjmp %s\n", d.on_init ());
  else
    fprintf (f, "\trts\n");
  fprintf (f, "_UserSave:\n");
  fprintf (f, "\tjsr _initstack\n");
  if (d.on_save ())
    fprintf (f, "\tjmp %s\n", d.on_save ());
  else
    fprintf (f, "\trts\n");
  fprintf (f, "_UserRun:\n");
  fprintf (f, "\tjsr _initstack\n");

  for (i = 0; i < d.GroupObjects (); i++)
    GenGroupObjectUpdate (f, d.GroupObjects[i]);
  if (d.on_run ())
    fprintf (f, "\tjmp %s\n", d.on_run ());
  else
    fprintf (f, "\trts\n");
}

String
GetVariant (Device & d)
{
  char buf[1000];
  sprintf (buf, "bcu_%04x%s", d.BCU, d.Model_lineno ? d.Model () : "");
  return buf;
}
