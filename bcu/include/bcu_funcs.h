/*
    BCU SDK bcu development enviroment
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

#ifndef _BCU_FUNCS_H
#define _BCU_FUNCS_H

extern const uchar OR_TAB[8];
extern const uchar AND_TAB[8];

static uchar inline
_U_flagsGet (uchar no)
{
  uchar ret;
  if (!__builtin_constant_p (no))
    asm volatile ("lda %1\n\tjsr U_flagsGet":"=z" (ret):"r" (no):"A", "X",
		  "RegC", "RegJ");
  else
  asm volatile ("lda $%1\n\tjsr U_flagsGet":"=z" (ret):"i" (no):"A", "X",
		"RegC", "RegJ");
  return ret;
}

static void inline
_U_flagsSet (uchar no, uchar flag)
{
  if (!__builtin_constant_p (no))
    asm volatile ("lda %0\n\tjsr U_flagSet"::"r" (no), "z" (flag):"A", "X",
		  "RegB", "RegC", "RegJ");
  else
  asm volatile ("lda $%0\n\tjsr U_flagSet"::"i" (no), "z" (flag):"A", "X",
		"RegB", "RegC", "RegJ");
}

static void inline
_U_transRequest (uchar no)
{
  if (!__builtin_constant_p (no))
    asm volatile ("lda %0\n\tjsr U_transRequest"::"r" (no):"A", "X", "RegB",
		  "RegC", "RegJ");
  else
  asm volatile ("lda $%0\n\tjsr U_transRequest"::"i" (no):"A", "X", "RegB",
		"RegC", "RegJ");
}

static void inline
_EEwrite (uchar offset, uchar value)
{
  if (!__builtin_constant_p (offset))
    {
      if (!__builtin_constant_p (value))
	asm volatile ("lda %0\n\tldx %1\n\tjsr EEwrite"::"r" (value),
		      "r" (offset):"A", "X", "RegB", "RegC", "RegH");
      else
      asm volatile ("lda %0\n\tldx $%1\n\tjsr EEwrite"::"r" (value),
		    "i" (offset):"A", "X", "RegB", "RegC", "RegH");
    }
  else
    {
      if (!__builtin_constant_p (value))
	asm volatile ("lda $%0\n\tldx %1\n\tjsr EEwrite"::"i" (value),
		      "r" (offset):"A", "X", "RegB", "RegC", "RegH");
      else
      asm volatile ("lda $%0\n\tldx $%1\n\tjsr EEwrite"::"i" (value),
		    "i" (offset):"A", "X", "RegB", "RegC", "RegH");
    }
}

static void inline
_EEsetChecksum ()
{
  asm volatile ("jsr EEsetChecksum":::"A", "X", "RegB", "RegC", "RegH");
}

static uchar inline
_U_debounce (uchar value, uchar time)
{
  uchar ret;
  if (!__builtin_constant_p (time))
    asm
      volatile ("lda %1\n\tldx %2\n\tjsr U_debounce\n\tsta %0":"=r" (ret):"r"
		(value), "r" (time):"X", "A", "RegB", "RegC", "RegD", "RegE",
		"RegF", "RegG");
  else
  asm
    volatile ("lda %1\n\tldx $%2\n\tjsr U_debounce\n\tsta %0":"=r" (ret):"r"
	      (value), "i" (time):"X", "A", "RegB", "RegC", "RegD", "RegE",
	      "RegF", "RegG");
  return ret;
}

static uchar inline
_U_deb10 (uchar value)
{
  uchar ret;
  asm volatile ("lda %1\n\tjsr U_deb10\n\tsta %0":"=r" (ret):"r" (value):"X",
		"A", "RegB", "RegC", "RegD", "RegE", "RegF", "RegG");
  return ret;
}

static uchar inline
_U_deb30 (uchar value)
{
  uchar ret;
  asm volatile ("lda %1\n\tjsr U_deb30\n\tsta %0":"=r" (ret):"r" (value):"X",
		"A", "RegB", "RegC", "RegD", "RegE", "RegF", "RegG");
  return ret;
}

static void inline
_U_delMsgs ()
{
  asm volatile ("jsr U_delMsgs":::"A", "X", "RegB");
}

static short inline
_U_readAD (uchar channel, uchar count)
{
  short ret;
  if (!__builtin_constant_p (count))
    {
      if (!__builtin_constant_p (channel))
	asm
	  volatile ("lda %1\n\tldx %2\n\tjsr U_readAD":"=t" (ret):"r"
		    (channel), "r" (count):"A", "X");
      else
      asm
	volatile ("lda %1\n\tldx $%2\n\tjsr U_readAD":"=t" (ret):"r" (channel),
		  "i" (count):"A", "X");
    }
  else
    {
      if (!__builtin_constant_p (channel))
	asm
	  volatile ("lda $%1\n\tldx %2\n\tjsr U_readAD":"=t" (ret):"i"
		    (channel), "r" (count):"A", "X");
      else
      asm
	volatile ("lda $%1\n\tldx $%2\n\tjsr U_readAD":"=t" (ret):"i"
		  (channel), "i" (count):"A", "X");
    }
  return ret;
}

static uchar inline
_U_ioAST (uchar val)
{
  uchar ret;
  if (!__builtin_constant_p (val))
    asm volatile ("lda %1\n\tjsr U_ioAST":"=z" (ret):"r" (val):"A", "X",
		  "RegB", "RegC", "RegD");
  else
  asm volatile ("lda $%1\n\tjsr U_ioAST":"=z" (ret):"i" (val):"A", "X", "RegB",
		"RegC", "RegD");
  return ret;
}

static void inline
_TM_Load (uchar setup, uchar runtime)
{
  if (!__builtin_constant_p (runtime))
    {
      if (!__builtin_constant_p (setup))
	asm volatile ("lda %0\n\tldx %1\n\tjsr TM_Load"::"r" (setup),
		      "r" (runtime):"A", "X", "RegB", "RegC", "RegD", "RegE",
		      "RegF");
      else
      asm volatile ("lda %0\n\tldx $%1\n\tjsr TM_Load"::"r" (setup),
		    "i" (runtime):"A", "X", "RegB", "RegC", "RegD", "RegE",
		    "RegF");
    }
  else
    {
      if (!__builtin_constant_p (setup))
	asm volatile ("lda $%0\n\tldx %1\n\tjsr TM_Load"::"i" (setup),
		      "r" (runtime):"A", "X", "RegB", "RegC", "RegD", "RegE",
		      "RegF");
      else
      asm volatile ("lda $%0\n\tldx $%1\n\tjsr TM_Load"::"i" (setup),
		    "i" (runtime):"A", "X", "RegB", "RegC", "RegD", "RegE",
		    "RegF");
    }
}

static void inline
_U_SetTM (uchar timer, uchar pointer, uchar time)
{
  if (!__builtin_constant_p (pointer))
    {
      if (!__builtin_constant_p (timer))
	asm
	  volatile ("lda %1\n\tldx %2\n\tjsr U_SetTM"::"e" (time),"r" (timer),
		    "r" (pointer):"A", "X", "RegB", "RegC", "RegD");
      else
      asm volatile ("lda %1\n\tldx $%2\n\tjsr U_SetTM"::"e" (time),"r" (timer),
		    "i" (pointer):"A", "X", "RegB", "RegC", "RegD");
    }
  else
    {
      if (!__builtin_constant_p (timer))
	asm
	  volatile ("lda $%1\n\tldx %2\n\tjsr U_SetTM"::"e" (time),"i" (timer),
		    "r" (pointer):"A", "X", "RegB", "RegC", "RegD");
      else
      asm
	volatile ("lda $%1\n\tldx $%2\n\tjsr U_SetTM"::"e" (time),"i" (timer),
		  "i" (pointer):"A", "X", "RegB", "RegC", "RegD");
    }
}

static void inline
_U_SetTMx (uchar timer, uchar time)
{
  if (!__builtin_constant_p (timer))
    asm volatile ("lda %1\n\tjsr U_SetTMx"::"e" (time),"r" (timer):"A", "X",
		  "RegB", "RegC", "RegD");
  else
  asm volatile ("lda %1\n\tjsr U_SetTMx"::"e" (time),"r" (timer):"A", "X",
		"RegB", "RegC", "RegD");
}

static void inline
_U_delay (uchar delay)
{
  if (!__builtin_constant_p (delay))
    asm volatile ("lda %0\n\tjsr U_delay"::"r" (delay):"A", "X", "RegB");
  else
  asm volatile ("lda $%0\n\tjsr U_delay"::"i" (delay):"A", "X", "RegB");
}

static void inline
_FreeBuf (uchar pointer)
{
  if (!__builtin_constant_p (pointer))
    asm volatile ("ldx %0\n\tjsr FreeBuf"::"r" (pointer):"A", "X", "RegB");
  else
  asm volatile ("ldx $%0\n\tjsr FreeBuf"::"i" (pointer):"A", "X", "RegB");
}

#ifdef BCU_0020_H

static void inline
_U_EE_WriteBlock (void *ptr, long data)
{
  asm volatile ("jsr U_EE_WriteBlock"::"v" (ptr), "y" (data):"A", "X", "RegB",
		"RegC", "RegJ");
}

static uchar inline
_U_GetAccess ()
{
  uchar ret;
  asm volatile ("jsr U_GetAccess\n\tsta %0":"=r" (ret)::"A");
  return ret;
}

static void inline
_U_SetPollingRsp (uchar val)
{
  if (!__builtin_constant_p (val))
    asm volatile ("lda %0\n\tjsr U_SetPollingRsp"::"r" (val):"A");
  else
  asm volatile ("lda $%0\n\tjsr U_SetPollingRsp"::"i" (val):"A");
}

static void inline
_U_Char_Out (uchar val)
{
  if (!__builtin_constant_p (val))
    asm volatile ("lda %0\n\tjsr U_Char_Out"::"r" (val):"A", "X");
  else
  asm volatile ("lda $%0\n\tjsr U_Char_Out"::"i" (val):"A", "X");
}

static void inline
_U_TS_Set (uchar timer, uchar mode, uchar scale, uchar value, uchar param)
{
  if (!__builtin_constant_p (timer))
    {
      if (!__builtin_constant_p (mode))
	asm volatile ("lda %1\n\tldx %2\n\tjsr U_TS_Set"::"z" (scale),
		      "r" (mode), "r" (timer), "c" (value), "d" (param):"A",
		      "X", "RegB", "RegC", "RegD", "RegE", "RegF");
      else
      asm volatile ("lda %1\n\tldx $%2\n\tjsr U_TS_Set"::"z" (scale),
		    "r" (mode), "i" (timer), "c" (value), "d" (param):"A",
		    "X", "RegB", "RegC", "RegD", "RegE", "RegF");
    }
  else
    {
      if (!__builtin_constant_p (mode))
	asm volatile ("lda $%1\n\tldx %2\n\tjsr U_TS_Set"::"z" (scale),
		      "i" (mode), "r" (timer), "c" (value), "d" (param):"A",
		      "X", "RegB", "RegC", "RegD", "RegE", "RegF");
      else
      asm volatile ("lda $%1\n\tldx $%2\n\tjsr U_TS_Set"::"z" (scale),
		    "i" (mode), "i" (timer), "c" (value), "d" (param):"A",
		    "X", "RegB", "RegC", "RegD", "RegE", "RegF");
    }
}

static void inline
_U_TS_Del (uchar val)
{
  if (!__builtin_constant_p (val))
    asm volatile ("ldx %0\n\tjsr U_TS_Del"::"r" (val):"X");
  else
  asm volatile ("ldx $%0\n\tjsr U_TS_Del"::"i" (val):"X");
}

static void inline
_U_MS_Post (uchar msgid, uchar pointer)
{
  if (!__builtin_constant_p (pointer))
    {
      if (!__builtin_constant_p (msgid))
	asm volatile ("lda %0\n\tldx %1\n\tjsr U_MS_Post"::"r" (msgid),
		      "r" (pointer):"A", "X", "RegB", "RegL", "RegM", "RegN");
      else
      asm volatile ("lda %0\n\tldx $%1\n\tjsr U_MS_Post"::"r" (msgid),
		    "i" (pointer):"A", "X", "RegB", "RegL", "RegM", "RegN");
    }
  else
    {
      if (!__builtin_constant_p (msgid))
	asm volatile ("lda $%0\n\tldx %1\n\tjsr U_MS_Post"::"i" (msgid),
		      "r" (pointer):"A", "X", "RegB", "RegL", "RegM", "RegN");
      else
      asm volatile ("lda $%0\n\tldx $%1\n\tjsr U_MS_Post"::"i" (msgid),
		    "i" (pointer):"A", "X", "RegB", "RegL", "RegM", "RegN");
    }
}

static void inline
_U_MS_Switch (uchar msgid, uchar destination)
{
  if (!__builtin_constant_p (msgid))
    {
      if (!__builtin_constant_p (destination))
	asm volatile ("lda %0\n\tldx %1\n\tjsr U_MS_Switch"::
		      "r" (destination), "r" (msgid):"A", "X", "RegB", "RegL",
		      "RegM", "RegN");
      else
      asm volatile ("lda %0\n\tldx $%1\n\tjsr U_MS_Switch"::"r" (destination),
		    "i" (msgid):"A", "X", "RegB", "RegL", "RegM", "RegN");
    }
  else
    {
      if (!__builtin_constant_p (destination))
	asm volatile ("lda $%0\n\tldx %1\n\tjsr U_MS_Switch"::
		      "i" (destination), "r" (msgid):"A", "X", "RegB", "RegL",
		      "RegM", "RegN");
      else
      asm volatile ("lda $%0\n\tldx $%1\n\tjsr U_MS_Switch"::
		    "i" (destination), "i" (msgid):"A", "X", "RegB", "RegL",
		    "RegM", "RegN");
    }
}

static void inline
_U_FT12_Reset (uchar baudrate)
{
  if (!__builtin_constant_p (baudrate))
    asm volatile ("lda %0\n\tjsr U_FT12_Reset"::"r" (baudrate):"A", "X",
		  "RegB", "RegC", "RegD", "RegE", "RegF", "RegG", "RegH",
		  "RegI", "RegJ", "RegK", "RegL", "RegM", "RegN");
  else
  asm volatile ("lda $%0\n\tjsr U_FT12_Reset"::"i" (baudrate):"A", "X",
		"RegB", "RegC", "RegD", "RegE", "RegF", "RegG", "RegH",
		"RegI", "RegJ", "RegK", "RegL", "RegM", "RegN");
}

static void inline
_U_SCI_Init (uchar baudrate)
{
  if (!__builtin_constant_p (baudrate))
    asm volatile ("lda %0\n\tjsr U_SCI_Init"::"r" (baudrate):"A", "X");
  else
  asm volatile ("lda $%0\n\tjsr U_SCI_Init"::"i" (baudrate):"A", "X");
}

static void inline
_U_SPI_Init ()
{
  asm volatile ("jsr U_SPI_Init":::"A", "X");
}

#endif

/* not implemented:
AL_SAPcallback
U_testObj
U_EE_WriteHI
U_map
S_AstShift
S_LastShift
U_SerialShift
TM_GetFlg
U_SetTM
U_GetTM
U_GetTMx
AllocBuf
PopBuf
multDE_FG
divDE_BC
FP_Flt2Int
FP_Int2Flt
shlAn
shrAn
rolAn
U_SetBit
U_GetBit
U_FT12_GetStatus

not supported:
U_TS_Seti
 */

#endif
