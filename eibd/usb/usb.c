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
static int usb_debug = USB_DEFAULT_DEBUG_LEVEL;

static struct list_head usbi_handles = { .prev = &usbi_handles, .next = &usbi_handles };
static libusb_dev_handle_t cur_handle = 1;

void libusb_set_debug(int level)
{
  if (usb_debug || level)
    fprintf(stderr, "libusb: setting debugging level to %d (%s)\n",
	level, level ? "on" : "off");

  usb_debug = level;
}

void _usbi_debug(int level, const char *func, int line, char *fmt, ...)
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

struct callback {
  struct list_head list;

  libusb_device_id_t devid;
  enum libusb_event_type type;
};

static struct list_head callbacks;
static struct usbi_event_callback usbi_event_callbacks[2];

static void *process_callbacks(void *unused)
{
  while (1) {
    struct list_head *tmp;


    tmp = callbacks.next;
    while (tmp != &callbacks) {
      struct callback *cb;
      libusb_device_id_t devid;
      enum libusb_event_type type;
      libusb_event_callback_t func;
      void *arg;

      /* Dequeue callback */
      cb = list_entry(tmp, struct callback, list);

      devid = cb->devid;
      type = cb->type;
      func = usbi_event_callbacks[type].func;
      arg = usbi_event_callbacks[type].arg;

      list_del(&cb->list);
      free(cb);


      /* Make callback */
      if (func)
        func(devid, type, arg);

      tmp = callbacks.next;
    }
    break;
  }
}

void libusb_init(void)
{
  int ret;

  if (getenv("USB_DEBUG") && usb_debug == USB_DEFAULT_DEBUG_LEVEL)
    libusb_set_debug(atoi(getenv("USB_DEBUG")));

  list_init(&callbacks);

  /* Start up thread for callbacks */
  ret = 0;
  if (ret < 0)
    usbi_debug(1, "unable to create polling thread (ret = %d)", ret);

  usb_os_init();
  usbi_rescan_devices();
}

void libusb_set_event_callback(enum libusb_event_type type,
	libusb_event_callback_t func, void *arg)
{
  if (type < 0 || type > 1)
    return;

  usbi_event_callbacks[type].func = func;
  usbi_event_callbacks[type].arg = arg;
}

void usbi_callback(libusb_device_id_t devid, enum libusb_event_type type)
{
  struct callback *cb;

  /* FIXME: Return/log error if malloc fails? */
  cb = malloc(sizeof(*cb));
  if (!cb)
    return;

  cb->devid = devid;
  cb->type = type;

  list_add(&cb->list, &callbacks);
  process_callbacks(0);
}

struct usbi_dev_handle *usbi_find_dev_handle(libusb_dev_handle_t dev)
{ 
  struct usbi_dev_handle *hdev;

  /* FIXME: We should probably index the device id in a rbtree or something */
  list_for_each_entry(hdev, &usbi_handles, list) {
    if (hdev->handle == dev)
      return hdev;
  }
   
  return NULL;
}   

int libusb_open(libusb_device_id_t devid, libusb_dev_handle_t *handle)
{
  struct usbi_device *idev;
  struct usbi_dev_handle *hdev;
  int ret;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENODEV;

  hdev = malloc(sizeof(*hdev));
  if (!hdev)
    return -ENOMEM;

  hdev->handle = cur_handle++;	/* FIXME: Locking */
  hdev->idev = idev;
  hdev->interface = hdev->altsetting = -1;

  ret = usb_os_open(hdev);
  if (ret < 0) {
    free(hdev);
    return ret;
  }

  list_add(&hdev->list, &usbi_handles);

  *handle = hdev->handle;

  return 0;
}

int libusb_get_device_id(libusb_dev_handle_t dev, libusb_device_id_t *devid)
{
  struct usbi_dev_handle *hdev;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  *devid = hdev->idev->devid;

  return 0;
}

int libusb_get_string(libusb_dev_handle_t dev, int index, int langid,
	void *buf, size_t buflen)
{
  return libusb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
                        (USB_DESC_TYPE_STRING << 8) + index, langid, buf,
			buflen, 1000);
}

int libusb_get_string_simple(libusb_dev_handle_t dev, int index,
	void *buf, size_t buflen)
{
  unsigned char *cbuf = buf;
  char tbuf[256];
  int ret, langid, si, di;

  /*
   * Asking for the zero'th index is special - it returns a string
   * descriptor that contains all the language IDs supported by the
   * device. Typically there aren't many - often only one. The
   * language IDs are 16 bit numbers, and they start at the third byte
   * in the descriptor. See USB 2.0 specification, section 9.6.7, for
   * more information on this. */
  ret = libusb_get_string(dev, 0, 0, tbuf, sizeof(tbuf));
  if (ret < 0)
    return ret;

  if (ret < 4)
    return -EIO;

  langid = tbuf[2] | (tbuf[3] << 8);

  ret = libusb_get_string(dev, index, langid, tbuf, sizeof(tbuf));
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
      cbuf[di++] = '?';
    else
      cbuf[di++] = tbuf[si];
  }

  cbuf[di] = 0;

  return di;
}

int libusb_close(libusb_dev_handle_t dev)
{
  struct usbi_dev_handle *hdev;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  ret = usb_os_close(hdev);
  list_del(&hdev->list);
  free(hdev);

  return ret;
}

