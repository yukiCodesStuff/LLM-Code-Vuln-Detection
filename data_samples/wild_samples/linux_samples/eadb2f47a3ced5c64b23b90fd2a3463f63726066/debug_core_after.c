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
#include <linux/security.h>

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
			/*
			 * This is a brutal way to interfere with the debugger
			 * and prevent gdb being used to poke at kernel memory.
			 * This could cause trouble if lockdown is applied when
			 * there is already an active gdb session. For now the
			 * answer is simply "don't do that". Typically lockdown
			 * *will* be applied before the debug core gets started
			 * so only developers using kgdb for fairly advanced
			 * early kernel debug can be biten by this. Hopefully
			 * they are sophisticated enough to take care of
			 * themselves, especially with help from the lockdown
			 * message printed on the console!
			 */
			if (security_locked_down(LOCKDOWN_DBG_WRITE_KERNEL)) {
				if (IS_ENABLED(CONFIG_KGDB_KDB)) {
					/* Switch back to kdb if possible... */
					dbg_kdb_mode = 1;
					continue;
				} else {
					/* ... otherwise just bail */
					break;
				}
			}
			error = gdb_serial_stub(ks);
		}

		if (error == DBG_PASS_EVENT) {
			dbg_kdb_mode = !dbg_kdb_mode;
		} else if (error == DBG_SWITCH_CPU_EVENT) {
			kgdb_info[dbg_switch_cpu].exception_state |=
				DCPU_NEXT_MASTER;
			goto cpu_loop;
		} else {
			kgdb_info[cpu].ret_state = error;
			break;
		}
	}

	dbg_activate_sw_breakpoints();

	/* Call the I/O driver's post_exception routine */
	if (dbg_io_ops->post_exception)
		dbg_io_ops->post_exception();

	atomic_dec(&ignore_console_lock_warning);

	if (!kgdb_single_step) {
		raw_spin_unlock(&dbg_slave_lock);
		/* Wait till all the CPUs have quit from the debugger. */
		while (kgdb_do_roundup && atomic_read(&slaves_in_kgdb))
			cpu_relax();
	}

kgdb_restore:
	if (atomic_read(&kgdb_cpu_doing_single_step) != -1) {
		int sstep_cpu = atomic_read(&kgdb_cpu_doing_single_step);
		if (kgdb_info[sstep_cpu].task)
			kgdb_sstep_pid = kgdb_info[sstep_cpu].task->pid;
		else
			kgdb_sstep_pid = 0;
	}
	if (arch_kgdb_ops.correct_hw_break)
		arch_kgdb_ops.correct_hw_break();
	if (trace_on)
		tracing_on();

	kgdb_info[cpu].debuggerinfo = NULL;
	kgdb_info[cpu].task = NULL;
	kgdb_info[cpu].exception_state &=
		~(DCPU_WANT_MASTER | DCPU_IS_SLAVE);
	kgdb_info[cpu].enter_kgdb--;
	smp_mb__before_atomic();
	atomic_dec(&masters_in_kgdb);
	/* Free kgdb_active */
	atomic_set(&kgdb_active, -1);
	raw_spin_unlock(&dbg_master_lock);
	dbg_touch_watchdogs();
	local_irq_restore(flags);
	rcu_read_unlock();

	return kgdb_info[cpu].ret_state;
}
NOKPROBE_SYMBOL(kgdb_cpu_enter);

/*
 * kgdb_handle_exception() - main entry point from a kernel exception
 *
 * Locking hierarchy:
 *	interface locks, if any (begin_session)
 *	kgdb lock (kgdb_active)
 */
