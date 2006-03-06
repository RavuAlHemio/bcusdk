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

/*
 * All libusb functions return an OS independent error code
 * (ie. no OS specific errno).
 * XXX more needed
 */
#define LIBUSB_SUCCESS			0	/* Call success */
#define LIBUSB_FAILURE			-1	/* Unspecified error */
#define LIBUSB_NO_RESOURCES		-2	/* No resources available */
#define LIBUSB_NO_BANDWIDTH		-3	/* No bandwidth available */
#define LIBUSB_NOT_SUPPORTED		-4	/* Not supported by HCD */
#define LIBUSB_HC_HARDWARE_ERROR	-5	/* USB host controller error */
#define LIBUSB_INVALID_PERM		-6	/* Privileged operation */
#define LIBUSB_BUSY			-7	/* Busy condition */
#define LIBUSB_BADARG			-8	/* Invalid parameter */
#define LIBUSB_NOACCESS			-9	/* Access to device denied */
#define LIBUSB_PARSE_ERROR		-11	/* Data could not be parsed */
#define LIBUSB_UNKNOWN_DEVICE		-12	/* Device id is stale or invalid */

#define LIBUSB_IO_STALL			-53	/* Endpoint stalled */
#define LIBUSB_IO_CRC_ERROR		-54	/* CRC error */
#define LIBUSB_IO_DEVICE_HUNG		-55	/* Device hung */
#define LIBUSB_IO_REQ_TOO_BIG		-56	/* Request too big */
#define LIBUSB_IO_BIT_STUFFING		-57	/* Bit stuffing error */
#define LIBUSB_IO_UNEXPECTED_PID	-58	/* Unexpected PID */
#define LIBUSB_IO_DATA_OVERRUN		-59	/* Data overrun */
#define LIBUSB_IO_DATA_UNDERRUN		-50	/* Data underrun */
#define LIBUSB_IO_BUFFER_OVERRUN	-51	/* Buffer overrun */
#define LIBUSB_IO_BUFFER_UNDERRUN	-52	/* Buffer underrun */
#define LIBUSB_IO_PID_CHECK_FAILURE	-53	/* PID check failure */
#define LIBUSB_IO_DATA_TOGGLE_MISMATCH	-54	/* Data toggle mismatch */
#define LIBUSB_IO_TIMEOUT		-55	/* I/O timeout */

/* Data types */
typedef unsigned int libusb_device_id_t;
typedef unsigned int libusb_bus_id_t;

typedef unsigned int libusb_dev_handle_t;
typedef unsigned int libusb_match_handle_t;
typedef struct usbi_io libusb_io_handle_t;

enum libusb_event_type {
  USB_ATTACH = 0,
  USB_DETACH,
};

typedef void (*libusb_event_callback_t)(libusb_device_id_t devid,
	enum libusb_event_type event_type, void *arg);
typedef void (*libusb_io_callback_t)(libusb_io_handle_t *iohand);

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

/* Miscellaneous support functions */
void libusb_init(void);							/* D */
void libusb_set_event_callback(enum libusb_event_type type,
	libusb_event_callback_t callback, void *arg);			/* D */
void libusb_set_debug(int level);					/* D */
uint16_t libusb_le16_to_cpup(uint16_t *data);				/* D */
uint16_t libusb_le16_to_cpu(uint16_t data);				/* D */
#define libusb_cpu_to_le16p libusb_le16_to_cpup
#define libusb_cpu_to_le16 libusb_le16_to_cpu
const char *libusb_strerror(int err);					/* D */

/* Functions for enumerating busses */
int libusb_get_first_bus_id(libusb_bus_id_t *busid);			/* D */
int libusb_get_next_bus_id(libusb_bus_id_t *busid);			/* D */
int libusb_get_prev_bus_id(libusb_bus_id_t *busid);			/* D */
int libusb_get_busnum(libusb_bus_id_t busid);				/* D */

/* Functions for enumerating devices and the heirarchy */
int libusb_get_first_device_id(libusb_bus_id_t busid,
	libusb_device_id_t *devid);					/* D */
int libusb_get_next_device_id(libusb_device_id_t *devid);		/* D */
int libusb_get_prev_device_id(libusb_device_id_t *devid);		/* D */

int libusb_get_child_count(libusb_device_id_t devid,
	unsigned char *count);						/* D */
int libusb_get_child_device_id(libusb_device_id_t hub_devid,
	int port, libusb_device_id_t *child_devid);			/* D */
int libusb_get_parent_device_id(libusb_device_id_t child_devid,
	libusb_device_id_t *hub_devid);					/* D */

/* Functions for obtaining information on devices */
int libusb_get_bus_id(libusb_device_id_t devid,
	libusb_bus_id_t *busid);					/* D */
int libusb_get_devnum(libusb_device_id_t devid, unsigned char *devnum);	/* D */
int libusb_get_configuration(libusb_device_id_t devid,
	unsigned char *cfg);						/* D */
int libusb_get_altinterface(libusb_device_id_t devid,
	unsigned char *alt);						/* N */

char *libusb_get_device_designator(libusb_device_id_t devid);		/* N */
char *libusb_get_bus_designator(libusb_device_id_t devid);		/* N */
char *libusb_get_hub_path(libusb_device_id_t devid);			/* N */

