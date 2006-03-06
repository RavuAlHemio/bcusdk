/*
 * Parses descriptors
 *
 * Copyright 2001-2005 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL, read LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>	/* malloc, free */
#include <string.h>	/* memset */

#include "usbi.h"

int libusb_get_descriptor(libusb_dev_handle_t dev, unsigned char type,
	unsigned char index, void *buf, unsigned int buflen)
{
  return libusb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
                        (type << 8) + index, 0, buf, buflen, 1000);
}

/*
 * This code looks surprisingly similar to the code I wrote for the Linux
 * kernel. It's not a coincidence :)
 */

/* FIXME: Audit all of the increments to make sure we skip descriptors correctly on errors */

static int usbi_parse_endpoint(struct usbi_endpoint *ep,
	unsigned char *buf, unsigned int buflen)
{
  uint8_t desc_len, desc_type;
  int parsed = 0, numskipped = 0;

  desc_len = buf[0];
  desc_type = buf[1];

  /* Everything should be fine being passed into here, but sanity check JIC */
  if (desc_len > buflen) {
    usbi_debug(1, "ran out of descriptors parsing");
    return -1;
  }
                
  if (desc_type != USB_DESC_TYPE_ENDPOINT) {
    usbi_debug(2, "unexpected descriptor 0x%X, expecting endpoint descriptor, type 0x%X",
	desc_type, USB_DESC_TYPE_ENDPOINT);
    return parsed;
  }

  if (desc_len < USBI_ENDPOINT_DESC_SIZE) {
    usbi_debug(2, "endpoint descriptor too short. only %d bytes long",
	desc_len);
    return parsed;
  }

  ep->desc.bEndpointAddress = buf[2];
  ep->desc.bmAttributes = buf[3];
  ep->desc.wMaxPacketSize = libusb_le16_to_cpup((uint16_t *)&buf[4]);
  ep->desc.bInterval = buf[6];
  ep->desc.bRefresh = buf[7];
  ep->desc.bSynchAddress = buf[8];

  /* FIXME: Maybe report about extra unparsed data after the descriptor? */

  /* Skip over the just parsed data */
  buf += desc_len;
  buflen -= desc_len;
  parsed += desc_len;

  /* Skip over the rest of the Class Specific or Vendor Specific descriptors */
  while (buflen >= USBI_DESC_HEADER_SIZE) {
    desc_len = buf[0];
    desc_type = buf[1];

    if (desc_len < USBI_DESC_HEADER_SIZE) {
      usbi_debug(1, "invalid descriptor length of %d", desc_len);
      return -1;
    }

    /* If we find another "proper" descriptor then we're done  */
    if (desc_type == USB_DESC_TYPE_ENDPOINT || desc_type == USB_DESC_TYPE_INTERFACE ||
        desc_type == USB_DESC_TYPE_CONFIG || desc_type == USB_DESC_TYPE_DEVICE)
      break;

    usbi_debug(1, "skipping descriptor 0x%X", desc_type);

    numskipped++;

    buf += desc_len;
    buflen -= desc_len;
    parsed += desc_len;
  }

  if (numskipped)
    usbi_debug(2, "skipped %d class/vendor specific endpoint descriptors",
	numskipped);

  return parsed;
}

