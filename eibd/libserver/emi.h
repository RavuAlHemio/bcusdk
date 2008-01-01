/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2008 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#ifndef EIB_EMI_H
#define EIB_EMI_H

#include "lpdu.h"

/** convert L_Data_PDU to CEMI frame */
CArray L_Data_ToCEMI (uchar code, const L_Data_PDU & p);
/** create L_Data_PDU out of a CEMI frame */
L_Data_PDU *CEMI_to_L_Data (const CArray & data);

/** convert L_Data_PDU to EMI1/2 frame */
CArray L_Data_ToEMI (uchar code, const L_Data_PDU & p);
/** create L_Data_PDU out of a EMI1/2 frame */
L_Data_PDU *EMI_to_L_Data (const CArray & data);

#endif
