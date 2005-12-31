/*
 * Linux USB support
 *
 * Copyright 2000-2005 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL, read LICENSE for details.
 */

#include <stdlib.h>	/* getenv, etc */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <dirent.h>
#include <ctype.h>

#include "usbi.h"

static struct list_head usbi_ios = { .prev = &usbi_ios, .next = &usbi_ios };

static char usbi_path[PATH_MAX + 1] = "";

/*
 * FIXME: We should try to translate errno error values into the standard
 * libusb errors instead of just returning LIBUSB_FAILURE
 */

static int device_open(struct usbi_device *idev)
{
  int fd;

  fd = open(idev->filename, O_RDWR);
  if (fd < 0) {
    fd = open(idev->filename, O_RDONLY);
    if (fd < 0) {
      usbi_debug(1, "failed to open %s: %s", idev->filename, strerror(errno));
      return LIBUSB_FAILURE;
    }
  }
  
  return fd;
}

int usb_os_open(struct usbi_dev_handle *dev)
{
  struct usbi_device *idev = dev->idev;

  dev->fd = device_open(idev);

  list_init(&dev->ios);

  /* FIXME: We need to query the current config here and set cur_config */

  return 0;
}

int usb_os_close(struct usbi_dev_handle *dev)
{
  if (dev->fd < 0)
    return 0;

  if (close(dev->fd) == -1)
    /* Failing trying to close a file really isn't an error, so return 0 */
    usbi_debug(2, "error closing device fd %d: %s", dev->fd, strerror(errno));

  return 0;
}

int libusb_set_configuration(libusb_dev_handle_t dev, unsigned char cfg)
{
  struct usbi_dev_handle *hdev;
  int ret, _cfg = cfg;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  ret = ioctl(hdev->fd, IOCTL_USB_SETCONFIG, &_cfg);
  if (ret < 0) {
    usbi_debug(1, "could not set config %u: %s", cfg, strerror(errno));
    return LIBUSB_FAILURE;
  }

  hdev->idev->cur_config = cfg;

  return 0;
}

int libusb_get_configuration(libusb_device_id_t devid, unsigned char *cfg)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  /* FIXME: Requery the kernel to make sure this is current information */
  *cfg = idev->cur_config;

  return 0;
}

int libusb_claim_interface(libusb_dev_handle_t dev, int interface)
{
  struct usbi_dev_handle *hdev;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  ret = ioctl(hdev->fd, IOCTL_USB_CLAIMINTF, &interface);
  if (ret < 0) {
    usbi_debug(1, "could not claim interface %d: %s", interface, strerror(errno));
    return LIBUSB_FAILURE;
  }

  hdev->interface = interface;

  /* FIXME: We need to query the current alternate setting and set altsetting */

  return 0;
}

int libusb_release_interface(libusb_dev_handle_t dev, int interface)
{
  struct usbi_dev_handle *hdev;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  ret = ioctl(hdev->fd, IOCTL_USB_RELEASEINTF, &interface);
  if (ret < 0) {
    usbi_debug(1, "could not release interface %d: %s", interface, strerror(errno));
    return LIBUSB_FAILURE;
  }

  hdev->interface = -1;

  return 0;
}

int libusb_set_altinterface(libusb_dev_handle_t dev, unsigned char alt)
{
  struct usbi_dev_handle *hdev;
  struct usbk_setinterface setintf;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  if (hdev->interface < 0)
    return LIBUSB_BADARG;

  setintf.interface = hdev->interface;
  setintf.altsetting = alt;

  ret = ioctl(hdev->fd, IOCTL_USB_SETINTF, &setintf);
  if (ret < 0) {
    usbi_debug(1, "could not set alternate interface %d/%d: %s", hdev->interface, alt, strerror(errno));
    return LIBUSB_FAILURE;
  }

  hdev->altsetting = alt;

  return 0;
}

int libusb_get_altinterface(libusb_device_id_t devid, unsigned char *alt)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  /* FIXME: Query the kernel for this information */
  return LIBUSB_FAILURE;
}

static int wakeup_event_thread(void)
{
}

