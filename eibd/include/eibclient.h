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
#ifndef EIBCLIENT_H
#define EIBCLIENT_H

#include "sys/cdefs.h"
#include "stdint.h"

__BEGIN_DECLS;

#include "eibloadresult.h"

/** type represents a connection to eibd */
typedef struct _EIBConnection EIBConnection;

/** type for storing a EIB address */
typedef uint16_t eibaddr_t;

/** opens a connection to eibd.
 *   url can either be <code>ip:host:[port]</code> or <code>local:/path/to/socket</code>
 * \param url contains the url to connect to
 * \return connection handle or NULL
 */
EIBConnection *EIBSocketURL (const char *url);
/** opens a connection to eibd over socket
 * \param path path to the socket
 * \return connection handle or NULL
 */
EIBConnection *EIBSocketLocal (const char *path);
/** opens a connection to eibd over TCP/IP
 * \param host hostname running eibd
 * \param port portnumber
 * \return connection handle or NULL
 */
EIBConnection *EIBSocketRemote (const char *host, int port);

/** closes and frees a connection
 * \param con eibd connection
 */
int EIBClose (EIBConnection * con);

/** switches the connection to binary busmonitor mode
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIBOpenBusmonitor (EIBConnection * con);
/** switches the connection to text busmonitor mode
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIBOpenBusmonitorText (EIBConnection * con);
/** switches the connection to binary vbusmonitor mode
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIBOpenVBusmonitor (EIBConnection * con);
/** switches the connection to text vbusmonitor mode
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIBOpenVBusmonitorText (EIBConnection * con);
/** receives a packet on a busmonitor connection
 * \param con eibd connection
 * \param maxlen size of the buffer
 * \param buf buffer
 * \return -1 if error, else length of the packet
 */
int EIBGetBusmonitorPacket (EIBConnection * con, int maxlen, uint8_t * buf);

/** opens a T_Connection
 * \param con eibd connection
 * \param dest destionation address
 * \return 0 if successful, -1 if error
 */
int EIBOpenT_Connection (EIBConnection * con, eibaddr_t dest);
/** opens a T_Individual
 * \param con eibd connection
 * \param dest destionation address
 * \param write_only if not null, no packets from the bus will be delivered
 * \return 0 if successful, -1 if error
 */
int EIBOpenT_Individual (EIBConnection * con, eibaddr_t dest, int write_only);
/** opens a T_Group
 * \param con eibd connection
 * \param dest group address
 * \param write_only if not null, no packets from the bus will be delivered
 * \return 0 if successful, -1 if error
 */
int EIBOpenT_Group (EIBConnection * con, eibaddr_t dest, int write_only);
/** opens a T_Broadcase
 * \param con eibd connection
 * \param write_only if not null, no packets from the bus will be delivered
 * \return 0 if successful, -1 if error
 */
int EIBOpenT_Broadcast (EIBConnection * con, int write_only);
/** opens a raw Layer 4 connection
 * \param con eibd connection
 * \param src my source address (0 means default)
 * \return 0 if successful, -1 if error
 */
int EIBOpenT_TPDU (EIBConnection * con, eibaddr_t src);
/** sends a APDU
 * \param con eibd connection
 * \param len length of the APDU
 * \param data buffer with APDU
 * \return tranmited length or -1 if error
 */
int EIBSendAPDU (EIBConnection * con, int len, uint8_t * data);
/** receive a APDU
 * \param con eibd connection
 * \param maxlen buffer size
 * \param buf buffer
 * \return received length or -1 if error
 */
int EIBGetAPDU (EIBConnection * con, int maxlen, uint8_t * buf);
/** receive a APDU with source address
 * \param con eibd connection
 * \param maxlen buffer size
 * \param buf buffer
 * \param src pointer, where the source address should be stored
 * \return received length or -1 if error
 */
int EIBGetAPDU_Src (EIBConnection * con, int maxlen, uint8_t * buf,
		    eibaddr_t * src);

/** sends a TPDU with destination address
 * \param con eibd connection
 * \param dest destination address
 * \param len length of the APDU
 * \param data buffer with APDU
 * \return tranmited length or -1 if error
 */
int EIBSendTPDU (EIBConnection * con, eibaddr_t dest, int len,
		 uint8_t * data);
/** receive a TPDU with source address
 * \param con eibd connection
 * \param maxlen buffer size
 * \param buf buffer
 * \param src pointer, where the source address should be stored
 * \return received length or -1 if error
 */
#define EIBGetTPDU EIBGetAPDU_Src

/** list devices in programming mode
 * \param con eibd connection
 * \param maxlen buffer size
 * \param buf buffer
 * \return number of used bytes in the buffer or -1 if error
 */
int EIB_M_ReadIndividualAddresses (EIBConnection * con, int maxlen,
				   uint8_t * buf);

/** turns programming mode on
 * \param con eibd connection
 * \param dest address of EIB device
 * \return 0 if successful, -1 if error
 */
int EIB_M_Progmode_On (EIBConnection * con, eibaddr_t dest);
/** turns programming mode off
 * \param con eibd connection
 * \param dest address of EIB device
 * \return 0 if successful, -1 if error
 */
int EIB_M_Progmode_Off (EIBConnection * con, eibaddr_t dest);
/** toggle programming mode 
 * \param con eibd connection
 * \param dest address of EIB device
 * \return 0 if successful, -1 if error
 */
