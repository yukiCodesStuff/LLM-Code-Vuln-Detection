/*
 * Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2010-2015, Intel Corporation.
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

#include <type_support.h> /*uint32_t */
#include "gp_timer.h"   /*system_local.h,
			  gp_timer_public.h*/

#ifndef __INLINE_GP_TIMER__
#include "gp_timer_private.h"  /*device_access.h*/
#endif /* __INLINE_GP_TIMER__ */
#include "system_local.h"

/* FIXME: not sure if reg_load(), reg_store() should be API.
 */
static uint32_t
gp_timer_reg_load(uint32_t reg);

static void
gp_timer_reg_store(uint32_t reg, uint32_t value);

static uint32_t
gp_timer_reg_load(uint32_t reg)
{
	return ia_css_device_load_uint32(
			GP_TIMER_BASE +
			(reg * sizeof(uint32_t)));
}