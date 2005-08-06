#ifndef __USB_H__
#define __USB_H__

/*
 * Prototypes, structure definitions and macros.
 *
 * Copyright 2000-2005 Johannes Erdfelt <johannes@erdfelt.com>
 * Copyright 2004-2005 Sun Microsystems, Inc.  All rights reserved.
 *
 * This file (and only this file) may alternatively be licensed under the
 * BSD license as well, read LICENSE for details.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <dirent.h>

/*
 * USB spec information
 *
 * This is all stuff grabbed from various USB specs and is pretty much
 * not subject to change
 */

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_DATA			10
#define USB_CLASS_VENDOR_SPEC		0xff

/*
 * Descriptor types
 */
#define USB_DESC_TYPE_DEVICE		0x01
#define USB_DESC_TYPE_CONFIG		0x02
#define USB_DESC_TYPE_STRING		0x03
#define USB_DESC_TYPE_INTERFACE		0x04
#define USB_DESC_TYPE_ENDPOINT		0x05

#define USB_DESC_TYPE_HID		0x21
#define USB_DESC_TYPE_REPORT		0x22
#define USB_DESC_TYPE_PHYSICAL		0x23
#define USB_DESC_TYPE_HUB		0x29

/* Endpoint descriptor */
struct usb_endpoint_desc {
  uint8_t  bEndpointAddress;
  uint8_t  bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t  bInterval;
  uint8_t  bRefresh;
  uint8_t  bSynchAddress;
};

#define USB_ENDPOINT_ADDRESS_MASK	0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_TYPE_MASK		0x03    /* in bmAttributes */
#define USB_ENDPOINT_TYPE_CONTROL	0
#define USB_ENDPOINT_TYPE_ISOCHRONOUS	1
#define USB_ENDPOINT_TYPE_BULK		2
#define USB_ENDPOINT_TYPE_INTERRUPT	3

/* Interface descriptor */
struct usb_interface_desc {
  uint8_t  bInterfaceNumber;
  uint8_t  bAlternateSetting;
  uint8_t  bNumEndpoints;
  uint8_t  bInterfaceClass;
  uint8_t  bInterfaceSubClass;
  uint8_t  bInterfaceProtocol;
  uint8_t  iInterface;
};

/* Configuration descriptor */
struct usb_config_desc {
  uint16_t wTotalLength;
  uint8_t  bNumInterfaces;
  uint8_t  bConfigurationValue;
  uint8_t  iConfiguration;
  uint8_t  bmAttributes;
  uint8_t  MaxPower;
};

/* Device descriptor */
struct usb_device_desc {
  uint16_t bcdUSB;
  uint8_t  bDeviceClass;
  uint8_t  bDeviceSubClass;
  uint8_t  bDeviceProtocol;
  uint8_t  bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t  iManufacturer;
  uint8_t  iProduct;
  uint8_t  iSerialNumber;
  uint8_t  bNumConfigurations;
};

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
/* 0x02 is reserved */
#define USB_REQ_SET_FEATURE		0x03
/* 0x04 is reserved */
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03

/*
 * Various libusb API related stuff
 */

#define USB_ENDPOINT_IN			0x80
#define USB_ENDPOINT_OUT		0x00

/* Error codes */
#define USB_ERROR_BEGIN			500000

/* Data types */
typedef unsigned int usb_device_id_t;
typedef unsigned int usb_bus_id_t;

typedef struct usbi_dev_handle usb_dev_handle_t;
typedef struct usbi_match usb_match_handle_t;
typedef struct usbi_io usb_io_handle_t;

enum {
  USB_ATTACH,
  USB_DETACH,
};

typedef void (*usb_event_callback_t)(usb_device_id_t devid, int event_type);
typedef void (*usb_io_callback_t)(usb_io_handle_t *iohand);

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Legend for implementation status:
 * R - Going to be removed
 * N - Not implemented
 * P - Partially implemented
 * D - Done
 * V - Verify implementation is correct
 */

/* Function prototypes */
char *usb_strerror(void);						/* R */

/* Miscellaneous support functions */
void usb_init(usb_event_callback_t callback);				/* D */
void usb_set_debug(int level);						/* D */
uint16_t usb_le16_to_cpup(uint16_t *data);				/* D */
uint16_t usb_le16_to_cpu(uint16_t data);				/* D */
#define usb_cpu_to_le16p usb_le16_to_cpup
#define usb_cpu_to_le16 usb_le16_to_cpu

/* Functions for enumerating busses */
usb_bus_id_t    usb_get_first_bus_id(void);				/* D */
usb_bus_id_t    usb_get_next_bus_id(usb_bus_id_t busid);		/* D */
usb_bus_id_t    usb_get_prev_bus_id(usb_bus_id_t busid);		/* D */
int             usb_get_busnum(usb_bus_id_t busid);			/* D */

/* Functions for enumerating devices and the heirarchy */
usb_device_id_t usb_get_next_device_id(usb_device_id_t devid);		/* DV */
usb_device_id_t usb_get_prev_device_id(usb_device_id_t devid);		/* DV */
usb_device_id_t usb_get_root_device_id(usb_bus_id_t busid);		/* D */
int             usb_get_child_count(usb_device_id_t devid);		/* D */
usb_device_id_t usb_get_child_device_id(usb_device_id_t devid,
		int port);						/* D */
usb_device_id_t usb_get_parent_device_id(usb_device_id_t devid);	/* D */