int
kgdb_handle_exception(int evector, int signo, int ecode, struct pt_regs *regs)
{
	struct kgdb_state kgdb_var;
	struct kgdb_state *ks = &kgdb_var;
	int ret = 0;

	if (arch_kgdb_ops.enable_nmi)
		arch_kgdb_ops.enable_nmi(0);
	/*
	 * Avoid entering the debugger if we were triggered due to an oops
	 * but panic_timeout indicates the system should automatically
	 * reboot on panic. We don't want to get stuck waiting for input
	 * on such systems, especially if its "just" an oops.
	 */
	if (signo != SIGTRAP && panic_timeout)
		return 1;

	memset(ks, 0, sizeof(struct kgdb_state));
	ks->cpu			= raw_smp_processor_id();
	ks->ex_vector		= evector;
	ks->signo		= signo;
	ks->err_code		= ecode;
	ks->linux_regs		= regs;

	if (kgdb_reenter_check(ks))
		goto out; /* Ouch, double exception ! */
	if (kgdb_info[ks->cpu].enter_kgdb != 0)
		goto out;

	ret = kgdb_cpu_enter(ks, regs, DCPU_WANT_MASTER);
out:
	if (arch_kgdb_ops.enable_nmi)
		arch_kgdb_ops.enable_nmi(1);
	return ret;
}
NOKPROBE_SYMBOL(kgdb_handle_exception);

/*
 * GDB places a breakpoint at this function to know dynamically loaded objects.
 */
static int module_event(struct notifier_block *self, unsigned long val,
	void *data)
{
	return 0;
}

static struct notifier_block dbg_module_load_nb = {
	.notifier_call	= module_event,
};

int kgdb_nmicallback(int cpu, void *regs)
{
#ifdef CONFIG_SMP
	struct kgdb_state kgdb_var;
	struct kgdb_state *ks = &kgdb_var;

	kgdb_info[cpu].rounding_up = false;

	memset(ks, 0, sizeof(struct kgdb_state));
	ks->cpu			= cpu;
	ks->linux_regs		= regs;

	if (kgdb_info[ks->cpu].enter_kgdb == 0 &&
			raw_spin_is_locked(&dbg_master_lock)) {
		kgdb_cpu_enter(ks, regs, DCPU_IS_SLAVE);
		return 0;
	}
#endif
	return 1;
}
NOKPROBE_SYMBOL(kgdb_nmicallback);

int kgdb_nmicallin(int cpu, int trapnr, void *regs, int err_code,
							atomic_t *send_ready)
{
#ifdef CONFIG_SMP
	if (!kgdb_io_ready(0) || !send_ready)
		return 1;

	if (kgdb_info[cpu].enter_kgdb == 0) {
		struct kgdb_state kgdb_var;
		struct kgdb_state *ks = &kgdb_var;

		memset(ks, 0, sizeof(struct kgdb_state));
		ks->cpu			= cpu;
		ks->ex_vector		= trapnr;
		ks->signo		= SIGTRAP;
		ks->err_code		= err_code;
		ks->linux_regs		= regs;
		ks->send_ready		= send_ready;
		kgdb_cpu_enter(ks, regs, DCPU_WANT_MASTER);
		return 0;
	}
#endif
	return 1;
}
NOKPROBE_SYMBOL(kgdb_nmicallin);

static void kgdb_console_write(struct console *co, const char *s,
   unsigned count)
{
	unsigned long flags;

	/* If we're debugging, or KGDB has not connected, don't try
	 * and print. */
	if (!kgdb_connected || atomic_read(&kgdb_active) != -1 || dbg_kdb_mode)
		return;

	local_irq_save(flags);
	gdbstub_msg_write(s, count);
	local_irq_restore(flags);
}

static struct console kgdbcons = {
	.name		= "kgdb",
	.write		= kgdb_console_write,
	.flags		= CON_PRINTBUFFER | CON_ENABLED,
	.index		= -1,
};

static int __init opt_kgdb_con(char *str)
{
	kgdb_use_con = 1;

	if (kgdb_io_module_registered && !kgdb_con_registered) {
		register_console(&kgdbcons);
		kgdb_con_registered = 1;
	}

	return 0;
}

early_param("kgdbcon", opt_kgdb_con);