static int usbi_parse_interface(struct usbi_interface *intf,
	unsigned char *buf, unsigned int buflen)
{
  int i, retval, parsed = 0, numskipped;
  uint8_t desc_len, desc_type, alt_num;
  struct usbi_altsetting *as;

  intf->num_altsettings = 0;

  while (buflen > 0) {
    desc_len = buf[0];
    desc_type = buf[1];

    intf->altsettings = realloc(intf->altsettings, sizeof(intf->altsettings[0]) * (intf->num_altsettings + 1));
    if (!intf->altsettings) {
      intf->num_altsettings = 0;
      usbi_debug(1, "couldn't allocated %d bytes for altsettings",
	sizeof(intf->altsettings[0]) * (intf->num_altsettings + 1));

      return -1;
    }

    as = intf->altsettings + intf->num_altsettings;
    intf->num_altsettings++;

    as->desc.bInterfaceNumber = buf[2];
    as->desc.bAlternateSetting = buf[3];
    as->desc.bNumEndpoints = buf[4];
    as->desc.bInterfaceClass = buf[5];
    as->desc.bInterfaceSubClass = buf[6];
    as->desc.bInterfaceProtocol = buf[7];
    as->desc.iInterface = buf[8];

    /* Skip over the interface */
    buf += desc_len;
    parsed += desc_len;
    buflen -= desc_len;

    numskipped = 0;

    /* Skip over any interface, class or vendor descriptors */
    while (buflen >= USBI_DESC_HEADER_SIZE) {
      desc_len = buf[0];
      desc_type = buf[1];

      if (desc_len < USBI_DESC_HEADER_SIZE) {
        usbi_debug(1, "invalid descriptor length of %d", desc_len);
        return -1;
      }

      /* If we find another "proper" descriptor then we're done */
      if (desc_type == USB_DESC_TYPE_INTERFACE || desc_type == USB_DESC_TYPE_ENDPOINT ||
          desc_type == USB_DESC_TYPE_CONFIG || desc_type == USB_DESC_TYPE_DEVICE)
        break;

      numskipped++;

      buf += desc_len;
      parsed += desc_len;
      buflen -= desc_len;
    }

    if (numskipped)
      usbi_debug(2, "skipped %d class/vendor specific interface descriptors",
	numskipped);

    /* Did we hit an unexpected descriptor? */
    desc_len = buf[0];
    desc_type = buf[1];
    if (buflen >= USBI_DESC_HEADER_SIZE &&
        (desc_type == USB_DESC_TYPE_CONFIG || desc_type == USB_DESC_TYPE_DEVICE))
      return parsed;

    if (as->desc.bNumEndpoints > USBI_MAXENDPOINTS) {
      usbi_debug(1, "too many endpoints, ignoring rest");

      return -1;
    }

    as->endpoints = malloc(as->desc.bNumEndpoints *
                     sizeof(struct usb_endpoint_desc));
    if (!as->endpoints) {
      usbi_debug(1, "couldn't allocated %d bytes for endpoints",
	as->desc.bNumEndpoints * sizeof(struct usb_endpoint_desc));
      return -1;      
    }
    as->num_endpoints = as->desc.bNumEndpoints;

    memset(as->endpoints, 0, as->num_endpoints * sizeof(as->endpoints[0]));

    for (i = 0; i < as->num_endpoints; i++) {
      desc_len = buf[0];
      desc_type = buf[1];

      if (desc_len > buflen) {
        usbi_debug(1, "ran out of descriptors parsing");
        return -1;
      }
                
      retval = usbi_parse_endpoint(as->endpoints + i, buf, buflen);
      if (retval < 0)
        return retval;

      buf += retval;
      parsed += retval;
      buflen -= retval;
    }

    /* We check to see if it's an alternate to this one */
    desc_type = buf[1];
    alt_num = buf[3];
    if (buflen < USBI_INTERFACE_DESC_SIZE ||
        desc_type != USB_DESC_TYPE_INTERFACE ||
        !alt_num)
      return parsed;
  }

  return parsed;
}

int usbi_parse_configuration(struct usbi_config *cfg, unsigned char *buf,
	size_t buflen)
{
  uint8_t desc_len, desc_type;
  int i, retval;

  desc_len = buf[0];
  desc_type = buf[1];

  cfg->desc.wTotalLength = libusb_le16_to_cpup((uint16_t *)&buf[2]);
  cfg->desc.bNumInterfaces = buf[4];
  cfg->desc.bConfigurationValue = buf[5];
  cfg->desc.iConfiguration = buf[6];
  cfg->desc.bmAttributes = buf[7];
  cfg->desc.MaxPower = buf[8];

  if (cfg->desc.bNumInterfaces > USBI_MAXINTERFACES) {
    usbi_debug(1, "too many interfaces, ignoring rest");
    return -1;
  }

  cfg->interfaces = malloc(cfg->desc.bNumInterfaces * sizeof(cfg->interfaces[0]));
  if (!cfg->interfaces) {
    usbi_debug(1, "couldn't allocated %d bytes for interfaces",
	cfg->desc.bNumInterfaces * sizeof(cfg->interfaces[0]));
    return -1;      
  }

  cfg->num_interfaces = cfg->desc.bNumInterfaces;

  memset(cfg->interfaces, 0, cfg->num_interfaces * sizeof(cfg->interfaces[0]));

  buf += desc_len;
  buflen -= desc_len;
        
  for (i = 0; i < cfg->num_interfaces; i++) {
    int numskipped = 0;

    /* Skip over the rest of the Class specific or Vendor specific descriptors */
    while (buflen >= USBI_DESC_HEADER_SIZE) {
      desc_len = buf[0];
      desc_type = buf[1];

      if (desc_len > buflen || desc_len < USBI_DESC_HEADER_SIZE) {
        usbi_debug(1, "invalid descriptor length of %d", desc_len);
        return -1;
      }

      /* If we find another "proper" descriptor then we're done */
      if (desc_type == USB_DESC_TYPE_ENDPOINT || desc_type == USB_DESC_TYPE_INTERFACE ||
          desc_type == USB_DESC_TYPE_CONFIG || desc_type == USB_DESC_TYPE_DEVICE)
        break;

      usbi_debug(2, "skipping descriptor 0x%X", desc_type);
      numskipped++;

      buf += desc_len;
      buflen -= desc_len;
    }

    if (numskipped)
      usbi_debug(2, "skipped %d class/vendor specific endpoint descriptors\n",
	numskipped);

    retval = usbi_parse_interface(cfg->interfaces + i, buf, buflen);
    if (retval < 0)
      return retval;

    buf += retval;
    buflen -= retval;
  }

  return buflen;
}

