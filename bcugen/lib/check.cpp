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

#include <stdlib.h>
#include "check.h"
#include "regexp.h"
#include "symboltab.h"

int
UsedbyInterface (const Device & d, const String & name)
{
  int i, j, k;
  for (i = 0; i < d.FunctionalBlocks (); i++)
    for (j = 0; j < d.FunctionalBlocks[i].Interfaces (); j++)
      for (k = 0; k < d.FunctionalBlocks[i].Interfaces[j].Reference (); k++)
	if (d.FunctionalBlocks[i].Interfaces[j].Reference[k] == name)
	  return 1;
  return 0;
}

int
ObjectName (const Device & d, const String & Name)
{
  int i, j;
  for (i = 0; i < d.StringParameters (); i++)
    if (d.StringParameters[i].Name == Name)
      return 1;
  for (i = 0; i < d.FloatParameters (); i++)
    if (d.FloatParameters[i].Name == Name)
      return 1;
  for (i = 0; i < d.IntParameters (); i++)
    if (d.IntParameters[i].Name == Name)
      return 1;
  for (i = 0; i < d.ListParameters (); i++)
    if (d.ListParameters[i].Name == Name)
      return 1;

  for (i = 0; i < d.PollingMasters (); i++)
    if (d.PollingMasters[i].Name == Name)
      return 1;
  for (i = 0; i < d.PollingSlaves (); i++)
    if (d.PollingSlaves[i].Name == Name)
      return 1;
  for (i = 0; i < d.Objects (); i++)
    for (j = 0; j < d.Objects[i].Propertys (); j++)
      if (d.Objects[i].Propertys[j].Name == Name)
	return 1;
  for (i = 0; i < d.GroupObjects (); i++)
    if (d.GroupObjects[i].Name == Name)
      return 1;

  return 0;
}

void
undefined (const char *object, const char *value, int lineno)
{
  die (_("line %d: missing attribute %s of %s"), lineno, value, object);
}

void
CheckStringParameter (Device & d, StringParameter & o)
{
  if (!o.Name ())
    undefined ("StringParameter", "Name", o.lineno);
  if (!o.MaxLength_lineno)
    undefined ("StringParameter", "MaxLength", o.lineno);
  if (o.MaxLength < 0)
    die (_("line %d: negative string len"), o.MaxLength_lineno);
  if (o.RegExp ())
    if (!checkRegExp (o.RegExp ()))
      die (_("line %d: invalid regular expression"), o.RegExp_lineno);
#ifdef PHASE1
  if (!o.Title ())
    undefined ("StringParameter", "Title", o.lineno);
  if (!o.Default ())
    undefined ("StringParameter", "Default", o.lineno);
  if (strlen (o.Default ()) >= o.MaxLength)
    die (_("line %d: default value too long"), o.Default_lineno);
  if (o.RegExp ())
    if (!validateString (o.RegExp (), o.Default ()))
      die (_("line %d: Default does not match regular expression"),
	   o.Default_lineno);
  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
      o.Value = o.Default;
      o.Value_lineno = o.Default_lineno;
    }
  else
    o.ID_lineno = o.lineno;
#else
  if (!o.Value ())
    die (_("line %d: No value defined"), o.lineno);
  if (strlen (o.Value ()) >= o.MaxLength)
    die (_("line %d: value too long"), o.lineno);
  if (o.RegExp ())
    if (!validateString (o.RegExp (), o.Value ()))
      die (_("line %d: value does not match regular expression"), o.lineno);
#endif

}

