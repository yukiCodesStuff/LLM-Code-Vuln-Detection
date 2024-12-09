/*
 * Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2015, Intel Corporation.
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

#include "ia_css_debug.h"
#include "ia_css_tdf.host.h"

const int16_t g_pyramid[8][8] = {
{128, 384, 640, 896, 896, 640, 384, 128},
{384, 1152, 1920, 2688, 2688, 1920, 1152, 384},
{640, 1920, 3200, 4480, 4480, 3200, 1920, 640},
{896, 2688, 4480, 6272, 6272, 4480, 2688, 896},
{896, 2688, 4480, 6272, 6272, 4480, 2688, 896},
{640, 1920, 3200, 4480, 4480, 3200, 1920, 640},
{384, 1152, 1920, 2688, 2688, 1920, 1152, 384},
{128, 384, 640, 896, 896, 640, 384, 128}