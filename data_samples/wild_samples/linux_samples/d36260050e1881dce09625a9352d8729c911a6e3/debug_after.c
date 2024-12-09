/*
 * Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2010-2016, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include "debug.h"

#ifndef __INLINE_DEBUG__
#include "debug_private.h"
#endif /* __INLINE_DEBUG__ */

#include "memory_access.h"

#define __INLINE_SP__
#include "sp.h"

#include "assert_support.h"

/* The address of the remote copy */
hrt_address	debug_buffer_address = (hrt_address)-1;
hrt_vaddress	debug_buffer_ddr_address = (hrt_vaddress)-1;
/* The local copy */
static debug_data_t		debug_data;
debug_data_t		*debug_data_ptr = &debug_data;

void debug_buffer_init(const hrt_address addr)
{
	debug_buffer_address = addr;

	debug_data.head = 0;
	debug_data.tail = 0;
}