void
CheckFloatParameter (Device & d, FloatParameter & o)
{
  if (!o.Name ())
    undefined ("FloatParameter", "Name", o.lineno);
  if (!o.MinValue_lineno)
    undefined ("FloatParameter", "MinValue", o.lineno);
  if (!o.MaxValue_lineno)
    undefined ("FloatParameter", "MaxValue", o.lineno);
  if (o.MinValue > o.MaxValue)
    die (_("line %d: MinValue > MaxValue"), o.MaxValue_lineno);

#ifdef PHASE1
  if (!o.Title ())
    undefined ("FloatParameter", "Title", o.lineno);
  if (!o.Default_lineno)
    undefined ("FloatParameter", "Default", o.lineno);
  if (o.Default < o.MinValue)
    die (_("line %d: Default< MinValue"), o.Default_lineno);
  if (o.Default > o.MaxValue)
    die (_("line %d: Default> MaxValue"), o.Default_lineno);

  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
      o.Value = o.Default;
      o.Value_lineno = o.Default_lineno;
    }
  else
    o.ID_lineno = o.lineno;
#else
  if (!o.Value_lineno)
    die (_("line %d: No value defined"), o.lineno);
  if (o.Value < o.MinValue)
    die (_("line %d: Value< MinValue"), o.lineno);
  if (o.Value > o.MaxValue)
    die (_("line %d: Value> MaxValue"), o.lineno);
#endif

}

void
CheckIntParameter (Device & d, IntParameter & o)
{
  if (!o.Name ())
    undefined ("IntParameter", "Name", o.lineno);
  if (!o.MinValue_lineno)
    undefined ("IntParameter", "MinValue", o.lineno);
  if (!o.MaxValue_lineno)
    undefined ("IntParameter", "MaxValue", o.lineno);
  if (o.MinValue > o.MaxValue)
    die (_("line %d: MinValue > MaxValue"), o.MaxValue_lineno);

#ifdef PHASE1
  if (!o.Title ())
    undefined ("IntParameter", "Title", o.lineno);
  if (!o.Default_lineno)
    undefined ("IntParameter", "Default", o.lineno);
  if (o.Default < o.MinValue)
    die (_("line %d: Default< MinValue"), o.Default_lineno);
  if (o.Default > o.MaxValue)
    die (_("line %d: Default> MaxValue"), o.Default_lineno);

  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
      o.Value = o.Default;
      o.Value_lineno = o.Default_lineno;
    }
  else
    o.ID_lineno = o.lineno;
#else
  if (!o.Value_lineno)
    die (_("line %d: No value defined"), o.lineno);
  if (o.Value < o.MinValue)
    die (_("line %d: Value< MinValue"), o.lineno);
  if (o.Value > o.MaxValue)
    die (_("line %d: Value> MaxValue"), o.lineno);
#endif

  if (o.MinValue < 0)
    {
      int i = abs (o.MinValue);
      int j = abs (o.MaxValue);
      if (i < j)
	j = i;
      if (i <= 127)
	o.SIZE = -1;
      else if (i <= 32767)
	o.SIZE = -2;
      else if (i <= 2147483647)
	o.SIZE = -3;
      else
	o.SIZE = -4;
    }
  else
    {
      int i = abs (o.MinValue);
      int j = abs (o.MaxValue);
      if (i < j)
	j = i;
      if (i <= 255)
	o.SIZE = 1;
      else if (i <= 65535)
	o.SIZE = 2;
      else if (i <= 4294967295U)
	o.SIZE = 3;
      else
	o.SIZE = 4;
    }
}

void
CheckListParameter (Device & d, ListParameter & o)
{
  int i, df = -1;
  if (!o.Name ())
    undefined ("ListParameter", "Name", o.lineno);

  if (!o.Elements_lineno)
    undefined ("ListParameter", "Elements", o.lineno);
  if (!o.Elements ())
    die (_("line %d: no enumeration values"), o.Elements_lineno);

#ifdef PHASE1
  if (!o.Title ())
    undefined ("ListParameter", "Title", o.lineno);
  if (!o.Default ())
    undefined ("ListParameter", "Default", o.lineno);

  o.ListElements.resize (o.Elements ());
  for (i = 0; i < o.Elements (); i++)
    {
      o.ListElements[i].Name =
	NewSymbol (o.Elements[i].Name, o.Elements_lineno);
      o.ListElements[i].Value = o.Elements[i].Value;
      if (o.Elements[i].Name == o.Default)
	df = i;
      o.Elements[i].Value = o.ListElements[i].Name;
    }
  if (df == -1)
    die (_("line %d: unknown default value"), o.Default_lineno);

  o.ListDefault = o.Elements[df].Value;

  NewSymbol (o.Name + "_t", o.lineno);
  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
      o.Value = o.Elements[df].Value;
      o.Value_lineno = o.Default_lineno;
    }
  else
    o.ID_lineno = o.lineno;


