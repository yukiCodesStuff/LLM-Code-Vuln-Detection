// SPDX-License-Identifier: GPL-2.0-only
/*
 * Kernel Debug Core
 *
 * Maintainer: Jason Wessel <jason.wessel@windriver.com>
 *
 * Copyright (C) 2000-2001 VERITAS Software Corporation.
 * Copyright (C) 2002-2004 Timesys Corporation
 * Copyright (C) 2003-2004 Amit S. Kale <amitkale@linsyssoft.com>
 * Copyright (C) 2004 Pavel Machek <pavel@ucw.cz>
 * Copyright (C) 2004-2006 Tom Rini <trini@kernel.crashing.org>
 * Copyright (C) 2004-2006 LinSysSoft Technologies Pvt. Ltd.
 * Copyright (C) 2005-2009 Wind River Systems, Inc.
 * Copyright (C) 2007 MontaVista Software, Inc.
 * Copyright (C) 2008 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *
 * Contributors at various stages not listed above:
 *  Jason Wessel ( jason.wessel@windriver.com )
 *  George Anzinger <george@mvista.com>
 *  Anurekh Saxena (anurekh.saxena@timesys.com)
 *  Lake Stevens Instrument Division (Glenn Engel)
 *  Jim Kingdon, Cygnus Support.
 *
 * Original KGDB stub: David Grothe <dave@gcom.com>,
 * Tigran Aivazian <tigran@sco.com>
 */

#define pr_fmt(fmt) "KGDB: " fmt

#include <linux/pid_namespace.h>
#include <linux/clocksource.h>
#include <linux/serial_core.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/console.h>
#include <linux/threads.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sysrq.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/kgdb.h>
#include <linux/kdb.h>
#include <linux/nmi.h>
#include <linux/pid.h>
#include <linux/smp.h>
#include <linux/mm.h>
#include <linux/vmacache.h>
#include <linux/rcupdate.h>
#include <linux/irq.h>

#include <asm/cacheflush.h>
#include <asm/byteorder.h>
#include <linux/atomic.h>

#include "debug_core.h"

static int kgdb_break_asap;

struct debuggerinfo_struct kgdb_info[NR_CPUS];

/* kgdb_connected - Is a host GDB connected to us? */
int				kgdb_connected;
EXPORT_SYMBOL_GPL(kgdb_connected);

/* All the KGDB handlers are installed */
int			kgdb_io_module_registered;

/* Guard for recursive entry */
static int			exception_level;

struct kgdb_io		*dbg_io_ops;
static DEFINE_SPINLOCK(kgdb_registration_lock);

/* Action for the reboot notifier, a global allow kdb to change it */
static int kgdbreboot;
/* kgdb console driver is loaded */
static int kgdb_con_registered;
/* determine if kgdb console output should be used */
static int kgdb_use_con;
/* Flag for alternate operations for early debugging */
bool dbg_is_early = true;
/* Next cpu to become the master debug core */
int dbg_switch_cpu;

/* Use kdb or gdbserver mode */
int dbg_kdb_mode = 1;

module_param(kgdb_use_con, int, 0644);
module_param(kgdbreboot, int, 0644);

/*
 * Holds information about breakpoints in a kernel. These breakpoints are
 * added and removed by gdb.
 */
static struct kgdb_bkpt		kgdb_break[KGDB_MAX_BREAKPOINTS] = {
	[0 ... KGDB_MAX_BREAKPOINTS-1] = { .state = BP_UNDEFINED }
		if (dbg_kdb_mode) {
			kgdb_connected = 1;
			error = kdb_stub(ks);
			if (error == -1)
				continue;
			kgdb_connected = 0;
		} else {
			error = gdb_serial_stub(ks);
		}