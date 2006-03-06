/*
 * Handling of busses and devices
 *
 * Copyright 2000-2005 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL, read LICENSE for details.
 */

#include <errno.h>
#include <stdlib.h>	/* malloc, free */
#include <string.h>	/* memcpy */

#include "usbi.h"

static struct list_head usbi_busses = { .prev = &usbi_busses, .next = &usbi_busses };
static struct list_head usbi_devices = { .prev = &usbi_devices, .next = &usbi_devices };

static libusb_bus_id_t cur_bus_id = 1;
static libusb_device_id_t cur_device_id = 1;

/*
 * Bus code
 */
void usbi_add_bus(struct usbi_bus *ibus)
{
  /* FIXME: Handle busid rollover gracefully? */
  ibus->busid = cur_bus_id++;

  list_init(&ibus->devices);

  list_add(&ibus->list, &usbi_busses);
}

void usbi_free_bus(struct usbi_bus *ibus)
{
  free(ibus);
}

void usbi_remove_bus(struct usbi_bus *ibus)
{
  list_del(&ibus->list);
  usbi_free_bus(ibus);
}

struct usbi_bus *usbi_find_bus_by_id(libusb_bus_id_t busid)
{
  struct usbi_bus *ibus;

  /* FIXME: We should probably index the device id in a rbtree or something */
  list_for_each_entry(ibus, &usbi_busses, list) {
    if (ibus->busid == busid)
      return ibus;
  }

  return NULL;
}

static void usbi_refresh_busses(void)
{
  struct list_head busses;
  struct usbi_bus *ibus, *tibus;
  int ret;

  list_init(&busses);

  ret = usbi_os_find_busses(&busses);
  if (ret < 0)
    return;

  /*
   * Now walk through all of the busses we know about and compare against
   * this new list. Any duplicates will be removed from the new list.
   * If we don't find it in the new list, the bus was removed. Any
   * busses still in the new list, are new to us.
   */
  list_for_each_entry_safe(ibus, tibus, &usbi_busses, list) {
    struct usbi_bus *nibus, *tnibus;
    int found = 0;

    list_for_each_entry_safe(nibus, tnibus, &busses, list) {
      if (ibus->busnum != nibus->busnum) {
        /* Remove it from the new devices list */
        list_del(&nibus->list);

        usbi_free_bus(nibus);
        found = 1;
        break;
      }
    }

    if (!found)
      /* The device was removed from the system */
      list_del(&ibus->list);
  }

  /*
   * Anything on the *busses list is new. So add them to usbi_busses
   * and process them like the new bus they are
   */
  list_for_each_entry_safe(ibus, tibus, &busses, list) {
    list_del(&ibus->list);
    usbi_add_bus(ibus);
  }
}

int libusb_get_first_bus_id(libusb_bus_id_t *busid)
{
  struct list_head *tmp;

  if (list_empty(&usbi_busses))
    return 0;

  tmp = usbi_busses.next;
  *busid = list_entry(tmp, struct usbi_bus, list)->busid;

  return 0;
}

/*
 * FIXME: It would be nice if we can handle the case where the bus id passed
 * to the next/prev functions didn't exist. Maybe we can switch to an rbtree
 * and find the next bus id in the list?
 */
int libusb_get_next_bus_id(libusb_bus_id_t *busid)
{
  struct usbi_bus *ibus;
  struct list_head *tmp;

  ibus = usbi_find_bus_by_id(*busid);
  if (!ibus)
    return LIBUSB_FAILURE;

  tmp = ibus->list.next;
  if (tmp == &usbi_busses)
    return LIBUSB_FAILURE;

  *busid = list_entry(tmp, struct usbi_bus, list)->busid;

  return 0;
}

int libusb_get_prev_bus_id(libusb_bus_id_t *busid)
{
  struct usbi_bus *ibus;
  struct list_head *tmp;

  ibus = usbi_find_bus_by_id(*busid);
  if (!ibus)
    return LIBUSB_FAILURE;

  tmp = ibus->list.prev;
  if (tmp == &usbi_busses)
    return LIBUSB_FAILURE;

  *busid = list_entry(tmp, struct usbi_bus, list)->busid;

  return 0;
}