#else
  if (!o.Value ())
    undefined ("ListParameter", "Value", o.lineno);
  for (i = 0; i < o.Elements (); i++)
    {
      if (o.Elements[i].Value == o.Value)
	df = i;
    }

  if (df == -1)
    die (_("line %d: unknown default value"), o.Default_lineno);

#endif

  o.def = df;
}

void
CheckPollingMaster (Device & d, PollingMaster & o)
{

  if (!o.Name ())
    undefined ("PollingMaster", "Name", o.lineno);

#ifdef PHASE1
  if (!o.Title ())
    undefined ("PollingMaster", "Title", o.lineno);

  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
    }
  else
    o.ID_lineno = o.lineno;
#else
  if (!o.PollingAddress_lineno && o.PollingCount_lineno)
    die (_("line %d: missing PollingAddress"), o.lineno);
  if (o.PollingAddress_lineno && !o.PollingCount_lineno)
    die (_("line %d: missing PollingCount"), o.lineno);
  if (o.PollingAddress_lineno && o.PollingCount_lineno)
    {
      if (o.PollingAddress & (~0xffff))
	die (_("line %d: wrong polling address %04X"), o.lineno,
	     o.PollingAddress);
      if (o.PollingCount < 0)
	die (_("line %d: negative polling count"), o.lineno);
      if (o.PollingCount > 15)
	die (_("line %d: polling count>15"), o.lineno);
    }
#endif

}
void
CheckPollingSlave (Device & d, PollingSlave & o)
{
  if (!o.Name ())
    undefined ("PollingSlave", "Name", o.lineno);

#ifdef PHASE1
  if (!o.Title ())
    undefined ("PollingSlave", "Title", o.lineno);

  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
    }
  else
    o.ID_lineno = o.lineno;
#else
  if (!o.PollingAddress_lineno && o.PollingSlot_lineno)
    die (_("line %d: missing PollingAddress"), o.lineno);
  if (o.PollingAddress_lineno && !o.PollingSlot_lineno)
    die (_("line %d: missing PollingSlot"), o.lineno);
  if (o.PollingAddress_lineno && o.PollingSlot_lineno)
    {
      if (o.PollingAddress & (~0xffff))
	die (_("line %d: wrong polling address %04X"), o.lineno,
	     o.PollingAddress);
      if (o.PollingSlot < 0)
	die (_("line %d: negative polling slot"), o.lineno);
      if (o.PollingSlot > 15)
	die (_("line %d: polling slot>15"), o.lineno);
    }
#endif
}
void
CheckProperty (Device & d, Property & o, Object & o1)
{
  if (!o.Name ())
    undefined ("Property", "Name", o.lineno);

  if (!o.MaxArrayLength_lineno)
    {
      o.MaxArrayLength = 1;
      o.MaxArrayLength_lineno = o.lineno;
    }
  if (o.MaxArrayLength <= 0)
    die (_("line %d: negative array size"), o.lineno);

  if (o.MaxArrayLength <= 0)
    die (_("line %d: maximum array size to big"), o.lineno);

  if (!o.PropertyID_lineno)
    undefined ("Property", "PropertyID", o.lineno);

  if (!o.Type_lineno)
    undefined ("Property", "Type", o.lineno);

  if (o.PropertyID & (~0xffff) || o.PropertyID == 0)
    die (_("line %d: wrong Property ID %d"), o.lineno, o.PropertyID);

  if (o.Function ())
    NewSymbol (o.Function, o.Function_lineno);

  if (!o.Writeable_lineno)
    {
      o.Writeable_lineno = o.lineno;
      o.Writeable = 1;
    }

  if (!o.Constant_lineno)
    {
      o.Constant_lineno = o.lineno;
      o.Constant = 0;
    }

#ifdef PHASE1
  if (!o.Title ())
    undefined ("Property", "Title", o.lineno);

  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
    }
  else
    o.ID_lineno = o.lineno;
  o.Disable = 0;
  o.ReadOnly = !o.Writeable;
