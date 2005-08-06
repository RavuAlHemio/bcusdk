/*
 * Handling of asynchronous IO with devices
 *
 * Copyright 2000-2005 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL, read LICENSE for details.
 */

#include <errno.h>
#include <string.h>	/* memset() */
#include <time.h>	/* time() */

#include "usbi.h"

static struct list_head completions = { .prev = &completions, .next = &completions };

/*
 * Helper functions
 */
struct usbi_io *usbi_alloc_io(usb_dev_handle_t *dev, enum usbi_io_type type,
	unsigned char ep, void *setup, void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback)
{
  struct usbi_io *io;

  io = malloc(sizeof(*io));
  if (!io)
    return NULL;

  memset(io, 0, sizeof(*io));

  list_init (&io->list);
  io->dev = dev;
  io->ep = ep;
  io->setup = setup;
  io->buffer = buffer;
  io->bufferlen = bufferlen;
  io->start = time(NULL);
  io->timeout = timeout;
  io->callback = callback;

  return io;
}

void usbi_free_io(struct usbi_io *io)
{
  if (io->inprogress)
    usbi_os_io_cancel(io);

  list_del(&io->list);

  if (io->tempbuf)
    free(io->tempbuf);

  /* io->setup is only allocated by usb_submit_control() */
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
usb_io_handle_t *usb_io_poll_completions(void)
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

void usb_io_wait(usb_io_handle_t *io)
{
  struct timeval tv;
  fd_set writefds;
  fd_set readfds;
  int h=usb_io_wait_handle(io);

  while(!usb_is_io_completed(io))
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

int usb_io_cancel(usb_io_handle_t *io)
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
  usb_io_wait(io);

  return ret;

out:
  return ret;
}

int usb_io_free(usb_io_handle_t *io)
{
  usbi_free_io(io);
  return 0;
}

int usb_is_io_completed(usb_io_handle_t *io)
{
  if(io->timeout &&
     time (NULL) - io->start > io->timeout &&
     io->inprogress)
    usbi_os_io_cancel(io);

  usbi_os_io_complete (io->dev);
  return !io->inprogress;
}

int usb_io_comp_status(usb_io_handle_t *io)
{
  return io->transferstatus;
}

unsigned char *usb_io_data(usb_io_handle_t *io)
{
  return io->buffer;
}

int usb_io_xfer_size(usb_io_handle_t *io)
{
  return io->transferlen;
}

int usb_io_req_size(usb_io_handle_t *io)
{
  return io->bufferlen;
}

int usb_io_ep_addr(usb_io_handle_t *io)
{
  return io->ep;
}

unsigned long usb_io_seq_nbr(usb_io_handle_t *io)
{
  return -EINVAL;
}

usb_dev_handle_t *usb_io_dev(usb_io_handle_t *io)
{
  return io->dev;
}

usb_io_handle_t *usb_submit_control(usb_dev_handle_t *dev,
	unsigned char ep, uint8_t bRequestType, uint8_t bRequest,
	uint16_t wValue, uint16_t wIndex, void *buffer,
	size_t bufferlen, unsigned int timeout,
	usb_io_callback_t callback)
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
  *(uint16_t *)(setup + 2) = usb_cpu_to_le16(wValue);
  *(uint16_t *)(setup + 4) = usb_cpu_to_le16(wIndex);
  *(uint16_t *)(setup + 6) = usb_cpu_to_le16(bufferlen);

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

usb_io_handle_t *usb_submit_bulk_write(usb_dev_handle_t *dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback)
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

usb_io_handle_t *usb_submit_bulk_read(usb_dev_handle_t *dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback)
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

usb_io_handle_t *usb_submit_interrupt_write(usb_dev_handle_t *dev,
	unsigned char ep, const void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback)
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

usb_io_handle_t *usb_submit_interrupt_read(usb_dev_handle_t *dev,
	unsigned char ep, void *buffer, size_t bufferlen,
	unsigned int timeout, usb_io_callback_t callback)
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

