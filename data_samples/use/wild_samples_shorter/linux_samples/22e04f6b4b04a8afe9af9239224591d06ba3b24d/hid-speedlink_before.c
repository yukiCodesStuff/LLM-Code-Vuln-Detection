 *  Fixes "jumpy" cursor and removes nonexistent keyboard LEDS from
 *  the HID descriptor.
 *
 *  Copyright (c) 2011 Stefan Kriwanek <mail@stefankriwanek.de>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
		struct hid_usage *usage, __s32 value)
{
	/* No other conditions due to usage_table. */
	/* Fix "jumpy" cursor (invalid events sent by device). */
	if (value == 256)
		return 1;
	/* Drop useless distance 0 events (on button clicks etc.) as well */
	if (value == 0)
		return 1;