#else

  if (!o.WriteAccess_lineno)
    {
      o.WriteAccess_lineno = o.lineno;
      o.WriteAccess = 3;
    }
  if (!o.ReadAccess_lineno)
    {
      o.ReadAccess_lineno = o.lineno;
      o.ReadAccess = 3;
    }
  if (o.ReadAccess < 0 || o.ReadAccess > 3)
    die (_("line %d: invalid access level"), o.ReadAccess_lineno);

  if (o.WriteAccess < 0 || o.WriteAccess > 3)
    die (_("line %d: invalid access level"), o.WriteAccess_lineno);

  if (!o.Disable_lineno)
    {
      o.Disable_lineno = o.lineno;
      o.Disable = 0;
    }

  if (!o.ReadOnly_lineno)
    {
      o.ReadOnly_lineno = o.lineno;
      o.ReadOnly = o.Writeable;
    }

#endif
  if (o.Writeable && o.Constant)
    die (_("line %d: writable object is constant??"), o.lineno);
  if (!o.ReadOnly && o.Constant)
    die (_("line %d: not read only object is constant??"), o.lineno);
}

void
CheckObject (Device & d, Object & o, int &no)
{
  int i, j;
  int ra, wa;
  int PropCount;

  if (!o.Name ())
    undefined ("Object", "Name", o.lineno);

#ifdef PHASE1
  o.ObjectIndex = no;
  o.ObjectIndex_lineno = o.lineno;
  no++;

  NewSymbol (o.Name, o.lineno);
#else

#endif

  if (!o.ObjectType_lineno)
    undefined ("Object", "ObjectType", o.lineno);

  if (o.ObjectType & (~0xffff))
    die (_("line %d: wrong object type %04X"), o.lineno, o.ObjectType);

  for (i = 0; i < o.Propertys (); i++)
    CheckProperty (d, o.Propertys[i], o);

  for (i = 0; i < o.Propertys (); i++)
    for (j = i + 1; j < o.Propertys (); j++)
      if (o.Propertys[i].PropertyID == o.Propertys[j].PropertyID)
	die (_("line %d: duplicate property ID with %s"),
	     o.Propertys[i].lineno, o.Propertys[j].Name ());
#ifdef PHASE1
  o.PropCount = o.Propertys ();
  o.RAccess = 3;
  o.WAccess = 3;
#else
  ra = 4;
  wa = 4;
  PropCount = 0;
  for (i = 0; i < o.Propertys (); i++)
    {
      Property & p = o.Propertys[i];
      if (p.Disable)
	continue;
      PropCount++;
      if (!p.ReadOnly)
	{
	  if (p.WriteAccess < wa)
	    {
	      if (wa != 4)
		warn (_
		      ("line %d: inconsistent access settings for property %s"),
		      o.lineno, p.Name ());
	      wa = p.WriteAccess;
	    }
	  else if (p.WriteAccess != wa && wa != 4)
	    warn (_("line %d: inconsistent access settings for property %s"),
		  o.lineno, p.Name ());
	}
      if (p.ReadAccess < ra)
	{
	  if (ra != 4)
	    warn (_("line %d: inconsistent access settings for property %s"),
		  o.lineno, p.Name ());
	  ra = p.ReadAccess;
	}
      else if (p.ReadAccess != ra && ra != 4)
	warn (_("line %d: inconsistent access settings for property %s"),
	      o.lineno, p.Name ());

    }
  if (ra == 4)
    ra = 3;
  if (wa == 4)
    wa = 3;
  o.RAccess = ra;
  o.WAccess = wa;
  o.PropCount = PropCount;
#endif
  if (o.PropCount > 255)
    die (_("line %d: too many properties"), o.lineno);
}