int usbi_os_io_submit(struct usbi_io *io)
{
  struct usbi_dev_handle *dev = io->dev;
  int ret;

  switch (io->type) {
  case USBI_IO_CONTROL:
    io->urb.type = USBK_URB_TYPE_CONTROL;
    break;
  case USBI_IO_INTERRUPT:
    io->urb.type = USBK_URB_TYPE_INTERRUPT;
    break;
  case USBI_IO_BULK:
    io->urb.type = USBK_URB_TYPE_BULK;
    break;
  case USBI_IO_ISOCHRONOUS:
    io->urb.type = USBK_URB_TYPE_ISO;
    break;
  }

  io->urb.endpoint = io->ep;
  io->urb.status = 0;
  io->urb.flags = 0;

  if (io->setup) {
    io->tempbuf = malloc(USBI_CONTROL_SETUP_LEN + io->bufferlen);
    if (!io->tempbuf)
      return LIBUSB_NO_RESOURCES;

    memcpy(io->tempbuf, io->setup, USBI_CONTROL_SETUP_LEN);
    /* FIXME: Only do this on writes? */
    memcpy(io->tempbuf + USBI_CONTROL_SETUP_LEN, io->buffer, io->bufferlen);

    io->urb.buffer = io->tempbuf;
    io->urb.buffer_length = USBI_CONTROL_SETUP_LEN + io->bufferlen;
  } else {
    io->urb.buffer = io->buffer;
    io->urb.buffer_length = io->bufferlen;
  }

  io->urb.actual_length = 0;
  io->urb.number_of_packets = 0;
  io->urb.signr = 0;
  io->urb.usercontext = (void *)io;

  io->inprogress = 1;

  if (list_empty(&dev->ios)) {
    list_add(&dev->io_list, &usbi_ios);
    memcpy(&dev->tvo, &io->tvo, sizeof(dev->tvo));
  } else if (usbi_timeval_compare(&io->tvo, &dev->tvo) < 0)
    memcpy(&dev->tvo, &io->tvo, sizeof(dev->tvo));

  list_add(&io->list, &dev->ios);

  ret = ioctl(dev->fd, IOCTL_USB_SUBMITURB, &io->urb);
  if (ret < 0) {
    usbi_debug(1, "error submitting URB: %s", strerror(errno));

    io->inprogress = 0;
    list_del(&dev->io_list);
    list_del(&io->list);

    return LIBUSB_FAILURE;
  }

  /* Always do this to avoid race conditions */
  wakeup_event_thread();

  return 0;
}

/* FIXME: Make sure there aren't any race conditions here */
int usbi_os_io_complete(struct usbi_dev_handle *dev)
{
  struct usbk_urb *urb;
  struct usbi_io *io;
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_REAPURBNDELAY, (void *)&urb);
  if (ret < 0) {
    usbi_debug(1, "error reaping URB: %s", strerror(errno));
    return LIBUSB_FAILURE;
  }

  io = urb->usercontext;
  list_del(&io->list);		/* lock obtained by caller */

  /* FIXME: Translate the status code */

  if (io->setup)
    memcpy(io->buffer, io->urb.buffer + USBI_CONTROL_SETUP_LEN, io->bufferlen);

  /* FIXME: Should this be done without a lock held so the completion handler can callback into this code? */
  usbi_io_complete(io, urb->status, urb->actual_length);

  if (list_empty(&io->dev->ios))
    list_del(&io->dev->io_list);

  return 0;
}

int usbi_os_io_timeout(struct usbi_dev_handle *dev, struct timeval *tvc)
{
  struct timeval tvo = { .tv_sec = 0 };
  struct usbi_io *io, *tio;

  list_for_each_entry_safe(io, tio, &dev->ios, list) {
    if (usbi_timeval_compare(&io->tvo, tvc) <= 0) {
      int ret;

      list_del(&io->list);

      ret = ioctl(io->dev->fd, IOCTL_USB_DISCARDURB, &io->urb);
      if (ret < 0) {
        usbi_debug(1, "error cancelling URB: %s", strerror(errno));
        /* FIXME: Better error handling */
        return LIBUSB_FAILURE;
      }

      usbi_io_complete(io, LIBUSB_IO_TIMEOUT, 0);
    } else if (!tvo.tv_sec || usbi_timeval_compare(&io->tvo, &tvo) < 0)
      /* New soonest timeout */
      memcpy(&tvo, &io->tvo, sizeof(tvo));
  }

  memcpy(&dev->tvo, &tvo, sizeof(dev->tvo));

  return 0;
}

int usbi_os_io_cancel(struct usbi_io *io)
{
  int ret;

  ret = ioctl(io->dev->fd, IOCTL_USB_DISCARDURB, &io->urb);
  if (ret < 0) {
    usbi_debug(1, "error cancelling URB: %s", strerror(errno));
    return LIBUSB_FAILURE;
  }

  /* Always do this to avoid race conditions */
  wakeup_event_thread();

  return 0;
}

void *usbi_poll_events()
{
  char filename[PATH_MAX + 1];
  int fd;

  snprintf(filename, sizeof(filename), "%s/devices", usbi_path);
  fd = open(filename, O_RDONLY);
  if (fd < 0)
    usbi_debug(0, "unable to open %s to check for topology changes", filename);

  while (1) {
    struct usbi_dev_handle *dev, *tdev;
    struct timeval tvc, tvo;
    fd_set readfds, writefds;
    int ret, maxfd;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    /* Always check the event_pipe and the devices file */
    if (fd >= 0)
      FD_SET(fd, &readfds);

    maxfd = fd;

    gettimeofday(&tvc, NULL);

    memset(&tvo, 0, sizeof(tvo));

    list_for_each_entry(dev, &usbi_ios, io_list) {
      FD_SET(dev->fd, &writefds);
      if (dev->fd > maxfd)
        maxfd = dev->fd;

      if (dev->tvo.tv_sec &&
          (!tvo.tv_sec || usbi_timeval_compare(&dev->tvo, &tvo)))
        /* New soonest timeout */
        memcpy(&tvo, &dev->tvo, sizeof(tvo));
    }

    if (!tvo.tv_sec) {
      /* Default to an hour from now */
      tvo.tv_sec = tvc.tv_sec + (60 * 60);
      tvo.tv_usec = tvc.tv_usec;
    } else if (usbi_timeval_compare(&tvo, &tvc) < 0)
      /* Don't give a negative timeout */
      memcpy(&tvo, &tvc, sizeof(tvo));

    /* Make tvo absolute time now */
    tvo.tv_sec -= tvc.tv_sec;
    if (tvo.tv_usec < tvc.tv_usec) {
      tvo.tv_sec--;
      tvo.tv_usec += (1000000 - tvc.tv_usec);
    } else
      tvo.tv_usec -= tvc.tv_usec;

    ret = select(maxfd + 1, &readfds, &writefds, NULL, &tvo);
    if (ret < 0) {
      usbi_debug(1, "select() call failed: %s", strerror(errno));
      return 0;
    }

    gettimeofday(&tvc, NULL);

    /* FIXME: We need to handle new/removed busses as well */
    if (fd >= 0 && FD_ISSET(fd, &readfds))
      usbi_rescan_devices();

    list_for_each_entry_safe(dev, tdev, &usbi_ios, io_list) {
      if (FD_ISSET(dev->fd, &writefds))
        usbi_os_io_complete(dev);

      if (usbi_timeval_compare(&dev->tvo, &tvc) <= 0)
        usbi_os_io_timeout(dev, &tvc);

      if (list_empty(&dev->ios))
        list_del(&dev->io_list);
    }
    break;
  }

  return NULL;
}

int libusb_io_wait_handle(libusb_io_handle_t *io)
{
  return io->dev->fd;
}

int usbi_os_find_busses(struct list_head *busses)
{
  DIR *dir;
  struct dirent *entry;

  /* Scan /proc/bus/usb for bus named directories */

  dir = opendir(usbi_path);
  if (!dir) {
    usbi_debug(1, "could not opendir(%s): %s", usbi_path, strerror(errno));
    return LIBUSB_FAILURE;
  }

  while ((entry = readdir(dir)) != NULL) {
    struct usbi_bus *ibus;

    /* Skip anything starting with a . */
    if (entry->d_name[0] == '.')
      continue;

    if (!strchr("0123456789", entry->d_name[strlen(entry->d_name) - 1])) {
      usbi_debug(2, "skipping non bus dir %s", entry->d_name);
      continue;
    }

    /* FIXME: Centralize allocation and initialization */
    ibus = malloc(sizeof(*ibus));
    if (!ibus)
      return LIBUSB_NO_RESOURCES;

    memset(ibus, 0, sizeof(*ibus));

    ibus->busnum = atoi(entry->d_name);
    snprintf(ibus->filename, sizeof(ibus->filename), "%s/%s",
	usbi_path, entry->d_name);

    list_add(&ibus->list, busses);

    usbi_debug(2, "found bus dir %s", ibus->filename);
  }

  closedir(dir);

  return 0;
}

static int device_is_new(struct usbi_device *idev, unsigned short devnum)
{
  char filename[PATH_MAX + 1];
  struct stat st;

  /* Compare the mtime to ensure it's new */
  snprintf(filename, sizeof(filename) - 1, "%s/%03d", idev->bus->filename, devnum);
  stat(filename, &st);

  if (st.st_mtime == idev->mtime)
    /* mtime matches, not a new device */
    return 0;

  /*
   * FIXME: mtime does not match. Maybe the USB drivers have been unloaded and
   * reloaded? We should probably track the mtime of the bus to catch this
   * case and detach all devices on the bus. We would then detect all of
   * the devices as new.
   */

  usbi_debug(1, "device %s previously existed, but mtime has changed",
	filename);

  return 0;
}

static int create_new_device(struct usbi_device **dev, struct usbi_bus *ibus,
	unsigned short devnum, unsigned int max_children)
{
  struct usbi_device *idev;
  int i, ret;
  int fd;

  idev = malloc(sizeof(*idev));
  if (!idev)
    return LIBUSB_NO_RESOURCES;

  memset(idev, 0, sizeof(*idev));

  idev->devnum = devnum;
  snprintf(idev->filename, sizeof(idev->filename) - 1, "%s/%03d",
	ibus->filename, idev->devnum);

  idev->num_ports = max_children;
  if (max_children) {
    idev->children = malloc(idev->num_ports * sizeof(idev->children[0]));
    if (!idev->children) {
      free(idev);
      return LIBUSB_NO_RESOURCES;
    }

    memset(idev->children, 0, idev->num_ports * sizeof(idev->children[0]));
  }

  fd = device_open(idev);
  if (fd < 0) {
    usbi_debug(2, "couldn't open %s: %s", idev->filename, strerror(errno));

    free(idev);
    return LIBUSB_UNKNOWN_DEVICE;
  }

  /* FIXME: Better error messages (with filename for instance) */

  /* FIXME: Fetch the size of the descriptor first, then the rest. This is needed for forward compatibility */

  idev->desc.device_raw.data = malloc(USBI_DEVICE_DESC_SIZE);
  if (!idev->desc.device_raw.data) {
    usbi_debug(1, "unable to allocate memory for cached device descriptor");
    goto done;
  }

  ret = read(fd, idev->desc.device_raw.data, USBI_DEVICE_DESC_SIZE);
  if (ret < 0) {
    usbi_debug(1, "couldn't read descriptor: %s", strerror(errno));
    goto done;
  }

  idev->desc.device_raw.len = USBI_DEVICE_DESC_SIZE;
  usbi_parse_device_descriptor(idev, idev->desc.device_raw.data, idev->desc.device_raw.len);

  usbi_debug(2, "found device %03d on %s", idev->devnum, ibus->filename);

  /* Now try to fetch the rest of the descriptors */
  if (idev->desc.device.bNumConfigurations > USBI_MAXCONFIG)
    /* Silent since we'll try again later */
    goto done;

  if (idev->desc.device.bNumConfigurations < 1)
    /* Silent since we'll try again later */
    goto done;

  idev->desc.configs_raw = malloc(idev->desc.device.bNumConfigurations * sizeof(idev->desc.configs_raw[0]));
  if (!idev->desc.configs_raw) {
    usbi_debug(1, "unable to allocate memory for cached descriptors");

    goto done;
  }

  memset(idev->desc.configs_raw, 0, idev->desc.device.bNumConfigurations * sizeof(idev->desc.configs_raw[0]));

  idev->desc.configs = malloc(idev->desc.device.bNumConfigurations * sizeof(idev->desc.configs[0]));
  if (!idev->desc.configs)
    /* Silent since we'll try again later */
    goto done;

  idev->desc.num_configs = idev->desc.device.bNumConfigurations;

  memset(idev->desc.configs, 0, idev->desc.num_configs * sizeof(idev->desc.configs[0]));

  for (i = 0; i < idev->desc.num_configs; i++) {
    char buf[8];
    struct usbi_raw_desc *cfgr = idev->desc.configs_raw + i;

    /* Get the first 8 bytes so we can figure out what the total length is */
    ret = read(fd, buf, 8);
    if (ret < 8) {
      if (ret < 0)
        usbi_debug(1, "unable to get descriptor: %s", strerror(errno));
      else
        usbi_debug(1, "config descriptor too short (expected %d, got %d)", 8, ret);

      goto done;
    }

    cfgr->len = libusb_le16_to_cpup((uint16_t *)&buf[2]);

    cfgr->data = malloc(cfgr->len);
    if (!cfgr->data) {
      usbi_debug(1, "unable to allocate memory for descriptors");
      ret = LIBUSB_NO_RESOURCES;
      goto err;
    }

    /* Copy over the first 8 bytes we read */
    memcpy(cfgr->data, buf, 8);

    ret = read(fd, cfgr->data + 8, cfgr->len - 8);
    if (ret < cfgr->len - 8) {
      if (ret < 0)
        usbi_debug(1, "unable to get descriptor: %s", strerror(errno));
      else
        usbi_debug(1, "config descriptor too short (expected %d, got %d)", cfgr->len, ret);

      cfgr->len = 0;
      free(cfgr->data);

      goto done;
    }

    ret = usbi_parse_configuration(idev->desc.configs + i, cfgr->data, cfgr->len);
    if (ret > 0)
      usbi_debug(2, "%d bytes of descriptor data still left", ret);
    else if (ret < 0)
      usbi_debug(1, "unable to parse descriptors");
  }

done:
  *dev = idev;

  ibus->dev_by_num[devnum] = idev;

  close(fd);

  return 0;

err:
  close(fd);
  free(idev);

  return ret;
}

int usbi_os_refresh_devices(struct usbi_bus *ibus)
{
  char devfilename[PATH_MAX + 1];
  struct usbi_device *idev, *tidev;
  FILE *f;

  /*
   * This used to scan the bus directory for files named like devices.
   * Unfortunately, this has a couple of problems:
   * 1) Devices are added out of order. We need the root device first atleast.
   * 2) All kernels (up through 2.6.12 atleast) require write and/or root
   *    access to get to some important topology information.
   * So, we parse /proc/bus/usb/devices instead. It will give us the topology
   * information we're looking for, in the order we need, while being
   * available to normal users.
   */

  snprintf(devfilename, sizeof(devfilename), "%s/devices", usbi_path);
  f = fopen(devfilename, "r");
  if (!f) {
    usbi_debug(1, "could not open %s: %s", devfilename, strerror(errno));
    return LIBUSB_FAILURE;
  }

  /* Reset the found flag for all devices */
  list_for_each_entry(idev, &ibus->devices, bus_list)
    idev->found = 0;

  while (!feof(f)) {
    int busnum = 0, pdevnum = 0, pport = 0, devnum = 0, max_children = 0;
    char buf[1024], *p, *k, *v;

    if (!fgets(buf, sizeof(buf), f))
      continue;

    /* Strip off newlines and trailing spaces */
    for (p = strchr(buf, 0) - 1; p >= buf && isspace(*p); p--)
      *p = 0;

    /* Skip blank or short lines */
    if (!buf[0] || strlen(buf) < 4)
      continue;

    /* We need a character and colon to start the line */
    if (buf[1] != ':')
      break;

    switch (buf[0]) {
    case 'T': /* Topology information for a device. Also starts a new device */
      /* T:  Bus=02 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#=  5 Spd=12  MxCh= 0 */

      /* We need the bus number, parent dev number, this dev number */

      /* Tokenize into key and value pairs */
      p = buf + 2;
      do {
        /* Skip over whitespace */
        while (*p && isspace(*p))
          p++;
        if (!*p)
          break;

        /* Parse out the key */
        k = p;
        while (*p && (isalnum(*p) || *p == '#'))
          p++;
        if (!*p)
          break;
        *p++ = 0;

        /* Skip over the = */
        while (*p && (isspace(*p) || *p == '='))
          p++;
        if (!*p)
          break;

        /* Parse out the value */
        v = p;
        while (*p && (isdigit(*p) || *p == '.'))
          p++;
        if (*p)
          *p++ = 0;

        if (strcasecmp(k, "Bus") == 0)
          busnum = atoi(v);
        else if (strcasecmp(k, "Prnt") == 0)
          pdevnum = atoi(v);
        else if (strcasecmp(k, "Port") == 0)
          pport = atoi(v);
        else if (strcasecmp(k, "Dev#") == 0)
          devnum = atoi(v);
        else if (strcasecmp(k, "MxCh") == 0)
          max_children = atoi(v);
      } while (*p);

      /* Is this a device on the bus we're looking for? */
      if (busnum != ibus->busnum)
        break;

      /* Validate the data we parsed out */
      if (devnum < 1 || devnum >= USB_MAX_DEVICES_PER_BUS ||
	  max_children >= USB_MAX_DEVICES_PER_BUS ||
          pdevnum >= USB_MAX_DEVICES_PER_BUS ||
	  pport >= USB_MAX_DEVICES_PER_BUS) {
        usbi_debug(1, "invalid device number, max children or parent device");
        break;
      }

      /* Validate the parent device */
      if (pdevnum && (!ibus->dev_by_num[pdevnum] ||
          pport >= ibus->dev_by_num[pdevnum]->num_ports)) {
        usbi_debug(1, "no parent device or invalid child port number");
        break;
      }

      if (!pdevnum && ibus->root && ibus->root->found) {
        usbi_debug(1, "cannot have two root devices");
        break;
      }

      /* Only add this device if it's new */

      /* If we don't have a device by this number yet, it must be new */
      idev = ibus->dev_by_num[devnum];
      if (idev && device_is_new(idev, devnum))
        idev = NULL;

      if (!idev) {
        int ret;

        ret = create_new_device(&idev, ibus, devnum, max_children);
        if (ret) {
          usbi_debug(1, "ignoring new device because of errors");
          break;
        }

        usbi_add_device(ibus, idev);

        /* Setup parent/child relationship */
        if (pdevnum) {
          ibus->dev_by_num[pdevnum]->children[pport] = idev;
          idev->parent = ibus->dev_by_num[pdevnum];
        } else
          ibus->root = idev;
      }

      idev->found = 1;
      break;

    /* Ignore the rest */
#if 0
    case 'B': /* Bandwidth related information */
    case 'D': /* Device related information */
    case 'P': /* Vendor/Product information */
    case 'S': /* String descriptor */
    case 'C': /* Config information */
    case 'I': /* Interface information */
    case 'E': /* Endpoint information */
#endif
    default:
      break;
    }
  }

  list_for_each_entry_safe(idev, tidev, &ibus->devices, bus_list) {
    if (!idev->found) {
      /* Device disappeared, remove it */
      usbi_debug(2, "device %d removed", idev->devnum);
      usbi_remove_device(idev);
    }
  }

  return 0;
}

static int check_usb_vfs(const char *dirname)
{
  DIR *dir;
  struct dirent *entry;
  int found = 0;

  dir = opendir(dirname);
  if (!dir)
    return 0;

  while ((entry = readdir(dir)) != NULL) {
    /* Skip anything starting with a . */
    if (entry->d_name[0] == '.')
      continue;

    /* We assume if we find any files that it must be the right place */
    found = 1;
    break;
  }

  closedir(dir);

  return found;
}

