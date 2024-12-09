/*
 * Copyright 2010 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

/* Adjust unistd.h to provide 32-bit numbers and functions. */
#define __SYSCALL_COMPAT

#include <linux/compat.h>
#include <linux/syscalls.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/signal.h>
#include <asm/syscalls.h>

/*
 * Syscalls that take 64-bit numbers traditionally take them in 32-bit
 * "high" and "low" value parts on 32-bit architectures.
 * In principle, one could imagine passing some register arguments as
 * fully 64-bit on TILE-Gx in 32-bit mode, but it seems easier to
 * adapt the usual convention.
 */

long compat_sys_truncate64(char __user *filename, u32 dummy, u32 low, u32 high)
{
	return sys_truncate(filename, ((loff_t)high << 32) | low);
}