int libusb_get_busnum(libusb_bus_id_t busid)
{
  struct usbi_bus *ibus;

  ibus = usbi_find_bus_by_id(busid);
  if (!ibus)
    return -ENOENT;

  return ibus->busnum;
}

/*
 * Device code
 */
void usbi_add_device(struct usbi_bus *ibus, struct usbi_device *idev)
{
  /* FIXME: Handle devid rollover gracefully? */
  idev->devid = cur_device_id++;

  idev->bus = ibus;

  list_add(&idev->bus_list, &ibus->devices);
  list_add(&idev->dev_list, &usbi_devices);

  usbi_callback(idev->devid, USB_ATTACH);
}

void usbi_remove_device(struct usbi_device *idev)
{
  libusb_device_id_t devid = idev->devid;

  list_del(&idev->bus_list);
  list_del(&idev->dev_list);

  usbi_destroy_configuration(idev);
  free(idev);

  usbi_callback(devid, USB_DETACH);
}

struct usbi_device *usbi_find_device_by_id(libusb_device_id_t devid)
{
  struct usbi_device *idev;

  /* FIXME: We should probably index the device id in a rbtree or something */
  list_for_each_entry(idev, &usbi_devices, dev_list) {
    if (idev->devid == devid)
      return idev;
  }

  return NULL;
}

struct usbi_device *usbi_find_device_by_devnum(struct usbi_bus *ibus,
	unsigned int devnum)
{
  struct usbi_device *idev;

  /* FIXME: We should probably index the device num in a rbtree or something */
  list_for_each_entry(idev, &ibus->devices, bus_list) {
    if (idev->devnum == devnum)
      return idev;
  }

  return NULL;
}

void usbi_rescan_devices(void)
{
  struct usbi_bus *ibus;

  usbi_refresh_busses();

  list_for_each_entry(ibus, &usbi_busses, list) {
    usbi_os_refresh_devices(ibus);

#if 0
      /*
       * Some platforms fetch the descriptors on scanning (like Linux) so we
       * don't need to fetch them again
       */
      if (!idev->desc.device_raw.data) {
        libusb_dev_handle_t udev;

        ret = libusb_open(idev->devid, &udev);
        if (ret >= 0) {
          usbi_fetch_and_parse_descriptors(udev);

          libusb_close(udev);
        }
      }

      /* FIXME: Handle checking the device and config descriptor seperately */
#endif
  }
}

static int add_match_to_list(struct usbi_match *match, struct usbi_device *idev)
{
  if (match->num_matches == match->alloc_matches) {
    match->alloc_matches += 16;
    match->matches = realloc(match->matches, match->alloc_matches * sizeof(match->matches[0]));
    if (!match->matches)
      return -ENOMEM;
  }

  match->matches[match->num_matches++] = idev->devid;

  return 0;
}

static int match_interfaces(struct usbi_device *idev,
	int bClass, int bSubClass, int bProtocol)
{
  int c;

  if (bClass < 0 && bSubClass < 0 && bProtocol < 0)
    return 1;

  /* Now check all of the configs/interfaces/altsettings */
  for (c = 0; c < idev->desc.num_configs; c++) {
    struct usbi_config *cfg = &idev->desc.configs[c];
    int i;

    for (i = 0; i < cfg->num_interfaces; i++) {
      struct usbi_interface *intf = &cfg->interfaces[i];
      int a;

      for (a = 0; a < intf->num_altsettings; a++) {
        struct usb_interface_desc *as = &intf->altsettings[a].desc;

        if ((bClass < 0 || bClass == as->bInterfaceClass) &&
            (bSubClass < 0 || bSubClass == as->bInterfaceSubClass) &&
            (bProtocol < 0 || bProtocol == as->bInterfaceProtocol))
          return 1;
      }
    }
  }

  return 0;
}

static struct list_head usbi_match_handles = { .prev = &usbi_match_handles, .next = &usbi_match_handles };

static libusb_match_handle_t cur_match_handle = 1;

struct usbi_match *usbi_find_match(libusb_match_handle_t handle)
{
  struct usbi_match *match;

  /* FIXME: We should probably index the device id in a rbtree or something */
  list_for_each_entry(match, &usbi_match_handles, list) {
    if (match->handle == handle)
      return match;
  }