void usbi_destroy_configuration(struct usbi_device *dev)
{
  int c, i, j;
        
  for (c = 0; c < dev->desc.num_configs; c++) {
    struct usbi_config *cfg = dev->desc.configs + c;

    for (i = 0; i < cfg->num_interfaces; i++) {
      struct usbi_interface *intf = cfg->interfaces + i;
                                
      for (j = 0; j < intf->num_altsettings; j++) {
        struct usbi_altsetting *as = intf->altsettings + j;
                                        
        free(as->endpoints);
      }

      free(intf->altsettings);
    }

    free(cfg->interfaces);
    free(dev->desc.configs_raw[c].data);
  }

  free(dev->desc.configs_raw);
  free(dev->desc.configs);

  free(dev->desc.device_raw.data);
}

int usbi_fetch_and_parse_descriptors(struct usbi_dev_handle *hdev)
{
  struct usbi_device *dev = hdev->idev;
  int i;

  dev->desc.num_configs = dev->desc.device.bNumConfigurations;

  if (dev->desc.num_configs > USBI_MAXCONFIG) {
    usbi_debug(1, "too many configurations (%d > %d)", 
	dev->desc.num_configs, USBI_MAXCONFIG);
    goto err;
  }

  if (dev->desc.num_configs < 1) {
    usbi_debug(1, "not enough configurations (%d < 1)",
	dev->desc.num_configs);
    goto err;
  }

  dev->desc.configs_raw = malloc(dev->desc.num_configs * sizeof(dev->desc.configs_raw[0]));
  if (!dev->desc.configs_raw) {
    usbi_debug(1, "unable to allocate %d bytes for cached descriptors",
	dev->desc.num_configs * sizeof(dev->desc.configs_raw[0]));
    goto err;
  }

  memset(dev->desc.configs_raw, 0, dev->desc.num_configs * sizeof(dev->desc.configs_raw[0]));

  dev->desc.configs = malloc(dev->desc.num_configs * sizeof(dev->desc.configs[0]));
  if (!dev->desc.configs) {
    usbi_debug(1, "unable to allocate memory for config descriptors",
	dev->desc.num_configs * sizeof(dev->desc.configs[0]));
    goto err;
  }

  memset(dev->desc.configs, 0, dev->desc.num_configs * sizeof(dev->desc.configs[0]));

  for (i = 0; i < dev->desc.num_configs; i++) {
    char buf[8];
    struct usbi_raw_desc *cfgr = dev->desc.configs_raw + i;
    int ret;

    /* Get the first 8 bytes so we can figure out what the total length is */
    ret = libusb_get_descriptor(hdev->handle, USB_DESC_TYPE_CONFIG, i, buf, 8);
    if (ret < 8) {
      if (ret < 0)
        usbi_debug(1, "unable to get first 8 bytes of config descriptor (ret = %d)",
		ret);
      else
        usbi_debug(1, "config descriptor too short (expected 8, got %d)", ret);

      goto err;
    }

    cfgr->len = libusb_le16_to_cpup((uint16_t *)&buf[2]);

    cfgr->data = malloc(cfgr->len);
    if (!cfgr->data) {
      usbi_debug(1, "unable to allocate %d bytes for descriptors", cfgr->len);
      goto err;
    }

    ret = libusb_get_descriptor(hdev->handle, USB_DESC_TYPE_CONFIG, i, cfgr->data, cfgr->len);
    if (ret < cfgr->len) {
      if (ret < 0)
        usbi_debug(1, "unable to get rest of config descriptor (ret = %d)",
		ret);
      else
        usbi_debug(1, "config descriptor too short (expected %d, got %d)",
		cfgr->len, ret);

      cfgr->len = 0;
      free(cfgr->data);
      goto err;
    }

    ret = usbi_parse_configuration(dev->desc.configs + i, cfgr->data, cfgr->len);
    if (ret > 0)
      usbi_debug(2, "%d bytes of descriptor data still left", ret);
    else if (ret < 0)
      usbi_debug(2, "unable to parse descriptors");
  }

  return 0;

err:
  /* FIXME: Free already allocated config descriptors too */
  free(dev->desc.configs);
  free(dev->desc.configs_raw);

  dev->desc.configs = NULL;
  dev->desc.configs_raw = NULL;

  dev->desc.num_configs = 0;

  return 1;
}

int usbi_parse_device_descriptor(struct usbi_device *dev,
	unsigned char *buf, size_t buflen)
{
  uint8_t desc_len;

  desc_len = buf[0];

  /* FIXME: Verify size and type? */

  dev->desc.device.bcdUSB = libusb_le16_to_cpup((uint16_t *)&buf[2]);
  dev->desc.device.bDeviceClass = buf[4];
  dev->desc.device.bDeviceSubClass = buf[5];
  dev->desc.device.bDeviceProtocol = buf[6];
  dev->desc.device.bMaxPacketSize0 = buf[7];
  dev->desc.device.idVendor = libusb_le16_to_cpup((uint16_t *)&buf[8]);
  dev->desc.device.idProduct = libusb_le16_to_cpup((uint16_t *)&buf[10]);
  dev->desc.device.bcdDevice = libusb_le16_to_cpup((uint16_t *)&buf[12]);
  dev->desc.device.iManufacturer = buf[14];
  dev->desc.device.iProduct = buf[15];
  dev->desc.device.iSerialNumber = buf[16];
  dev->desc.device.bNumConfigurations = buf[17];

  return desc_len;
}

