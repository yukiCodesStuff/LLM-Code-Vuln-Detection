/*
 *  HID driver for Speedlink Vicious and Divine Cezanne (USB mouse).
 *  Fixes "jumpy" cursor and removes nonexistent keyboard LEDS from
 *  the HID descriptor.
 *
 *  Copyright (c) 2011, 2013 Stefan Kriwanek <dev@stefankriwanek.de>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"

static const struct hid_device_id speedlink_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_X_TENSIONS, USB_DEVICE_ID_SPEEDLINK_VAD_CEZANNE)},
	{ }
{
	/* No other conditions due to usage_table. */

	/* This fixes the "jumpy" cursor occuring due to invalid events sent
	 * by the device. Some devices only send them with value==+256, others
	 * don't. However, catching abs(value)>=256 is restrictive enough not
	 * to interfere with devices that were bug-free (has been tested).
	 */
	if (abs(value) >= 256)
		return 1;
	/* Drop useless distance 0 events (on button clicks etc.) as well */
	if (value == 0)
		return 1;

	return 0;
}