void
CheckGroupObject (Device & d, GroupObject & o)
{
  if (!o.Name ())
    undefined ("GroupObject", "Name", o.lineno);

  if (!o.Type_lineno)
    undefined ("GroupObject", "Type", o.lineno);

#ifdef PHASE1
  if (!o.Title ())
    undefined ("GroupObject", "Title", o.lineno);

  if (!o.StateBased_lineno)
    undefined ("GroupObject", "StateBased", o.lineno);

  o.ID = NewSymbol (o.Name, o.lineno);
  if (!UsedbyInterface (d, o.Name))
    {
      o.ID = 0;
    }
  else
    o.ID_lineno = o.lineno;

  if (o.on_update ())
    NewSymbol (o.on_update, o.on_update_lineno);

  if (o.on_update () && !o.Receiving_lineno)
    {
      o.Receiving = 1;
      o.Receiving_lineno = o.lineno;
    }

#else

  if (!o.Priority_lineno)
    {
      o.Priority_lineno = o.lineno;
      o.Priority = PRIO_LOW;
    }

#endif
  if (!o.Sending_lineno)
    {
      o.Sending = 0;
      o.Sending_lineno = o.lineno;
    }
  if (!o.Receiving_lineno)
    {
      o.Receiving = 0;
      o.Receiving_lineno = o.lineno;
    }
  if (!o.eeprom_lineno)
    {
      o.eeprom_lineno = 0;
      o.eeprom = 0;
    }
#ifdef PHASE1
  if (o.Sending)
    NewSymbol (o.Name + "_transmit", o.lineno);
#else
  if (!o.Sending && o.SendAddress_lineno)
    die (_("line %d: can not send on this group object"), o.lineno);

  if (!o.Receiving && o.ReceiveAddress ())
    die (_("line %d: can not receive on this group object"), o.lineno);

#endif
}

void
CheckInterface (Device & d, Interface & o)
{

#ifdef PHASE1

  if (!o.Reference_lineno)
    undefined ("Interface", "Reference", o.lineno);

  if (!o.Reference ())
    die (_("line %d: no references"), o.Reference_lineno);

  o.References.resize (o.Reference ());
  for (int i = 0; i < o.Reference (); i++)
    {
      Symbol s = FindSymbol (o.Reference[i], o.References_lineno);
      o.References[i] = s.Id;
      if (!ObjectName (d, o.Reference[i]))
	die (_("line %d: %s referes to wrong object"), o.References_lineno,
	     o.Reference[i] ());
    }

  o.ID = NewSymbolAn (o.lineno);
  o.ID_lineno = o.lineno;

  if (!o.Abbreviation ())
    undefined ("Interface", "Abbreviation", o.lineno);

  if (!o.DPTType_lineno)
    undefined ("Interface", "DPTType", o.lineno);

  if (o.DPTType < 0)
    die (_("line %d: invalid DPT Type"), o.DPTType_lineno);

#else

#endif

}

void
CheckFunctionalBlock (Device & d, FunctionalBlock & o)
{
  int i;

#ifdef PHASE1
  if (!o.Title ())
    undefined ("FunctionalBlock", "Title", o.lineno);

  o.ID = NewSymbolAn (o.lineno);
  o.ID_lineno = o.lineno;

  if (!o.ProfileID_lineno)
    undefined ("FunctionalBlock", "ProfileID", o.lineno);

  if (o.ProfileID <= 0)
    die (_("line %d: invalid ProfileID"), o.ProfileID_lineno);

#else

#endif

  for (i = 0; i < o.Interfaces (); i++)
    CheckInterface (d, o.Interfaces[i]);

}