#ifdef CONFIG_MAGIC_SYSRQ
static void sysrq_handle_dbg(int key)
{
	if (!dbg_io_ops) {
		pr_crit("ERROR: No KGDB I/O module available\n");
		return;
	}
	if (!kgdb_connected) {
#ifdef CONFIG_KGDB_KDB
		if (!dbg_kdb_mode)
			pr_crit("KGDB or $3#33 for KDB\n");
#else
		pr_crit("Entering KGDB\n");
#endif
	}

	kgdb_breakpoint();
}

static const struct sysrq_key_op sysrq_dbg_op = {
	.handler	= sysrq_handle_dbg,
	.help_msg	= "debug(g)",
	.action_msg	= "DEBUG",
};
#endif

void kgdb_panic(const char *msg)
{
	if (!kgdb_io_module_registered)
		return;

	/*
	 * We don't want to get stuck waiting for input from user if
	 * "panic_timeout" indicates the system should automatically
	 * reboot on panic.
	 */
	if (panic_timeout)
		return;

	if (dbg_kdb_mode)
		kdb_printf("PANIC: %s\n", msg);

	kgdb_breakpoint();
}

static void kgdb_initial_breakpoint(void)
{
	kgdb_break_asap = 0;

	pr_crit("Waiting for connection from remote gdb...\n");
	kgdb_breakpoint();
}

void __weak kgdb_arch_late(void)
{
}

void __init dbg_late_init(void)
{
	dbg_is_early = false;
	if (kgdb_io_module_registered)
		kgdb_arch_late();
	kdb_init(KDB_INIT_FULL);

	if (kgdb_io_module_registered && kgdb_break_asap)
		kgdb_initial_breakpoint();
}

static int
dbg_notify_reboot(struct notifier_block *this, unsigned long code, void *x)
{
	/*
	 * Take the following action on reboot notify depending on value:
	 *    1 == Enter debugger
	 *    0 == [the default] detach debug client
	 *   -1 == Do nothing... and use this until the board resets
	 */
	switch (kgdbreboot) {
	case 1:
		kgdb_breakpoint();
		goto done;
	case -1:
		goto done;
	}
	if (!dbg_kdb_mode)
		gdbstub_exit(code);
done:
	return NOTIFY_DONE;
}

static struct notifier_block dbg_reboot_notifier = {
	.notifier_call		= dbg_notify_reboot,
	.next			= NULL,
	.priority		= INT_MAX,
};

static void kgdb_register_callbacks(void)
{
	if (!kgdb_io_module_registered) {
		kgdb_io_module_registered = 1;
		kgdb_arch_init();
		if (!dbg_is_early)
			kgdb_arch_late();
		register_module_notifier(&dbg_module_load_nb);
		register_reboot_notifier(&dbg_reboot_notifier);
#ifdef CONFIG_MAGIC_SYSRQ
		register_sysrq_key('g', &sysrq_dbg_op);
#endif
		if (kgdb_use_con && !kgdb_con_registered) {
			register_console(&kgdbcons);
			kgdb_con_registered = 1;
		}
	}
}

static void kgdb_unregister_callbacks(void)
{
	/*
	 * When this routine is called KGDB should unregister from
	 * handlers and clean up, making sure it is not handling any
	 * break exceptions at the time.
	 */
	if (kgdb_io_module_registered) {
		kgdb_io_module_registered = 0;
		unregister_reboot_notifier(&dbg_reboot_notifier);
		unregister_module_notifier(&dbg_module_load_nb);
		kgdb_arch_exit();
#ifdef CONFIG_MAGIC_SYSRQ
		unregister_sysrq_key('g', &sysrq_dbg_op);
#endif
		if (kgdb_con_registered) {
			unregister_console(&kgdbcons);
			kgdb_con_registered = 0;
		}
	}
}

/**
 *	kgdb_register_io_module - register KGDB IO module
 *	@new_dbg_io_ops: the io ops vector
 *
 *	Register it with the KGDB core.
 */