int EIB_M_Progmode_Toggle (EIBConnection * con, eibaddr_t dest);
/** get programming mode status
 * \param con eibd connection
 * \param dest address of EIB device
 * \return 0 not in programming mode, -1 if error, else programming mode
 */
int EIB_M_Progmode_Status (EIBConnection * con, eibaddr_t dest);
/** get programming mode status
 * \param con eibd connection
 * \param dest address of EIB device
 * \return -1 if error, else mask version
 */
int EIB_M_GetMaskVersion (EIBConnection * con, eibaddr_t dest);

/** write individual address
 * \param con eibd connection
 * \param dest new address of EIB device
 * \return -1 if error, 0 if successful
 */
int EIB_M_WriteIndividualAddress (EIBConnection * con, eibaddr_t dest);

/** opens a management connection
 * \param con eibd connection
 * \param dest destionation address
 * \return 0 if successful, -1 if error
 */
int EIB_MC_Connect (EIBConnection * con, eibaddr_t dest);
/** read memory on a mangement connection
 * \param con eibd connection
 * \param addr Memory address
 * \param len size to read
 * \param buf buffer
 * \return -1 if error, else read length
 */
int EIB_MC_Read (EIBConnection * con, uint16_t addr, int len, uint8_t * buf);
/** read memory on a mangement connection
 * \param con eibd connection
 * \param addr Memory address
 * \param len size to read
 * \param buf buffer
 * \return -1 if error, else read length
 */
int EIB_MC_Write (EIBConnection * con, uint16_t addr, int len,
		  const uint8_t * buf);
/** turns programming mode on on a mangement connection
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIB_MC_Progmode_On (EIBConnection * con);
/** turns programming mode off on a mangement connection
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIB_MC_Progmode_Off (EIBConnection * con);
/** toggle programming mode on a mangement connection
 * \param con eibd connection
 * \return 0 if successful, -1 if error
 */
int EIB_MC_Progmode_Toggle (EIBConnection * con);
/** get programming mode status on a mangement connection
 * \param con eibd connection
 * \return 0 not in programming mode, -1 if error, else programming mode
 */
int EIB_MC_Progmode_Status (EIBConnection * con);
/** get programming mode status on a mangement connection
 * \param con eibd connection
 * \return -1 if error, else mask version
 */
int EIB_MC_GetMaskVersion (EIBConnection * con);
/** read a property on a mangement connection
 * \param con eibd connection
 * \param obj object index
 * \param property property ID
 * \param start start element
 * \param nr_of_elem number of elements
 * \param max_len buffer size
 * \param buf buffer
 * \return -1 if error, else read length
 */
int EIB_MC_PropertyRead (EIBConnection * con, uint8_t obj, uint8_t property,
			 uint16_t start, uint8_t nr_of_elem, int max_len,
			 uint8_t * buf);
/** wirte a property on a mangement connection
 * \param con eibd connection
 * \param obj object index
 * \param property property ID
 * \param start start element
 * \param nr_of_elem number of elements
 * \param len buffer size
 * \param buf buffer
 * \return -1 if error, else write length
 */
int EIB_MC_PropertyWrite (EIBConnection * con, uint8_t obj, uint8_t property,
			  uint16_t start, uint8_t nr_of_elem, int len,
			  const uint8_t * buf);
/** read a property description on a mangement connection
 * \param con eibd connection
 * \param obj object index
 * \param property property ID
 * \param type pointer to store type
 * \param max_nr_of_elem pointer to store element count
 * \param access pointer to access level
 * \return -1 if error, else 0
 */
int EIB_MC_PropertyDesc (EIBConnection * con, uint8_t obj, uint8_t property,
			 uint8_t * type, uint16_t * max_nr_of_elem,
			 uint8_t * access);
/** list properties on a management connection
 * \param con eibd connection
 * \param maxlen buffer size
 * \param buf buffer
 * \return number of used bytes in the buffer or -1 if error
 */
int EIB_MC_PropertyScan (EIBConnection * con, int maxlen, uint8_t * buf);
/** read PEI type on a management connection
 * \param con eibd connection
 * \return PEI type or -1 if error
 */
int EIB_MC_GetPEIType (EIBConnection * con);
/** read PEI type on a management connection
 * \param con eibd connection
 * \param channel ADC channel
 * \param count repeat count
 * \param val pointer to store result
 * \return 0, if successful or -1 if error
 */
int EIB_MC_ReadADC (EIBConnection * con, uint8_t channel, uint8_t count,
		    int16_t * val);
/** authorize on a management connection
 * \param con eibd connection
 * \param key key
 * \return -1 if error, else access level
 */
int EIB_MC_Authorize (EIBConnection * con, uint8_t key[4]);
/** sets a key on a management connection
 * \param con eibd connection
 * \param level Level to set
 * \param key key
 * \return -1 if error, else 0
 */
int EIB_MC_SetKey (EIBConnection * con, uint8_t key[4], uint8_t level);
/** loads an image over an management connection
 * \param con eibd connection
 * \param image pointer to image
 * \param len legth of the image
 * \return result
 */
BCU_LOAD_RESULT EIB_LoadImage (EIBConnection * con, const uint8_t * image,
			       int len);

__END_DECLS
#endif