  return NULL;
}

int libusb_match_devices_by_vendor(libusb_match_handle_t *handle,
        int vendor, int product)
{
  struct usbi_match *match;
  struct usbi_device *idev;

  if (vendor < -1 || vendor > 0xffff || product < -1 || product > 0xffff)
    return -EINVAL;

  match = malloc(sizeof(*match));
  if (!match)
    return -ENOMEM;

  memset(match, 0, sizeof(*match));

  match->handle = cur_match_handle++;	/* FIXME: Locking */

  list_for_each_entry(idev, &usbi_devices, dev_list) {
    struct usb_device_desc *desc = &idev->desc.device;

    if ((vendor < 0 || vendor == desc->idVendor) &&
        (product < 0 || product == desc->idProduct))
      add_match_to_list(match, idev);
  }

  *handle = match->handle;

  return 0;
}

int libusb_match_devices_by_class(libusb_match_handle_t *handle,
        int bClass, int bSubClass, int bProtocol)
{
  struct usbi_match *match;
  struct usbi_device *idev;

  if (bClass < -1 || bClass > 0xff || bSubClass < -1 || bSubClass > 0xff ||
      bProtocol < -1 || bProtocol > 0xff)
    return -EINVAL;

  match = malloc(sizeof(*match));
  if (!match)
    return -ENOMEM;

  memset(match, 0, sizeof(*match));

  match->handle = cur_match_handle++;	/* FIXME: Locking */

  list_for_each_entry(idev, &usbi_devices, dev_list) {
    if (match_interfaces(idev, bClass, bSubClass, bProtocol))
      add_match_to_list(match, idev);
  }

  *handle = match->handle;

  return 0;
}

int libusb_match_next_device(libusb_match_handle_t handle,
	libusb_device_id_t *mdevid)
{
  struct usbi_match *match;

  match = usbi_find_match(handle);
  if (!match)
    return LIBUSB_UNKNOWN_DEVICE;	/* FIXME: Better error code */

  while (match->cur_match < match->num_matches) {
    struct usbi_device *idev;
    libusb_device_id_t devid;

    devid = match->matches[match->cur_match++];
    idev = usbi_find_device_by_id(devid);
    if (idev) {
      *mdevid = devid;
      return 0;
    }
  }

  return -ESRCH;
}

int libusb_free_match(libusb_match_handle_t handle)
{
  struct usbi_match *match;

  match = usbi_find_match(handle);
  if (!match)
    return LIBUSB_UNKNOWN_DEVICE;	/* FIXME: Better error code */

  list_del(&match->list);
  free(match->matches);
  free(match);

  return 0;
}

/* Topology operations */
int libusb_get_first_device_id(libusb_bus_id_t busid,
	libusb_device_id_t *devid)
{
  struct usbi_bus *ibus;

  ibus = usbi_find_bus_by_id(busid);
  if (!ibus)
    return LIBUSB_UNKNOWN_DEVICE;

  if (!ibus->root)
    return LIBUSB_FAILURE;

  *devid = ibus->root->devid;

  return 0;
}

/*
 * FIXME: It would be nice if we can handle the case where the dev id passed
 * to the next/prev functions didn't exist. Maybe we can switch to an rbtree
 * and find the next dev id in the list?
 */
int libusb_get_next_device_id(libusb_device_id_t *devid)
{
  struct usbi_device *idev;
  struct list_head *tmp;

  idev = usbi_find_device_by_id(*devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  tmp = idev->dev_list.next;
  if (tmp == &usbi_devices)
    return LIBUSB_FAILURE;

  *devid = list_entry(tmp, struct usbi_device, dev_list)->devid;

  return 0;
}

int libusb_get_prev_device_id(libusb_device_id_t *devid)
{
  struct usbi_device *idev;
  struct list_head *tmp;

  idev = usbi_find_device_by_id(*devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  tmp = idev->dev_list.prev;
  if (tmp == &usbi_devices)
    return LIBUSB_FAILURE;

  *devid = list_entry(tmp, struct usbi_device, dev_list)->devid;

  return 0;
}

int libusb_get_child_count(libusb_device_id_t devid, unsigned char *count)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  *count = idev->num_ports;

  return 0;
}

int libusb_get_child_device_id(libusb_device_id_t hub_devid, int port,
	libusb_device_id_t *child_devid)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(hub_devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  port--;	/* 1-indexed */
  if (port < 0 || port > idev->num_ports)
    return LIBUSB_BADARG;

  if (!idev->children[port])
    return LIBUSB_BADARG;

  *child_devid = idev->children[port]->devid;

  return 0;
}

int libusb_get_parent_device_id(libusb_device_id_t child_devid,
	libusb_device_id_t *hub_devid)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(child_devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  if (!idev->parent)
    return LIBUSB_BADARG;

  *hub_devid = idev->parent->devid;

  return 0;
}

int libusb_get_bus_id(libusb_device_id_t devid, libusb_bus_id_t *busid)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  *busid = idev->bus->busid;

  return 0;
}

int libusb_get_devnum(libusb_device_id_t devid, unsigned char *devnum)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return LIBUSB_UNKNOWN_DEVICE;

  *devnum = idev->devnum;

  return 0;
}

/* Descriptor operations */
int libusb_get_device_desc(libusb_device_id_t devid,
	struct usb_device_desc *devdsc)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  memcpy(devdsc, &idev->desc.device, sizeof(*devdsc));

  return 0;
}

int libusb_get_config_desc(libusb_device_id_t devid, int cfgidx,
        struct usb_config_desc *cfgdsc)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  if (cfgidx < 0 || cfgidx >= idev->desc.num_configs)
    return -EINVAL;

  memcpy(cfgdsc, &idev->desc.configs[cfgidx], sizeof(*cfgdsc));

  return 0;
}

int libusb_get_interface_desc(libusb_device_id_t devid, int cfgidx, int ifcidx,
        struct usb_interface_desc *ifcdsc)
{
  struct usbi_device *idev;
  struct usbi_config *cfg;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  if (cfgidx < 0 || cfgidx >= idev->desc.num_configs)
    return -EINVAL;

  cfg = &idev->desc.configs[cfgidx];

  if (ifcidx < 0 || ifcidx >= cfg->num_interfaces)
    return -EINVAL;

  memcpy(ifcdsc, &cfg->interfaces[ifcidx].altsettings[0].desc, sizeof(*ifcdsc));

  return 0;
}

int libusb_get_endpoint_desc(libusb_device_id_t devid, int cfgidx, int ifcidx,
        int eptidx, struct usb_endpoint_desc *eptdsc)
{
  struct usbi_device *idev;
  struct usbi_config *cfg;
  struct usbi_altsetting *as;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  if (cfgidx < 0 || cfgidx >= idev->desc.num_configs)
    return -EINVAL;

  cfg = &idev->desc.configs[cfgidx];

  if (ifcidx < 0 || ifcidx >= cfg->num_interfaces)
    return -EINVAL;

  as = &cfg->interfaces[ifcidx].altsettings[0];

  if (eptidx < 0 || eptidx >= as->num_endpoints)
    return -EINVAL;

  memcpy(eptdsc, &as->endpoints[eptidx].desc, sizeof(*eptdsc));

  return 0;
}

int libusb_get_raw_device_desc(libusb_device_id_t devid,
	unsigned char *buffer, size_t buflen)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  if (idev->desc.device_raw.len < buflen)
    buflen = idev->desc.device_raw.len;

  memcpy(buffer, idev->desc.device_raw.data, buflen);

  return idev->desc.device_raw.len;
}

int libusb_get_raw_config_desc(libusb_device_id_t devid,
	int cfgidx, unsigned char *buffer, size_t buflen)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  if (cfgidx < 0 || cfgidx >= idev->desc.num_configs)
    return -EINVAL;

  if (idev->desc.configs_raw[cfgidx].len < buflen)
    buflen = idev->desc.configs_raw[cfgidx].len;

  memcpy(buffer, idev->desc.configs_raw[cfgidx].data, buflen);

  return idev->desc.configs_raw[cfgidx].len;
}

int libusb_refresh_descriptors(libusb_device_id_t devid)
{
  struct usbi_device *idev;

  idev = usbi_find_device_by_id(devid);
  if (!idev)
    return -ENOENT;

  /* FIXME: Implement */

  return 0;
}