/* Functions for obtaining information on devices */
usb_bus_id_t    usb_get_device_bus_id(usb_device_id_t devid);		/* D */
int             usb_get_devnum(usb_device_id_t devid);			/* D */
int		usb_get_configuration(usb_device_id_t devid);		/* D */
int		usb_get_altinterface(usb_device_id_t devid);		/* N */

char *usb_get_device_designator(usb_device_id_t devid);			/* N */
char *usb_get_bus_designator(usb_device_id_t devid);			/* N */
char *usb_get_hub_path(usb_device_id_t devid);				/* N */

/* Functions for obtaining descriptors for devices */
int usb_get_device_desc(usb_device_id_t devid,
	struct usb_device_desc *devdsc);  				/* D */
int usb_get_config_desc(usb_device_id_t devid, int cfgidx,
	struct usb_config_desc *cfgdsc);				/* D */
int usb_get_interface_desc(usb_device_id_t devid, int cfgidx, int ifcidx,
	struct usb_interface_desc *ifcdsc);				/* D */
int usb_get_endpoint_desc(usb_device_id_t devid, int cfgidx, int ifcidx,
	int eptidx, struct usb_endpoint_desc *eptdsc);			/* D */
int usb_get_raw_device_desc(usb_device_id_t devid,
	unsigned char *buffer, size_t buflen);				/* D */
int usb_get_raw_config_desc(usb_device_id_t devid,
	int cfgidx, unsigned char *buffer, size_t buflen);		/* D */
int usb_refresh_descriptors(usb_device_id_t devid);			/* D */

/* Functions for searching for devices */
int usb_match_devices_by_vendor(usb_match_handle_t **match,
	int vendor, int product);					/* D */
int usb_match_devices_by_class(usb_match_handle_t **match,
	int Class, int subclass, int protocol);				/* D */
int usb_match_next_device(usb_match_handle_t *match,
	usb_device_id_t *devid);					/* D */
int usb_free_match(usb_match_handle_t *handle);				/* D */

/* Functions to control the device */
int usb_open(usb_device_id_t devid, usb_dev_handle_t **dev);		/* D */
usb_device_id_t usb_device_id(usb_dev_handle_t *dev);			/* D */
int usb_claim_interface(usb_dev_handle_t *dev, int interface);		/* D */
int usb_release_interface(usb_dev_handle_t *dev, int interface);	/* D */
int usb_is_interface_claimed(usb_dev_handle_t *dev, int interface);	/* N */
int usb_set_configuration(usb_dev_handle_t *dev, int configuration);	/* D */
int usb_set_altinterface(usb_dev_handle_t *dev, int alternate);		/* D */
int usb_clear_halt(usb_dev_handle_t *dev, unsigned char ep);		/* D */
int usb_reset(usb_dev_handle_t *dev);					/* D */
int usb_close(usb_dev_handle_t *dev);					/* D */

/* FIXME: We need to preprocess this with configure.in */
int usb_get_driver_np(usb_dev_handle_t *dev, int interface, char *name,
	unsigned int namelen);						/* D */
int usb_detach_kernel_driver_np(usb_dev_handle_t *dev, int interface);	/* D */
int usb_attach_kernel_driver_np(usb_dev_handle_t *dev, int interface);	/* D */
int usb_get_string(usb_dev_handle_t *dev, int index, int langid,
		   unsigned char *buf, size_t buflen);
int usb_get_string_simple(usb_dev_handle_t *dev, int index, char *buf, size_t buflen);

/* Synchronous I/O functions */
int usb_control_msg(usb_dev_handle_t *dev, uint8_t bRequestType,
	uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	void *buffer, size_t bufferlen, unsigned int timeout);        	/* D */
int usb_bulk_write(usb_dev_handle_t *dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout); 	/* D */
int usb_bulk_read(usb_dev_handle_t *dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout);       	/* D */
int usb_interrupt_write(usb_dev_handle_t *dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout); 	/* D */
int usb_interrupt_read(usb_dev_handle_t *dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout);       	/* D */

/* Asynchronous I/O functions */
usb_io_handle_t *usb_io_poll_completions(void);				/* D */
void usb_io_wait(usb_io_handle_t *);					/* D */
int usb_io_cancel(usb_io_handle_t *);					/* D */
int usb_io_free(usb_io_handle_t *);					/* D */
int usb_is_io_completed(usb_io_handle_t *);				/* D */
int usb_io_comp_status(usb_io_handle_t *);				/* D */
int usb_io_xfer_size(usb_io_handle_t *);				/* D */
int usb_io_req_size(usb_io_handle_t *);					/* D */
int usb_io_ep_addr(usb_io_handle_t *);					/* D */
usb_dev_handle_t *usb_io_dev(usb_io_handle_t *);			/* D */
unsigned char *usb_io_data(usb_io_handle_t *);				/* D */
int usb_io_wait_handle(usb_io_handle_t *);				/* D */

usb_io_handle_t *usb_submit_control(usb_dev_handle_t *dev,
	unsigned char ep, uint8_t bRequestType, uint8_t bRequest,
	uint16_t wValue, uint16_t wIndex, void *buffer,
	size_t bufferlen, unsigned int timeout,
	usb_io_callback_t callback);					/* D */
usb_io_handle_t *usb_submit_bulk_write(usb_dev_handle_t *dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback);		/* D */
usb_io_handle_t *usb_submit_bulk_read(usb_dev_handle_t *dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback);		/* D */
usb_io_handle_t *usb_submit_interrupt_write(usb_dev_handle_t *dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback);		/* D */
usb_io_handle_t *usb_submit_interrupt_read(usb_dev_handle_t *dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback);		/* D */

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */

