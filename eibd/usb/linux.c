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

static int device_open(struct usbi_device *idev)
{
  int fd;

  fd = open(idev->filename, O_RDWR);
  if (fd < 0) {
    fd = open(idev->filename, O_RDONLY);
    if (fd < 0)
      USB_ERROR_STR(-errno, "failed to open %s: %s", idev->filename, strerror(errno));
  }
  
  return fd;
}

int usb_os_open(usb_dev_handle_t *dev)
{
  struct usbi_device *idev = dev->idev;

  dev->fd = device_open(idev);

  list_init(&dev->ios);

  /* FIXME: We need to query the current config here and set cur_config */

  return 0;
}

int usb_os_close(usb_dev_handle_t *dev)
{
  if (dev->fd < 0)
    return 0;

  if (close(dev->fd) == -1)
    /* Failing trying to close a file really isn't an error, so return 0 */
    USB_ERROR_STR(0, "tried to close device fd %d: %s", dev->fd,
	strerror(errno));

  return 0;
}

int usb_set_configuration(usb_dev_handle_t *dev, int configuration)
{
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_SETCONFIG, &configuration);
  if (ret < 0)
    USB_ERROR_STR(-errno, "could not set config %d: %s", configuration,
	strerror(errno));

  dev->idev->cur_config = configuration;

  return 0;
}

int usb_get_configuration(usb_device_id_t devid)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENODEV;

  /* FIXME: Requery the kernel to make sure this is current information */
  return idev->cur_config;
}

int usb_claim_interface(usb_dev_handle_t *dev, int interface)
{
  struct usbi_device *idev = dev->idev;
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_CLAIMINTF, &interface);
  if (ret < 0) {
    if (errno == EBUSY && usb_debug > 0)
      fprintf(stderr, "Check that you have permissions to write to %s and, if you don't, that you set up hotplug (http://linux-hotplug.sourceforge.net/) correctly.\n", idev->filename);

    USB_ERROR_STR(-errno, "could not claim interface %d: %s", interface,
	strerror(errno));
  }

  dev->interface = interface;

  /* FIXME: We need to query the current alternate setting and set altsetting */

  return 0;
}

int usb_release_interface(usb_dev_handle_t *dev, int interface)
{
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_RELEASEINTF, &interface);
  if (ret < 0)
    USB_ERROR_STR(-errno, "could not release intf %d: %s", interface,
    	strerror(errno));

  dev->interface = -1;

  return 0;
}

int usb_set_altinterface(usb_dev_handle_t *dev, int alternate)
{
  int ret;
  struct usbk_setinterface setintf;

  if (dev->interface < 0)
    USB_ERROR(-EINVAL);

  setintf.interface = dev->interface;
  setintf.altsetting = alternate;

  ret = ioctl(dev->fd, IOCTL_USB_SETINTF, &setintf);
  if (ret < 0)
    USB_ERROR_STR(ret, "could not set alt intf %d/%d: %s",
	dev->interface, alternate, strerror(errno));

  dev->altsetting = alternate;

  return 0;
}

int usb_get_altinterface(usb_device_id_t devid)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENODEV;

  /* FIXME: Query the kernel for this information */
  return -EINVAL;
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
      return -ENOMEM;

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

  ret = ioctl(dev->fd, IOCTL_USB_SUBMITURB, &io->urb);
  if (ret < 0) {
    usbi_debug(1, "error submitting URB: %s", strerror(errno));
    return -EINVAL;
  }

  io->inprogress = 1;

  if (list_empty(&dev->ios))
    list_add(&dev->io_list, &usbi_ios);

  list_add(&io->list, &dev->ios);

  /* Always do this to avoid race conditions */
  wakeup_event_thread();

  return 0;
}

int usbi_os_io_complete(struct usbi_dev_handle *dev)
{
  struct usbk_urb *urb;
  struct usbi_io *io;
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_REAPURBNDELAY, (void *)&urb);
  if (ret < 0) {
    usbi_debug(1, "error reaping URB: %s", strerror(errno));
    return -EINVAL;
  }

  io = urb->usercontext;
  list_del(&io->list);		/* lock obtained by caller */

  /* FIXME: Translate the status code */

  if (io->setup)
    memcpy(io->buffer, io->urb.buffer + USBI_CONTROL_SETUP_LEN, io->bufferlen);

  usbi_io_complete(io, urb->status, urb->actual_length);

  if (list_empty(&io->dev->ios))
    list_del(&io->dev->io_list);

  return 0;
}

int usbi_os_io_cancel(struct usbi_io *io)
{
  int ret;

  ret = ioctl(io->dev->fd, IOCTL_USB_DISCARDURB, &io->urb);
  if (ret < 0) {
    usbi_debug(1, "error cancelling URB: %s", strerror(errno));
    return -EINVAL;
  }

  /* Always do this to avoid race conditions */
  wakeup_event_thread();

  return 0;
}

void usbi_poll_events()
{
  do {
    struct usbi_dev_handle *dev, *tdev;
    fd_set readfds, writefds;
    int ret, maxfd;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    maxfd=-1;
    list_for_each_entry(dev, &usbi_ios, io_list) {
      FD_SET(dev->fd, &writefds);
      if (dev->fd > maxfd)
        maxfd = dev->fd;
    }
    tv.tv_usec=0;
    tv.tv_sec=0;

    ret = select(maxfd + 1, &readfds, &writefds, NULL, &tv);
    if (ret < 0) {
      usbi_debug(1, "select() call failed: %s", strerror(errno));
      continue;
    }

    list_for_each_entry_safe(dev, tdev, &usbi_ios, io_list) {
      if (FD_ISSET(dev->fd, &writefds))
        usbi_os_io_complete(dev);

      if (list_empty(&dev->ios))
        list_del(&dev->io_list);
    }
  } while (0);
}

