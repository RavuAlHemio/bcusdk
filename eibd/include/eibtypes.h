/*
    EIBD client library
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
#ifndef EIBTYPES_H
#define EIBTYPES_H

#define EIB_INVALID_REQUEST 0x0000
#define EIB_CONNECTION_INUSE 0x0001
#define EIB_CLOSED 0x0002
#define EIB_OPEN_BUSMONITOR 0x0003
#define EIB_BUSMONITOR_PACKET 0x0004
#define EIB_OPEN_BUSMONITOR_TEXT 0x0005
#define EIB_OPEN_T_CONNECTION 0x0006
#define EIB_OPEN_T_INDIVIDUAL 0x0007
#define EIB_OPEN_T_GROUP 0x0008
#define EIB_OPEN_T_BROADCAST 0x0009
#define EIB_APDU_PACKET 0x000A
#define EIB_M_INDIVIDUAL_ADDRESS_READ 0x000B
#define EIB_PROCESSING_ERROR 0x000C
#define EIB_PROG_MODE 0x0010
#define EIB_MASK_VERSION 0x0011
#define EIB_ERROR_ADDR_EXISTS 0x000D
#define EIB_ERROR_MORE_DEVICE 0x000E
#define EIB_M_INDIVIDUAL_ADDRESS_WRITE 0x000F
#define EIB_ERROR_TIMEOUT 0x0012
#define EIB_MC_CONNECTION 0x0013
#define EIB_MC_READ 0x0014
#define EIB_MC_WRITE 0x0015
#define EIB_MC_PROP_READ 0x0016
#define EIB_MC_PROP_WRITE 0x0017
#define EIB_MC_PEI_TYPE 0x0018
#define EIB_MC_ADC_READ 0x0019
#define EIB_MC_AUTHORIZE 0x001A
#define EIB_MC_KEY_WRITE 0x0001B
#define EIB_MC_MASK_VERSION 0x001C
#define EIB_MC_PROG_MODE 0x001D
#define EIB_MC_PROP_DESC 0x001E
#define EIB_OPEN_VBUSMONITOR 0x001F
#define EIB_OPEN_VBUSMONITOR_TEXT 0x0020
#define EIB_MC_PROP_SCAN 0x0021
#define EIB_OPEN_T_TPDU 0x0022

#endif