void usb_os_init(void)
{
  int ret;

  /* Find the path to the virtual filesystem */
  if (getenv("USB_DEVFS_PATH")) {
    if (check_usb_vfs(getenv("USB_DEVFS_PATH"))) {
      strncpy(usbi_path, getenv("USB_DEVFS_PATH"), sizeof(usbi_path) - 1);
      usbi_path[sizeof(usbi_path) - 1] = 0;
    } else
      usbi_debug(1, "couldn't find USB VFS in USB_DEVFS_PATH");
  }

  if (!usbi_path[0]) {
    if (check_usb_vfs("/proc/bus/usb")) {
      strncpy(usbi_path, "/proc/bus/usb", sizeof(usbi_path) - 1);
      usbi_path[sizeof(usbi_path) - 1] = 0;
    } else if (check_usb_vfs("/dev/usb")) {
      strncpy(usbi_path, "/dev/usb", sizeof(usbi_path) - 1);
      usbi_path[sizeof(usbi_path) - 1] = 0;
    } else
      usbi_path[0] = 0;	/* No path, no USB support */
  }

  if (usbi_path[0])
    usbi_debug(1, "found USB VFS at %s", usbi_path);
  else
    usbi_debug(1, "no USB VFS found, is it mounted?");
}

int libusb_clear_halt(libusb_dev_handle_t dev, unsigned char ep)
{
  struct usbi_dev_handle *hdev;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  ret = ioctl(hdev->fd, IOCTL_USB_CLEAR_HALT, &ep);
  if (ret) {
    usbi_debug(1, "could not clear halt ep %d: %s", ep, strerror(errno));
    return LIBUSB_FAILURE;
  }

  return 0;
}

int libusb_reset(libusb_dev_handle_t dev)
{
  struct usbi_dev_handle *hdev;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  ret = ioctl(hdev->fd, IOCTL_USB_RESET, NULL);
  if (ret) {
    usbi_debug(1, "could not reset: %s", strerror(errno));
    return LIBUSB_FAILURE;
  }

  return 0;
}

int libusb_get_driver_np(libusb_dev_handle_t dev, int interface, char *name,
	unsigned int namelen)
{
  struct usbi_dev_handle *hdev;
  struct usbk_getdriver getdrv;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  getdrv.interface = interface;
  ret = ioctl(hdev->fd, IOCTL_USB_GETDRIVER, &getdrv);
  if (ret) {
    usbi_debug(1, "could not get bound driver: %s", strerror(errno));
    return LIBUSB_FAILURE;
  }

  strncpy(name, getdrv.driver, namelen - 1);
  name[namelen - 1] = 0;

  return 0;
}

int libusb_attach_kernel_driver_np(libusb_dev_handle_t dev, int interface)
{
  struct usbi_dev_handle *hdev;
  struct usbk_ioctl command;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  command.ifno = interface;
  command.ioctl_code = IOCTL_USB_CONNECT;
  command.data = NULL;

  ret = ioctl(hdev->fd, IOCTL_USB_IOCTL, &command);
  if (ret) {
    usbi_debug(1, "could not attach kernel driver to interface %d: %s", strerror(errno));
    return LIBUSB_FAILURE;
  }

  return 0;
}

int libusb_detach_kernel_driver_np(libusb_dev_handle_t dev, int interface)
{
  struct usbi_dev_handle *hdev;
  struct usbk_ioctl command;
  int ret;

  hdev = usbi_find_dev_handle(dev);
  if (!hdev)
    return LIBUSB_UNKNOWN_DEVICE;

  command.ifno = interface;
  command.ioctl_code = IOCTL_USB_DISCONNECT;
  command.data = NULL;

  ret = ioctl(hdev->fd, IOCTL_USB_IOCTL, &command);
  if (ret) {
    usbi_debug(1, "could not detach kernel driver to interface %d: %s", strerror(errno));
    return LIBUSB_FAILURE;
  }

  return 0;
}