int usb_io_wait_handle(usb_io_handle_t *io)
{
  return io->dev->fd;
}

int usbi_os_find_busses(struct list_head *busses)
{
  DIR *dir;
  struct dirent *entry;

  /* Scan /proc/bus/usb for bus named directories */

  dir = opendir(usbi_path);
  if (!dir)
    USB_ERROR_STR(-errno, "couldn't opendir(%s): %s", usbi_path,
	strerror(errno));

  while ((entry = readdir(dir)) != NULL) {
    struct usbi_bus *ibus;

    /* Skip anything starting with a . */
    if (entry->d_name[0] == '.')
      continue;

    if (!strchr("0123456789", entry->d_name[strlen(entry->d_name) - 1])) {
      usbi_debug(2, "skipping non bus dir %s", entry->d_name);
      continue;
    }

    ibus = malloc(sizeof(*ibus));
    if (!ibus)
      USB_ERROR(-ENOMEM);

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

static int device_is_new(struct usbi_bus *ibus, unsigned short devnum)
{
  char filename[PATH_MAX + 1];
  struct usbi_device *idev;
  struct stat st;

  /* If we don't have a device by this number yet, it must be new */
  idev = ibus->dev_by_num[devnum];
  if (!idev)
    return 1;

  /* Compare the mtime to ensure it's new */
  snprintf(filename, sizeof(filename) - 1, "%s/%03d", ibus->filename, devnum);
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
    USB_ERROR(-ENOMEM);

  memset(idev, 0, sizeof(*idev));

  idev->devnum = devnum;
  snprintf(idev->filename, sizeof(idev->filename) - 1, "%s/%03d",
	ibus->filename, idev->devnum);

  idev->num_ports = max_children;
  if (max_children) {
    idev->children = malloc(idev->num_ports * sizeof(idev->children[0]));
    if (!idev->children) {
      free(idev);
      USB_ERROR(-ENOMEM);
    }

    memset(idev->children, 0, idev->num_ports * sizeof(idev->children[0]));
  }

  fd = device_open(idev);
  if (fd < 0) {
    usbi_debug(2, "couldn't open %s: %s", idev->filename, strerror(errno));

    free(idev);
    USB_ERROR(-ENODEV);
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

    cfgr->len = usb_le16_to_cpup((uint16_t *)&buf[2]);

    cfgr->data = malloc(cfgr->len);
    if (!cfgr->data) {
      usbi_debug(1, "unable to allocate memory for descriptors");
      ret = -ENOMEM;
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
  if (!f)
    USB_ERROR_STR(-errno, "couldn't open %s: %s", devfilename, strerror(errno));

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

    /* We need character and colon to start the line */
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

      if (!pdevnum && ibus->root) {
        usbi_debug(1, "cannot have two root devices");
        break;
      }

      /* Only add this device if it's new */
      if (device_is_new(ibus, devnum)) {
        int ret;

        ret = create_new_device(&idev, ibus, devnum, max_children);
        if (ret) {
          usbi_debug(1, "ignoring new device because of errors");
          break;
        }

        idev->found = 1;

        usbi_add_device(ibus, idev);
        ibus->dev_by_num[devnum] = idev;

        /* Setup parent/child relationship */
        if (pdevnum) {
          ibus->dev_by_num[pdevnum]->children[pport] = idev;
          idev->parent = ibus->dev_by_num[pdevnum];
        } else
          ibus->root = idev;
      }
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

int usb_clear_halt(usb_dev_handle_t *dev, unsigned char ep)
{
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_CLEAR_HALT, &ep);
  if (ret)
    USB_ERROR_STR(ret, "could not clear/halt ep %d: %s", ep,
    	strerror(errno));

  return 0;
}

int usb_reset(usb_dev_handle_t *dev)
{
  int ret;

  ret = ioctl(dev->fd, IOCTL_USB_RESET, NULL);
  if (ret)
     USB_ERROR_STR(ret, "could not reset: %s", strerror(errno));

  return 0;
}

int usb_get_driver_np(usb_dev_handle_t *dev, int interface, char *name,
	unsigned int namelen)
{
  struct usbk_getdriver getdrv;
  int ret;

  getdrv.interface = interface;
  ret = ioctl(dev->fd, IOCTL_USB_GETDRIVER, &getdrv);
  if (ret)
    USB_ERROR_STR(-errno, "could not get bound driver: %s", strerror(errno));

  strncpy(name, getdrv.driver, namelen - 1);
  name[namelen - 1] = 0;

  return 0;
}

int usb_attach_kernel_driver_np(usb_dev_handle_t *dev, int interface)
{
  struct usbk_ioctl command;
  int ret;

  command.ifno = interface;
  command.ioctl_code = IOCTL_USB_CONNECT;
  command.data = NULL;

  ret = ioctl(dev->fd, IOCTL_USB_IOCTL, &command);
  if (ret)
    USB_ERROR_STR(-errno, "could not attach kernel driver to interface %d: %s",
        interface, strerror(errno));

  return 0;
}

int usb_detach_kernel_driver_np(usb_dev_handle_t *dev, int interface)
{
  struct usbk_ioctl command;
  int ret;

  command.ifno = interface;
  command.ioctl_code = IOCTL_USB_DISCONNECT;
  command.data = NULL;

  ret = ioctl(dev->fd, IOCTL_USB_IOCTL, &command);
  if (ret)
    USB_ERROR_STR(-errno, "could not detach kernel driver from interface %d: %s",
        interface, strerror(errno));

  return 0;
}