/* Functions for obtaining descriptors for devices */
int libusb_get_device_desc(libusb_device_id_t devid,
	struct usb_device_desc *devdsc);  				/* D */
int libusb_get_config_desc(libusb_device_id_t devid, int cfgidx,
	struct usb_config_desc *cfgdsc);				/* D */
int libusb_get_interface_desc(libusb_device_id_t devid, int cfgidx, int ifcidx,
	struct usb_interface_desc *ifcdsc);				/* D */
int libusb_get_endpoint_desc(libusb_device_id_t devid, int cfgidx, int ifcidx,
	int eptidx, struct usb_endpoint_desc *eptdsc);			/* D */
int libusb_get_raw_device_desc(libusb_device_id_t devid,
	unsigned char *buffer, size_t buflen);				/* D */
int libusb_get_raw_config_desc(libusb_device_id_t devid,
	int cfgidx, unsigned char *buffer, size_t buflen);		/* D */
int libusb_refresh_descriptors(libusb_device_id_t devid);		/* D */
int libusb_get_string(libusb_dev_handle_t dev, int index, int langid,
	void *buf, size_t buflen);					/* D */
int libusb_get_string_simple(libusb_dev_handle_t dev, int index,
	void *buf, size_t buflen);					/* D */

/* Functions for searching for devices */
int libusb_match_devices_by_vendor(libusb_match_handle_t *match,
	int vendor, int product);					/* D */
int libusb_match_devices_by_class(libusb_match_handle_t *match,
	int bClass, int bSubClass, int bProtocol);			/* D */
int libusb_match_next_device(libusb_match_handle_t match,
	libusb_device_id_t *devid);					/* D */
int libusb_free_match(libusb_match_handle_t handle);			/* D */

/* Functions to control the device */
int libusb_open(libusb_device_id_t devid, libusb_dev_handle_t *dev);	/* D */
int libusb_get_device_id(libusb_dev_handle_t dev,
	libusb_device_id_t *devid);					/* D */
int libusb_claim_interface(libusb_dev_handle_t dev, int interface);	/* D */
int libusb_release_interface(libusb_dev_handle_t dev, int interface);	/* D */
int libusb_is_interface_claimed(libusb_dev_handle_t dev,
	int interface);							/* N */
int libusb_set_configuration(libusb_dev_handle_t dev,
	unsigned char cfg);						/* D */
int libusb_set_altinterface(libusb_dev_handle_t dev,
	unsigned char alt);						/* D */
int libusb_clear_halt(libusb_dev_handle_t dev, unsigned char ep);	/* D */
int libusb_reset(libusb_dev_handle_t dev);				/* D */
int libusb_close(libusb_dev_handle_t dev);				/* D */

/* FIXME: We need to preprocess this with configure.in */
int libusb_get_driver_np(libusb_dev_handle_t dev, int interface, char *name,
	unsigned int namelen);						/* D */
int libusb_attach_kernel_driver_np(libusb_dev_handle_t dev,
	int interface);							/* D */
int libusb_detach_kernel_driver_np(libusb_dev_handle_t dev,
	int interface);							/* D */

/* Synchronous I/O functions */
int libusb_control_msg(libusb_dev_handle_t dev, uint8_t bRequestType,
	uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	void *buffer, size_t bufferlen, unsigned int timeout);        	/* D */
int libusb_bulk_write(libusb_dev_handle_t dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout); 	/* D */
int libusb_bulk_read(libusb_dev_handle_t dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout);       	/* D */
int libusb_interrupt_write(libusb_dev_handle_t dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout); 	/* D */
int libusb_interrupt_read(libusb_dev_handle_t dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout);       	/* D */

/* Asynchronous I/O functions */
libusb_io_handle_t *libusb_io_poll_completions(void);			/* D */
void libusb_io_wait(libusb_io_handle_t *);				/* D */
int libusb_io_cancel(libusb_io_handle_t *);				/* D */
int libusb_io_free(libusb_io_handle_t *);				/* D */
int libusb_is_io_completed(libusb_io_handle_t *);			/* D */
int libusb_io_comp_status(libusb_io_handle_t *);			/* D */
int libusb_io_xfer_size(libusb_io_handle_t *);				/* D */
int libusb_io_req_size(libusb_io_handle_t *);				/* D */
int libusb_io_ep_addr(libusb_io_handle_t *);				/* D */
libusb_dev_handle_t libusb_io_dev(libusb_io_handle_t *);		/* D */
unsigned char *libusb_io_data(libusb_io_handle_t *);			/* D */
int libusb_io_wait_handle(libusb_io_handle_t *);			/* D */

libusb_io_handle_t *libusb_submit_control(libusb_dev_handle_t dev,
	unsigned char ep, uint8_t bRequestType, uint8_t bRequest,
	uint16_t wValue, uint16_t wIndex, void *buffer,
	size_t bufferlen, unsigned int timeout,
	libusb_io_callback_t callback);					/* D */
libusb_io_handle_t *libusb_submit_bulk_write(libusb_dev_handle_t dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback);		/* D */
libusb_io_handle_t *libusb_submit_bulk_read(libusb_dev_handle_t dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback);		/* D */
libusb_io_handle_t *libusb_submit_interrupt_write(libusb_dev_handle_t dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback);		/* D */
libusb_io_handle_t *libusb_submit_interrupt_read(libusb_dev_handle_t dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback);		/* D */

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */

