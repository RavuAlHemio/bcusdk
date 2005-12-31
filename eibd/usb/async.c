/*
 * Handling of asynchronous IO with devices
 *
 * Copyright 2000-2005 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL, read LICENSE for details.
 */

#include <errno.h>
#include <string.h>	/* memset() */
#include <sys/time.h>	/* gettimeofday() */
#include <time.h>	/* time() */

#include "usbi.h"

static struct list_head completions = { .prev = &completions, .next = &completions };

/*
 * Helper functions
 */
struct usbi_io *usbi_alloc_io(libusb_dev_handle_t dev, enum usbi_io_type type,
	unsigned char ep, void *setup, void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback)
{
  struct usbi_dev_handle *hdev;
  struct usbi_io *io;
  struct timeval tvc;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return NULL;

  io = malloc(sizeof(*io));
  if (!io)
    return NULL;

  memset(io, 0, sizeof(*io));

  list_init(&io->list);

  io->dev = hdev;
  io->type = type;
  io->ep = ep;
  io->setup = setup;
  io->buffer = buffer;
  io->bufferlen = bufferlen;
  io->start = time(NULL);
  io->timeout = timeout;
  io->callback = callback;

  /* Set the end time for the timeout */
  gettimeofday(&tvc, NULL);
  io->tvo.tv_sec = tvc.tv_sec + timeout / 1000;
  io->tvo.tv_usec = tvc.tv_usec + (timeout % 1000) * 1000;

  if (io->tvo.tv_usec > 1000000) {
    io->tvo.tv_usec -= 1000000;
    io->tvo.tv_sec++;
  }

  return io;
}

void usbi_free_io(struct usbi_io *io)
{
  if (io->inprogress)
    usbi_os_io_cancel(io);

  list_del(&io->list);

  if (io->tempbuf)
    free(io->tempbuf);

  /* io->setup is only allocated by libusb_submit_control() */
  if (io->setup)
    free(io->setup);

  free(io);
}

/* Helper routine. To be called from the various ports */
void usbi_io_complete(struct usbi_io *io, int status, int transferlen)
{
  io->transferstatus = status;
  io->transferlen = transferlen;
  io->inprogress = 0;

  if (!io->callback) {
    /* Add completion for later retrieval */
    list_add(&io->list, &completions);
  }

  if (io->callback)
    /* Callback is the only completion notification */
    io->callback(io);
}

/*
 * API implementation
 */
libusb_io_handle_t *libusb_io_poll_completions(void)
{
  struct usbi_io *io = NULL;

  usbi_poll_events();
  if (!list_empty(&completions)) {
    struct list_head *tmp = completions.next;

    io = list_entry(tmp, struct usbi_io, list);
    list_del(&io->list);
  }

  return io;
}

static void _libusb_io_wait(libusb_io_handle_t *io)
{
  struct timeval tv;
  fd_set writefds;
  fd_set readfds;
  int h=libusb_io_wait_handle(io);

  while(!libusb_is_io_completed(io))
    {
      tv.tv_usec=1;
      tv.tv_sec=io->start+io->timeout-time(NULL);
  
      FD_ZERO(&writefds);
      FD_SET(h, &writefds);
      FD_ZERO(&readfds);
      FD_SET(h, &readfds);
      select(h + 1, &readfds, &writefds, NULL, io->timeout?&tv:NULL);
    }
}

void libusb_io_wait(libusb_io_handle_t *io)
{
  _libusb_io_wait(io);
}

int libusb_io_cancel(libusb_io_handle_t *io)
{
  int ret = 0;

  if (!io->inprogress)
    /* Nothing to do */
    goto out;

  ret = usbi_os_io_cancel(io);
  if (ret)
    /* FIXME: What to do on failure? */
    goto out;

  /* Wait for completion to be returned for cancelled IO */
  _libusb_io_wait(io);


out:
  return ret;
}

int libusb_io_free(libusb_io_handle_t *io)
{
  usbi_free_io(io);
  return 0;
}

int libusb_is_io_completed(libusb_io_handle_t *io)
{
  if(io->timeout &&
     time (NULL) - io->start > io->timeout &&
     io->inprogress)
    usbi_os_io_cancel(io);

  usbi_os_io_complete (io->dev);
  return !io->inprogress;
}

int libusb_io_comp_status(libusb_io_handle_t *io)
{
  return io->transferstatus;
}

unsigned char *libusb_io_data(libusb_io_handle_t *io)
{
  return io->buffer;
}

int libusb_io_xfer_size(libusb_io_handle_t *io)
{
  return io->transferlen;
}

int libusb_io_req_size(libusb_io_handle_t *io)
{
  return io->bufferlen;
}

int libusb_io_ep_addr(libusb_io_handle_t *io)
{
  return io->ep;
}

unsigned long libusb_io_seq_nbr(libusb_io_handle_t *io)
{
  return -EINVAL;
}

libusb_dev_handle_t libusb_io_dev(libusb_io_handle_t *io)
{
  return io->dev->handle;
}

libusb_io_handle_t *libusb_submit_control(libusb_dev_handle_t dev,
	unsigned char ep, uint8_t bRequestType, uint8_t bRequest,
	uint16_t wValue, uint16_t wIndex, void *buffer,
	size_t bufferlen, unsigned int timeout,
	libusb_io_callback_t callback)
{
  struct usbi_io *io;
  unsigned char *setup;
  int ret;

  setup = malloc(USBI_CONTROL_SETUP_LEN);
  if (!setup)
    return NULL;

  /* Fill in the SETUP packet */
  setup[0] = bRequestType;
  setup[1] = bRequest;
  *(uint16_t *)(setup + 2) = libusb_cpu_to_le16(wValue);
  *(uint16_t *)(setup + 4) = libusb_cpu_to_le16(wIndex);
  *(uint16_t *)(setup + 6) = libusb_cpu_to_le16(bufferlen);

  io = usbi_alloc_io(dev, USBI_IO_CONTROL, ep, setup, buffer, bufferlen,
	timeout, callback);
  if (!io) {
    free(setup);
    return NULL;
  }

  ret = usbi_os_io_submit(io);
  if (ret < 0) {
    usbi_free_io(io);
    return NULL;
  }

  return io;
}

libusb_io_handle_t *libusb_submit_bulk_write(libusb_dev_handle_t dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback)
{
  struct usbi_io *io;
  int ret;

  io = usbi_alloc_io(dev, USBI_IO_BULK, ep, NULL, (void *)buffer, bufferlen,
	timeout, callback);
  if (!io)
    return NULL;

  ret = usbi_os_io_submit(io);
  if (ret < 0) {
    usbi_free_io(io);
    return NULL;
  }

  return io;
}

libusb_io_handle_t *libusb_submit_bulk_read(libusb_dev_handle_t dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback)
{
  struct usbi_io *io;
  int ret;

  io = usbi_alloc_io(dev, USBI_IO_BULK, ep, NULL, buffer, bufferlen,
	timeout, callback);
  if (!io)
    return NULL;

  ret = usbi_os_io_submit(io);
  if (ret < 0) {
    usbi_free_io(io);
    return NULL;
  }

  return io;
}

libusb_io_handle_t *libusb_submit_interrupt_write(libusb_dev_handle_t dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback)
{
  struct usbi_io *io;
  int ret;

  io = usbi_alloc_io(dev, USBI_IO_INTERRUPT, ep, NULL, (void *)buffer,
	bufferlen, timeout, callback);
  if (!io)
    return NULL;

  ret = usbi_os_io_submit(io);
  if (ret < 0) {
    usbi_free_io(io);
    return NULL;
  }

  return io;
}

libusb_io_handle_t *libusb_submit_interrupt_read(libusb_dev_handle_t dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, libusb_io_callback_t callback)
{
  struct usbi_io *io;
  int ret;

  io = usbi_alloc_io(dev, USBI_IO_INTERRUPT, ep, NULL, buffer, bufferlen,
	timeout, callback);
  if (!io)
    return NULL;

  ret = usbi_os_io_submit(io);
  if (ret < 0) {
    usbi_free_io(io);
    return NULL;
  }

  return io;
}

