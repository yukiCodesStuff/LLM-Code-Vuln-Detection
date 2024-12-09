/*
 * sysctl.c: General linux system control interface
 *
 * Begun 24 March 1995, Stephen Tweedie
 * Added /proc support, Dec 1995
 * Added bdflush entry and intvec min/max checking, 2/23/96, Tom Dyas.
 * Added hooks for /proc/sys/net (minor, minor patch), 96/4/1, Mike Shaver.
 * Added kernel/java-{interpreter,appletviewer}, 96/5/10, Mike Shaver.
 * Dynamic registration fixes, Stephen Tweedie.
 * Added kswapd-interval, ctrl-alt-del, printk stuff, 1/8/97, Chris Horn.
 * Made sysctl support optional via CONFIG_SYSCTL, 1/10/97, Chris
 *  Horn.
 * Added proc_doulongvec_ms_jiffies_minmax, 09/08/99, Carlos H. Bauer.
 * Added proc_doulongvec_minmax, 09/08/99, Carlos H. Bauer.
 * Changed linked lists to use list.h instead of lists.h, 02/24/00, Bill
 *  Wendling.
 * The list_for_each() macro wasn't appropriate for the sysctl loop.
 *  Removed it and replaced it with older style, 03/23/00, Bill Wendling
 */

#include <linux/module.h>
#include <linux/aio.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/bitmap.h>
#include <linux/signal.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/security.h>
#include <linux/ctype.h>
#include <linux/kmemcheck.h>
#include <linux/kmemleak.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/net.h>
#include <linux/sysrq.h>
#include <linux/highuid.h>
#include <linux/writeback.h>
#include <linux/ratelimit.h>
#include <linux/compaction.h>
#include <linux/hugetlb.h>
#include <linux/initrd.h>
#include <linux/key.h>
#include <linux/times.h>
#include <linux/limits.h>
#include <linux/dcache.h>
#include <linux/dnotify.h>
#include <linux/syscalls.h>
#include <linux/vmstat.h>
#include <linux/nfs_fs.h>
#include <linux/acpi.h>
#include <linux/reboot.h>
#include <linux/ftrace.h>
#include <linux/perf_event.h>
#include <linux/kprobes.h>
#include <linux/pipe_fs_i.h>
#include <linux/oom.h>
#include <linux/kmod.h>
#include <linux/capability.h>
#include <linux/binfmts.h>
#include <linux/sched/sysctl.h>
#include <linux/kexec.h>
#include <linux/bpf.h>
#include <linux/mount.h>

#include <asm/uaccess.h>
#include <asm/processor.h>

#ifdef CONFIG_X86
#include <asm/nmi.h>
#include <asm/stacktrace.h>
#include <asm/io.h>
#endif
#ifdef CONFIG_SPARC
#include <asm/setup.h>
#endif
#ifdef CONFIG_BSD_PROCESS_ACCT
#include <linux/acct.h>
#endif
#ifdef CONFIG_RT_MUTEXES
#include <linux/rtmutex.h>
#endif
#if defined(CONFIG_PROVE_LOCKING) || defined(CONFIG_LOCK_STAT)
#include <linux/lockdep.h>
#endif
#ifdef CONFIG_CHR_DEV_SG
#include <scsi/sg.h>
#endif

#ifdef CONFIG_LOCKUP_DETECTOR
#include <linux/nmi.h>
#endif

#if defined(CONFIG_SYSCTL)

/* External variables not in a header file. */
extern int suid_dumpable;
#ifdef CONFIG_COREDUMP
extern int core_uses_pid;
extern char core_pattern[];
extern unsigned int core_pipe_limit;
#endif
extern int pid_max;
extern int pid_max_min, pid_max_max;
extern int percpu_pagelist_fraction;
extern int compat_log;
extern int latencytop_enabled;
extern int sysctl_nr_open_min, sysctl_nr_open_max;
#ifndef CONFIG_MMU
extern int sysctl_nr_trim_pages;
#endif

/* Constants used for minimum and  maximum */
#ifdef CONFIG_LOCKUP_DETECTOR
static int sixty = 60;
#endif

static int __maybe_unused neg_one = -1;

static int zero;
static int __maybe_unused one = 1;
static int __maybe_unused two = 2;
static int __maybe_unused four = 4;
static unsigned long one_ul = 1;
static int one_hundred = 100;
static int one_thousand = 1000;
#ifdef CONFIG_PRINTK
static int ten_thousand = 10000;
#endif
#ifdef CONFIG_PERF_EVENTS
static int six_hundred_forty_kb = 640 * 1024;
#endif

/* this is needed for the proc_doulongvec_minmax of vm_dirty_bytes */
static unsigned long dirty_bytes_min = 2 * PAGE_SIZE;

/* this is needed for the proc_dointvec_minmax for [fs_]overflow UID and GID */
static int maxolduid = 65535;
static int minolduid;

static int ngroups_max = NGROUPS_MAX;
static const int cap_last_cap = CAP_LAST_CAP;

/*this is needed for proc_doulongvec_minmax of sysctl_hung_task_timeout_secs */
#ifdef CONFIG_DETECT_HUNG_TASK
static unsigned long hung_task_timeout_max = (LONG_MAX/HZ);
#endif

#ifdef CONFIG_INOTIFY_USER
#include <linux/inotify.h>
#endif
#ifdef CONFIG_SPARC
#endif

#ifdef __hppa__
extern int pwrsw_enabled;
#endif

#ifdef CONFIG_SYSCTL_ARCH_UNALIGN_ALLOW
extern int unaligned_enabled;
#endif

#ifdef CONFIG_IA64
extern int unaligned_dump_stack;
#endif

#ifdef CONFIG_SYSCTL_ARCH_UNALIGN_NO_WARN
extern int no_unaligned_warning;
#endif

#ifdef CONFIG_PROC_SYSCTL

#define SYSCTL_WRITES_LEGACY	-1
#define SYSCTL_WRITES_WARN	 0
#define SYSCTL_WRITES_STRICT	 1

static int sysctl_writes_strict = SYSCTL_WRITES_STRICT;

static int proc_do_cad_pid(struct ctl_table *table, int write,
		  void __user *buffer, size_t *lenp, loff_t *ppos);
static int proc_taint(struct ctl_table *table, int write,
			       void __user *buffer, size_t *lenp, loff_t *ppos);
#endif

#ifdef CONFIG_PRINTK
static int proc_dointvec_minmax_sysadmin(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp, loff_t *ppos);
#endif

static int proc_dointvec_minmax_coredump(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos);
#ifdef CONFIG_COREDUMP
static int proc_dostring_coredump(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos);
#endif

#ifdef CONFIG_MAGIC_SYSRQ
/* Note: sysrq code uses it's own private copy */
static int __sysrq_enabled = CONFIG_MAGIC_SYSRQ_DEFAULT_ENABLE;

static int sysrq_sysctl_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int error;

	error = proc_dointvec(table, write, buffer, lenp, ppos);
	if (error)
		return error;

	if (write)
		sysrq_toggle_support(__sysrq_enabled);

	return 0;
}
	{
		.procname	= "pipe-user-pages-soft",
		.data		= &pipe_user_pages_soft,
		.maxlen		= sizeof(pipe_user_pages_soft),
		.mode		= 0644,
		.proc_handler	= proc_doulongvec_minmax,
	},
	{
		.procname	= "mount-max",
		.data		= &sysctl_mount_max,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one,
	},
	{ }