/*
 * We used to determine endian at build time, but this was causing problems
 * with cross-compiling, so I decided to try this instead. It determines
 * endian at runtime, the first time the code is run. This will be add some
 * extra cycles, but it should be insignificant. A really good compiler
 * might even be able to optimize away the code to figure out the endianess.
 */

uint16_t libusb_le16_to_cpup(uint16_t *data)
{
  uint16_t endian = 0x1234;

  /* This test should be optimized away by the compiler */
  if (*(uint8_t *)&endian == 0x12) {
    unsigned char *p = (unsigned char *)data;

    return p[0] | (p[1] << 8);
  } else
    return *data;
}

uint16_t libusb_le16_to_cpu(uint16_t data)
{
  uint16_t endian = 0x1234;

  /* This test should be optimized away by the compiler */
  if (*(uint8_t *)&endian == 0x12) {
    unsigned char *p = (unsigned char *)&data;

    return p[0] | (p[1] << 8);
  } else
    return data;
}

static int finish_io(libusb_io_handle_t *io)
{
  int status, xferlen;

  libusb_io_wait(io);
  status = libusb_io_comp_status(io);
  xferlen = libusb_io_xfer_size(io);
  libusb_io_free(io);

  return (status < 0) ? status : xferlen;
}

int libusb_control_msg(libusb_dev_handle_t dev, uint8_t bRequestType,
	uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	void *buffer, size_t bufferlen, unsigned int timeout)
{
  libusb_io_handle_t *io;

  io = libusb_submit_control(dev, 0, bRequestType, bRequest, wValue, wIndex,
	buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  return finish_io(io);
}

int libusb_bulk_write(libusb_dev_handle_t dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout)
{
  libusb_io_handle_t *io;

  io = libusb_submit_bulk_write(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  return finish_io(io);
}

int libusb_bulk_read(libusb_dev_handle_t dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout)
{
  libusb_io_handle_t *io;

  io = libusb_submit_bulk_read(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  return finish_io(io);
}

int libusb_interrupt_write(libusb_dev_handle_t dev, unsigned char ep,
	const void *buffer, size_t bufferlen, unsigned int timeout)
{
  libusb_io_handle_t *io;

  io = libusb_submit_interrupt_write(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  return finish_io(io);
}

int libusb_interrupt_read(libusb_dev_handle_t dev, unsigned char ep,
	void *buffer, size_t bufferlen, unsigned int timeout)
{
  libusb_io_handle_t *io;

  io = libusb_submit_interrupt_read(dev, ep, buffer, bufferlen, timeout, NULL);
  if (!io)
    return -EINVAL;

  return finish_io(io);
}

/* FIXME: Maybe move these kinds of things to a util.c? */
int usbi_timeval_compare(struct timeval *tva, struct timeval *tvb)
{
  if (tva->tv_sec < tvb->tv_sec)
    return -1;
  else if (tva->tv_sec > tvb->tv_sec)
    return 1;

  if (tva->tv_usec < tvb->tv_usec)
    return -1;
  else if (tva->tv_usec > tvb->tv_usec)
    return 1;

  return 0;
}

static struct errorstr {
  int code;
  char *msg;
} errorstrs[] = {
  { LIBUSB_SUCCESS,                 "Call success" },
  { LIBUSB_FAILURE,                 "Unspecified error" },
  { LIBUSB_NO_RESOURCES,            "No resources available" },
  { LIBUSB_NO_BANDWIDTH,            "No bandwidth available" },
  { LIBUSB_NOT_SUPPORTED,           "Not supported by HCD" },
  { LIBUSB_HC_HARDWARE_ERROR,       "USB host controller error" },
  { LIBUSB_INVALID_PERM,            "Privileged operation" },
  { LIBUSB_BUSY,                    "Busy condition" },
  { LIBUSB_BADARG,                  "Invalid parameter" },
  { LIBUSB_NOACCESS,                "Access to device denied" },
  { LIBUSB_PARSE_ERROR,             "Data could not be parsed" },
  { LIBUSB_UNKNOWN_DEVICE,          "Device id is stale or invalid" },
  { LIBUSB_IO_STALL,                "Endpoint stalled" },
  { LIBUSB_IO_CRC_ERROR,            "CRC error" },
  { LIBUSB_IO_DEVICE_HUNG,          "Device hung" },
  { LIBUSB_IO_REQ_TOO_BIG,          "Request too big" },
  { LIBUSB_IO_BIT_STUFFING,         "Bit stuffing error" },
  { LIBUSB_IO_UNEXPECTED_PID,       "Unexpected PID" },
  { LIBUSB_IO_DATA_OVERRUN,         "Data overrun" },
  { LIBUSB_IO_DATA_UNDERRUN,        "Data underrun" },
  { LIBUSB_IO_BUFFER_OVERRUN,       "Buffer overrun" },
  { LIBUSB_IO_BUFFER_UNDERRUN,      "Buffer underrun" },
  { LIBUSB_IO_PID_CHECK_FAILURE,    "PID check failure" },
  { LIBUSB_IO_DATA_TOGGLE_MISMATCH, "Data toggle mismatch" },
  { LIBUSB_IO_TIMEOUT,              "I/O timeout" }
};

const char *libusb_strerror(int err)
{
  int i;

  for (i = 0; i < sizeof(errorstrs) / sizeof(errorstrs[0]); i++) {
    if (errorstrs[i].code == err)
      return errorstrs[i].msg;
  }

  return "Unknown error";
}