int kgdb_register_io_module(struct kgdb_io *new_dbg_io_ops)
{
	struct kgdb_io *old_dbg_io_ops;
	int err;

	spin_lock(&kgdb_registration_lock);

	old_dbg_io_ops = dbg_io_ops;
	if (old_dbg_io_ops) {
		if (!old_dbg_io_ops->deinit) {
			spin_unlock(&kgdb_registration_lock);

			pr_err("KGDB I/O driver %s can't replace %s.\n",
				new_dbg_io_ops->name, old_dbg_io_ops->name);
			return -EBUSY;
		}
		pr_info("Replacing I/O driver %s with %s\n",
			old_dbg_io_ops->name, new_dbg_io_ops->name);
	}

	if (new_dbg_io_ops->init) {
		err = new_dbg_io_ops->init();
		if (err) {
			spin_unlock(&kgdb_registration_lock);
			return err;
		}
	}

	dbg_io_ops = new_dbg_io_ops;

	spin_unlock(&kgdb_registration_lock);

	if (old_dbg_io_ops) {
		old_dbg_io_ops->deinit();
		return 0;
	}

	pr_info("Registered I/O driver %s\n", new_dbg_io_ops->name);

	/* Arm KGDB now. */
	kgdb_register_callbacks();

	if (kgdb_break_asap &&
	    (!dbg_is_early || IS_ENABLED(CONFIG_ARCH_HAS_EARLY_DEBUG)))
		kgdb_initial_breakpoint();

	return 0;
}
EXPORT_SYMBOL_GPL(kgdb_register_io_module);

/**
 *	kgdb_unregister_io_module - unregister KGDB IO module
 *	@old_dbg_io_ops: the io ops vector
 *
 *	Unregister it with the KGDB core.
 */
void kgdb_unregister_io_module(struct kgdb_io *old_dbg_io_ops)
{
	BUG_ON(kgdb_connected);

	/*
	 * KGDB is no longer able to communicate out, so
	 * unregister our callbacks and reset state.
	 */
	kgdb_unregister_callbacks();

	spin_lock(&kgdb_registration_lock);

	WARN_ON_ONCE(dbg_io_ops != old_dbg_io_ops);
	dbg_io_ops = NULL;

	spin_unlock(&kgdb_registration_lock);

	if (old_dbg_io_ops->deinit)
		old_dbg_io_ops->deinit();

	pr_info("Unregistered I/O driver %s, debugger disabled\n",
		old_dbg_io_ops->name);
}
EXPORT_SYMBOL_GPL(kgdb_unregister_io_module);

int dbg_io_get_char(void)
{
	int ret = dbg_io_ops->read_char();
	if (ret == NO_POLL_CHAR)
		return -1;
	if (!dbg_kdb_mode)
		return ret;
	if (ret == 127)
		return 8;
	return ret;
}

/**
 * kgdb_breakpoint - generate breakpoint exception
 *
 * This function will generate a breakpoint exception.  It is used at the
 * beginning of a program to sync up with a debugger and can be used
 * otherwise as a quick means to stop program execution and "break" into
 * the debugger.
 */
noinline void kgdb_breakpoint(void)
{
	atomic_inc(&kgdb_setting_breakpoint);
	wmb(); /* Sync point before breakpoint */
	arch_kgdb_breakpoint();
	wmb(); /* Sync point after breakpoint */
	atomic_dec(&kgdb_setting_breakpoint);
}
EXPORT_SYMBOL_GPL(kgdb_breakpoint);

static int __init opt_kgdb_wait(char *str)
{
	kgdb_break_asap = 1;

	kdb_init(KDB_INIT_EARLY);
	if (kgdb_io_module_registered &&
	    IS_ENABLED(CONFIG_ARCH_HAS_EARLY_DEBUG))
		kgdb_initial_breakpoint();

	return 0;
}

early_param("kgdbwait", opt_kgdb_wait);