void
CheckDevice (Device & d)
{
  int i;
  int objno = 4;

  if (!d.BCU_lineno)
    undefined ("Device", "BCU", d.lineno);
  d.MaskVersion = d.BCU;

  for (i = 0; i < d.StringParameters (); i++)
    CheckStringParameter (d, d.StringParameters[i]);
  for (i = 0; i < d.FloatParameters (); i++)
    CheckFloatParameter (d, d.FloatParameters[i]);
  for (i = 0; i < d.IntParameters (); i++)
    CheckIntParameter (d, d.IntParameters[i]);
  for (i = 0; i < d.ListParameters (); i++)
    CheckListParameter (d, d.ListParameters[i]);

  for (i = 0; i < d.PollingMasters (); i++)
    CheckPollingMaster (d, d.PollingMasters[i]);
  for (i = 0; i < d.PollingSlaves (); i++)
    CheckPollingSlave (d, d.PollingSlaves[i]);
  for (i = 0; i < d.Objects (); i++)
    CheckObject (d, d.Objects[i], objno);
  for (i = 0; i < d.GroupObjects (); i++)
    CheckGroupObject (d, d.GroupObjects[i]);

  for (i = 0; i < d.FunctionalBlocks (); i++)
    CheckFunctionalBlock (d, d.FunctionalBlocks[i]);


  if (!d.ManufacturerCode_lineno)
    {
      d.ManufacturerCode_lineno = d.lineno;
      d.ManufacturerCode = 0xffff;
    }
  if (d.ManufacturerCode < 0 || d.ManufacturerCode > 0xffff)
    die (_("line %d: invalid manufacturer code"), d.ManufacturerCode_lineno);
  if (!d.InternalManufacturerCode_lineno)
    {
      d.InternalManufacturerCode_lineno = d.lineno;
      d.InternalManufacturerCode = d.ManufacturerCode;
    }

  if (d.InternalManufacturerCode < 0 || d.InternalManufacturerCode > 0xffff)
    die (_("line %d: invalid internal manufacturer code"),
	 d.InternalManufacturerCode_lineno);

  if (!d.DeviceType_lineno)
    {
      d.DeviceType_lineno = d.lineno;
      d.DeviceType = 0xfff0;
    }
  if (d.DeviceType < 0 || d.DeviceType > 0xffff)
    die (_("line %d: invalid devicetype"), d.DeviceType_lineno);

  if (!d.Version_lineno)
    {
      d.Version_lineno = d.lineno;
      d.Version = 0;
    }
  if (d.Version < 0 || d.Version > 0xff)
    die (_("line %d: invalid version"), d.Version_lineno);


  if (!d.SyncRate_lineno)
    {
      d.SyncRate_lineno = d.lineno;
      d.SyncRate = 0xff;
    }
  if (d.SyncRate < 0 || d.SyncRate > 0xff)
    die (_("line %d: invalid sync rate"), d.SyncRate_lineno);

  if (!d.PEIType_lineno)
    undefined ("Device", "PEIType", d.lineno);
  if (d.PEIType < 0 || d.PEIType >= 20)
    die (_("line %d: invalid PEI Type"), d.PEIType_lineno);

  if (!d.PortADDR_lineno)
    {
      d.PortADDR_lineno = d.lineno;
      d.PortADDR = 0;
    }
  if (d.PortADDR < 0 || d.PortADDR > 0xff)
    die (_("line %d: invalid PortADDR"), d.PortADDR_lineno);

  if (!d.PortCDDR_lineno)
    {
      d.PortCDDR_lineno = d.lineno;
      d.PortCDDR = 0;
    }
  if (d.PortCDDR < 0 || d.PortCDDR > 0xff)
    die (_("line %d: invalid PortCDDR"), d.PortCDDR_lineno);

  if (d.BCU1_SEC_lineno && d.BCU != BCU_bcu12)
    die (_("line %d: not supported for this bcu type"), d.BCU1_SEC_lineno);
  if (d.BCU1_PROTECT_lineno && d.BCU != BCU_bcu12)
    die (_("line %d: not supported for this bcu type"),
	 d.BCU1_PROTECT_lineno);
  if (d.AutoPLMA_lineno && d.BCU != BCU_bcu12)
    die (_("line %d: not supported for this bcu type"), d.AutoPLMA_lineno);

  if (d.BCU2_PROTECT_lineno && d.BCU == BCU_bcu12)
    die (_("line %d: not supported for this bcu type"),
	 d.BCU2_PROTECT_lineno);
  if (d.BCU2_WATCHDOG_lineno && d.BCU == BCU_bcu12)
    die (_("line %d: not supported for this bcu type"),
	 d.BCU2_WATCHDOG_lineno);
  if (d.PLM_FAST_lineno && d.BCU == BCU_bcu12)
    die (_("line %d: not supported for this bcu type"), d.PLM_FAST_lineno);

  if (d.RouteCount_lineno)
    {
      if (d.RouteCount < 0 || d.RouteCount > 7)
	die (_("line %d: invalid value for RouteCount"), d.RouteCount_lineno);
    }
  else
    d.RouteCount = 6;

  if (d.BusyLimit_lineno)
    {
      if (d.BusyLimit < 0 || d.BusyLimit > 7)
	die (_("line %d: invalid value for BusyLimit"), d.BusyLimit_lineno);
    }
  else
    d.BusyLimit = 3;

  if (d.INAKLimit_lineno)
    {
      if (d.INAKLimit < 0 || d.INAKLimit > 7)
	die (_("line %d: invalid value for INAKLimit"), d.INAKLimit_lineno);
    }
  else
    d.INAKLimit = 3;

  if (d.RateLimit_lineno)
    {
      if (d.RateLimit < 1 || d.RateLimit > 127)
	die (_("line %d: invalid value for RateLimit"), d.RateLimit_lineno);
    }

  if (!d.U_DELMSG_lineno)
    d.U_DELMSG = 1;
  if (!d.CPOL_lineno)
    d.CPOL_lineno = 1;
  if (!d.CPHA_lineno)
    d.CPHA_lineno = 1;
  if (!d.AutoPLMA_lineno)
    d.AutoPLMA = 0;

  if (!d.BCU1_SEC_lineno)
    d.BCU1_SEC = 0;
  if (!d.BCU1_PROTECT_lineno)
    d.BCU1_PROTECT = 0;
  if (!d.BCU2_PROTECT_lineno)
    d.BCU2_PROTECT = 0;
  if (!d.BCU2_WATCHDOG_lineno)
    d.BCU2_WATCHDOG = 0;
  if (!d.PLM_FAST_lineno)
    d.PLM_FAST = 0;
  if (!d.A_Event_lineno)
    d.A_Event = 1;

#ifdef PHASE1
  if (!d.Title ())
    undefined ("Device", "Title", d.lineno);

#else
  if (!d.PhysicalAddress_lineno)
    undefined ("Device", "PhyiscalAddress", d.lineno);
#endif
  if (d.BCU == BCU_bcu12 && d.Objects ())
    die (_("BCU1 supports no objects"));

  if (d.BCU == BCU_bcu12 && d.PollingMasters ())
    die (_("BCU1 supports no polling"));

  if (d.BCU == BCU_bcu12 && d.PollingSlaves ())
    die (_("BCU1 supports no polling"));

  if (d.BCU == BCU_bcu12 && d.InstallKey_lineno)
    die (_("installkey not supported"));
  if (d.BCU == BCU_bcu12 && d.Keys ())
    die (_("bcu1 supports no access protection"));

  if (d.BCU != BCU_bcu12)
    {
      if (d.Objects ())
	die (_("not yet supported"));

      if (d.PollingMasters ())
	die (_("not yet supported"));

      if (d.PollingSlaves ())
	die (_("not yet supported"));

    }
}