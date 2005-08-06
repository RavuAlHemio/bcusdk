/*
 * Main API entry point
 *
 * Copyright 2000-2005 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL, read LICENSE for details.
 */

#include <stdlib.h>	/* getenv */
#include <stdio.h>	/* stderr */
#include <stdarg.h>	/* vsnprintf */
#include <errno.h>

#include "usbi.h"

#define USB_DEFAULT_DEBUG_LEVEL		0
int usb_debug = USB_DEFAULT_DEBUG_LEVEL;

usb_event_callback_t usbi_event_callback = NULL;

void usb_set_debug(int level)
{
  if (usb_debug || level)
    fprintf(stderr, "libusb: setting debugging level to %d (%s)\n",
	level, level ? "on" : "off");

  usb_debug = level;
}

void _usbi_debug(int level, char *func, int line, char *fmt, ...)
{
  char str[512];
  va_list ap;

  if (level > usb_debug)
    return;

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  fprintf(stderr, "libusb: [%s:%d] %s\n", func, line, str);
}

void usb_init(usb_event_callback_t callback)
{
  if (getenv("USB_DEBUG") && usb_debug == USB_DEFAULT_DEBUG_LEVEL)
    usb_set_debug(atoi(getenv("USB_DEBUG")));

  usbi_event_callback = callback;

  usb_os_init();
  usbi_rescan_devices();
}

int usb_open(usb_device_id_t devid, usb_dev_handle_t **handle)
{
  struct usbi_device *idev;
  usb_dev_handle_t *udev;
  int ret;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENODEV;

  udev = malloc(sizeof(*udev));
  if (!udev)
    return -ENOMEM;

  udev->idev = idev;
  udev->interface = udev->altsetting = -1;

  ret = usb_os_open(udev);
  if (ret < 0) {
    free(udev);
    return ret;
  }

  *handle = udev;

  return 0;
}

usb_device_id_t usb_device_id(usb_dev_handle_t *dev)
{
  struct usbi_device *idev = dev->idev;

  return idev->devid;
}

int usb_get_string(usb_dev_handle_t *dev, int index, int langid,
	unsigned char *buf, size_t buflen)
{
  return usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
                        (USB_DESC_TYPE_STRING << 8) + index, langid, buf,
			buflen, 1000);
}

int usb_get_string_simple(usb_dev_handle_t *dev, int index, char *buf, size_t buflen)
{
  char tbuf[256];
  int ret, langid, si, di;

  /*
   * Asking for the zero'th index is special - it returns a string
   * descriptor that contains all the language IDs supported by the
   * device. Typically there aren't many - often only one. The
   * language IDs are 16 bit numbers, and they start at the third byte
   * in the descriptor. See USB 2.0 specification, section 9.6.7, for
   * more information on this. */
  ret = usb_get_string(dev, 0, 0, tbuf, sizeof(tbuf));
  if (ret < 0)
    return ret;

  if (ret < 4)
    return -EIO;

  langid = tbuf[2] | (tbuf[3] << 8);

  ret = usb_get_string(dev, index, langid, tbuf, sizeof(tbuf));
  if (ret < 0)
    return ret;

  if (tbuf[1] != USB_DESC_TYPE_STRING)
    return -EBADE;

  if (tbuf[0] > ret)
    return -EOVERFLOW;

  for (di = 0, si = 2; si < tbuf[0]; si += 2) {
    if (di >= (buflen - 1))
      break;

    if (tbuf[si + 1])   /* high byte */
      buf[di++] = '?';
    else
      buf[di++] = tbuf[si];
  }

  buf[di] = 0;

  return di;
}

int usb_close(usb_dev_handle_t *udev)
{
  int ret;

  ret = usb_os_close(udev);
  free(udev);

  return ret;
}

/*
 * We used to determine endian at build time, but this was causing problems
 * with cross-compiling, so I decided to try this instead. It determines
 * endian at runtime, the first time the code is run. This will be add some
 * extra cycles, but it should be insignificant. A really good compiler
 * might even be able to optimize away the code to figure out the endianess.
 */

uint16_t usb_le16_to_cpup(uint16_t *data)
{
  uint16_t endian = 0x1234;

  /* This test should be optimized away by the compiler */
  if (*(uint8_t *)&endian == 0x12) {
    unsigned char *p = (unsigned char *)data;

    return p[0] | (p[1] << 8);
  } else
    return *data;
}

uint16_t usb_le16_to_cpu(uint16_t data)
{
  uint16_t endian = 0x1234;

  /* This test should be optimized away by the compiler */
  if (*(uint8_t *)&endian == 0x12) {
    unsigned char *p = (unsigned char *)&data;

    return p[0] | (p[1] << 8);
  } else
    return data;
}

int usb_control_msg(usb_dev_handle_t *dev, uint8_t bRequestType,
	uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	void *buffer, size_t bufferlen, unsigned int timeout)
{
  usb_io_handle_t *io;
  int status, xferlen;

  io = usb_submit_control(dev, 0, bRequestType, bRequest, wValue, wIndex,
	buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  usb_io_wait(io);
  status = usb_io_comp_status(io);
  xferlen = usb_io_xfer_size(io);
  usb_io_free(io);

  return (status < 0) ? status : xferlen;
}

int usb_bulk_write(usb_dev_handle_t *dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout)
{
  usb_io_handle_t *io;
  int status, xferlen;

  io = usb_submit_bulk_write(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  usb_io_wait(io);
  status = usb_io_comp_status(io);
  xferlen = usb_io_xfer_size(io);
  usb_io_free(io);

  return (status < 0) ? status : xferlen;
}

int usb_bulk_read(usb_dev_handle_t *dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout)
{
  usb_io_handle_t *io;
  int status, xferlen;

  io = usb_submit_bulk_read(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  usb_io_wait(io);
  status = usb_io_comp_status(io);
  xferlen = usb_io_xfer_size(io);
  usb_io_free(io);

  return (status < 0) ? status : xferlen;
}

int usb_interrupt_write(usb_dev_handle_t *dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout)
{
  usb_io_handle_t *io;
  int status, xferlen;

  io = usb_submit_interrupt_write(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  usb_io_wait(io);
  status = usb_io_comp_status(io);
  xferlen = usb_io_xfer_size(io);
  usb_io_free(io);

  return (status < 0) ? status : xferlen;
}

int usb_interrupt_read(usb_dev_handle_t *dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout)
{
  usb_io_handle_t *io;
  int status, xferlen;

  io = usb_submit_interrupt_read(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  usb_io_wait(io);
  status = usb_io_comp_status(io);
  xferlen = usb_io_xfer_size(io);
  usb_io_free(io);

  return (status < 0) ? status : xferlen;
}

