/*
 * Kernel Debugger Architecture Independent Main Code
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999-2004 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (C) 2000 Stephane Eranian <eranian@hpl.hp.com>
 * Xscale (R) modifications copyright (C) 2003 Intel Corporation.
 * Copyright (c) 2009 Wind River Systems, Inc.  All Rights Reserved.
 */

#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/kmsg_dump.h>
#include <linux/reboot.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/sched/stat.h>
#include <linux/sched/debug.h>
#include <linux/sysrq.h>
#include <linux/smp.h>
#include <linux/utsname.h>
#include <linux/vmalloc.h>
#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kgdb.h>
#include <linux/kdb.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/nmi.h>
#include <linux/time.h>
#include <linux/ptrace.h>
#include <linux/sysctl.h>
#include <linux/cpu.h>
#include <linux/kdebug.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/security.h>
#include "kdb_private.h"

#undef	MODULE_PARAM_PREFIX
#define	MODULE_PARAM_PREFIX "kdb."

static int kdb_cmd_enabled = CONFIG_KDB_DEFAULT_ENABLE;
module_param_named(cmd_enable, kdb_cmd_enabled, int, 0600);

char kdb_grep_string[KDB_GREP_STRLEN];
int kdb_grepping_flag;
EXPORT_SYMBOL(kdb_grepping_flag);
int kdb_grep_leading;
int kdb_grep_trailing;

/*
 * Kernel debugger state flags
 */
unsigned int kdb_flags;

/*
 * kdb_lock protects updates to kdb_initial_cpu.  Used to
 * single thread processors through the kernel debugger.
 */
int kdb_initial_cpu = -1;	/* cpu number that owns kdb */
int kdb_nextline = 1;
int kdb_state;			/* General KDB state */

struct task_struct *kdb_current_task;
struct pt_regs *kdb_current_regs;

const char *kdb_diemsg;
static int kdb_go_count;
#ifdef CONFIG_KDB_CONTINUE_CATASTROPHIC
static unsigned int kdb_continue_catastrophic =
	CONFIG_KDB_CONTINUE_CATASTROPHIC;
#else
static unsigned int kdb_continue_catastrophic;
#endif

/* kdb_cmds_head describes the available commands. */
static LIST_HEAD(kdb_cmds_head);

typedef struct _kdbmsg {
	int	km_diag;	/* kdb diagnostic */
	char	*km_msg;	/* Corresponding message text */
} kdbmsg_t;

#define KDBMSG(msgnum, text) \
	{ KDB_##msgnum, text }
{
	struct task_struct *p = curr_task(cpu);
#ifdef	_TIF_MCA_INIT
	if ((task_thread_info(p)->flags & _TIF_MCA_INIT) && KDB_TSK(cpu))
		p = krp->p;
#endif
	return p;
}

/*
 * Update the permissions flags (kdb_cmd_enabled) to match the
 * current lockdown state.
 *
 * Within this function the calls to security_locked_down() are "lazy". We
 * avoid calling them if the current value of kdb_cmd_enabled already excludes
 * flags that might be subject to lockdown. Additionally we deliberately check
 * the lockdown flags independently (even though read lockdown implies write
 * lockdown) since that results in both simpler code and clearer messages to
 * the user on first-time debugger entry.
 *
 * The permission masks during a read+write lockdown permits the following
 * flags: INSPECT, SIGNAL, REBOOT (and ALWAYS_SAFE).
 *
 * The INSPECT commands are not blocked during lockdown because they are
 * not arbitrary memory reads. INSPECT covers the backtrace family (sometimes
 * forcing them to have no arguments) and lsmod. These commands do expose
 * some kernel state but do not allow the developer seated at the console to
 * choose what state is reported. SIGNAL and REBOOT should not be controversial,
 * given these are allowed for root during lockdown already.
 */
static void kdb_check_for_lockdown(void)
{
	const int write_flags = KDB_ENABLE_MEM_WRITE |
				KDB_ENABLE_REG_WRITE |
				KDB_ENABLE_FLOW_CTRL;
	const int read_flags = KDB_ENABLE_MEM_READ |
			       KDB_ENABLE_REG_READ;

	bool need_to_lockdown_write = false;
	bool need_to_lockdown_read = false;

	if (kdb_cmd_enabled & (KDB_ENABLE_ALL | write_flags))
		need_to_lockdown_write =
			security_locked_down(LOCKDOWN_DBG_WRITE_KERNEL);

	if (kdb_cmd_enabled & (KDB_ENABLE_ALL | read_flags))
		need_to_lockdown_read =
			security_locked_down(LOCKDOWN_DBG_READ_KERNEL);

	/* De-compose KDB_ENABLE_ALL if required */
	if (need_to_lockdown_write || need_to_lockdown_read)
		if (kdb_cmd_enabled & KDB_ENABLE_ALL)
			kdb_cmd_enabled = KDB_ENABLE_MASK & ~KDB_ENABLE_ALL;

	if (need_to_lockdown_write)
		kdb_cmd_enabled &= ~write_flags;

	if (need_to_lockdown_read)
		kdb_cmd_enabled &= ~read_flags;
}
{
	char *cmdbuf;
	int diag;
	struct task_struct *kdb_current =
		kdb_curr_task(raw_smp_processor_id());

	KDB_DEBUG_STATE("kdb_local 1", reason);

	kdb_check_for_lockdown();

	kdb_go_count = 0;
	if (reason == KDB_REASON_DEBUG) {
		/* special case below */
	} else {
		kdb_printf("\nEntering kdb (current=0x%px, pid %d) ",
			   kdb_current, kdb_current ? kdb_current->pid : 0);
#if defined(CONFIG_SMP)
		kdb_printf("on processor %d ", raw_smp_processor_id());
#endif
	}

	switch (reason) {
	case KDB_REASON_DEBUG:
	{
		/*
		 * If re-entering kdb after a single step
		 * command, don't print the message.
		 */
		switch (db_result) {
		case KDB_DB_BPT:
			kdb_printf("\nEntering kdb (0x%px, pid %d) ",
				   kdb_current, kdb_current->pid);
#if defined(CONFIG_SMP)
			kdb_printf("on processor %d ", raw_smp_processor_id());
#endif
			kdb_printf("due to Debug @ " kdb_machreg_fmt "\n",
				   instruction_pointer(regs));
			break;
		case KDB_DB_SS:
			break;
		case KDB_DB_SSBPT:
			KDB_DEBUG_STATE("kdb_local 4", reason);
			return 1;	/* kdba_db_trap did the work */
		default:
			kdb_printf("kdb: Bad result from kdba_db_trap: %d\n",
				   db_result);
			break;
		}

	}
		break;
	case KDB_REASON_ENTER:
		if (KDB_STATE(KEYBOARD))
			kdb_printf("due to Keyboard Entry\n");
		else
			kdb_printf("due to KDB_ENTER()\n");
		break;
	case KDB_REASON_KEYBOARD:
		KDB_STATE_SET(KEYBOARD);
		kdb_printf("due to Keyboard Entry\n");
		break;
	case KDB_REASON_ENTER_SLAVE:
		/* drop through, slaves only get released via cpu switch */
	case KDB_REASON_SWITCH:
		kdb_printf("due to cpu switch\n");
		break;
	case KDB_REASON_OOPS:
		kdb_printf("Oops: %s\n", kdb_diemsg);
		kdb_printf("due to oops @ " kdb_machreg_fmt "\n",
			   instruction_pointer(regs));
		kdb_dumpregs(regs);
		break;
	case KDB_REASON_SYSTEM_NMI:
		kdb_printf("due to System NonMaskable Interrupt\n");
		break;
	case KDB_REASON_NMI:
		kdb_printf("due to NonMaskable Interrupt @ "
			   kdb_machreg_fmt "\n",
			   instruction_pointer(regs));
		break;
	case KDB_REASON_SSTEP:
	case KDB_REASON_BREAK:
		kdb_printf("due to %s @ " kdb_machreg_fmt "\n",
			   reason == KDB_REASON_BREAK ?
			   "Breakpoint" : "SS trap", instruction_pointer(regs));
		/*
		 * Determine if this breakpoint is one that we
		 * are interested in.
		 */
		if (db_result != KDB_DB_BPT) {
			kdb_printf("kdb: error return from kdba_bp_trap: %d\n",
				   db_result);
			KDB_DEBUG_STATE("kdb_local 6", reason);
			return 0;	/* Not for us, dismiss it */
		}
		break;
	case KDB_REASON_RECURSE:
		kdb_printf("due to Recursion @ " kdb_machreg_fmt "\n",
			   instruction_pointer(regs));
		break;
	default:
		kdb_printf("kdb: unexpected reason code: %d\n", reason);
		KDB_DEBUG_STATE("kdb_local 8", reason);
		return 0;	/* Not for us, dismiss it */
	}

	while (1) {
		/*
		 * Initialize pager context.
		 */
		kdb_nextline = 1;
		KDB_STATE_CLEAR(SUPPRESS);
		kdb_grepping_flag = 0;
		/* ensure the old search does not leak into '/' commands */
		kdb_grep_string[0] = '\0';

		cmdbuf = cmd_cur;
		*cmdbuf = '\0';
		*(cmd_hist[cmd_head]) = '\0';

do_full_getstr:
		/* PROMPT can only be set if we have MEM_READ permission. */
		snprintf(kdb_prompt_str, CMD_BUFLEN, kdbgetenv("PROMPT"),
			 raw_smp_processor_id());
		if (defcmd_in_progress)
			strncat(kdb_prompt_str, "[defcmd]", CMD_BUFLEN);

		/*
		 * Fetch command from keyboard
		 */
		cmdbuf = kdb_getstr(cmdbuf, CMD_BUFLEN, kdb_prompt_str);
		if (*cmdbuf != '\n') {
			if (*cmdbuf < 32) {
				if (cmdptr == cmd_head) {
					strscpy(cmd_hist[cmd_head], cmd_cur,
						CMD_BUFLEN);
					*(cmd_hist[cmd_head] +
					  strlen(cmd_hist[cmd_head])-1) = '\0';
				}
				if (!handle_ctrl_cmd(cmdbuf))
					*(cmd_cur+strlen(cmd_cur)-1) = '\0';
				cmdbuf = cmd_cur;
				goto do_full_getstr;
			} else {
				strscpy(cmd_hist[cmd_head], cmd_cur,
					CMD_BUFLEN);
			}

			cmd_head = (cmd_head+1) % KDB_CMD_HISTORY_COUNT;
			if (cmd_head == cmd_tail)
				cmd_tail = (cmd_tail+1) % KDB_CMD_HISTORY_COUNT;
		}

		cmdptr = cmd_head;
		diag = kdb_parse(cmdbuf);
		if (diag == KDB_NOTFOUND) {
			drop_newline(cmdbuf);
			kdb_printf("Unknown kdb command: '%s'\n", cmdbuf);
			diag = 0;
		}
		if (diag == KDB_CMD_GO
		 || diag == KDB_CMD_CPU
		 || diag == KDB_CMD_SS
		 || diag == KDB_CMD_KGDB)
			break;

		if (diag)
			kdb_cmderror(diag);
	}
	KDB_DEBUG_STATE("kdb_local 9", diag);
	return diag;
}


/*
 * kdb_print_state - Print the state data for the current processor
 *	for debugging.
 * Inputs:
 *	text		Identifies the debug point
 *	value		Any integer value to be printed, e.g. reason code.
 */
void kdb_print_state(const char *text, int value)
{
	kdb_printf("state: %s cpu %d value %d initial %d state %x\n",
		   text, raw_smp_processor_id(), value, kdb_initial_cpu,
		   kdb_state);
}

/*
 * kdb_main_loop - After initial setup and assignment of the
 *	controlling cpu, all cpus are in this loop.  One cpu is in
 *	control and will issue the kdb prompt, the others will spin
 *	until 'go' or cpu switch.
 *
 *	To get a consistent view of the kernel stacks for all
 *	processes, this routine is invoked from the main kdb code via
 *	an architecture specific routine.  kdba_main_loop is
 *	responsible for making the kernel stacks consistent for all
 *	processes, there should be no difference between a blocked
 *	process and a running process as far as kdb is concerned.
 * Inputs:
 *	reason		The reason KDB was invoked
 *	error		The hardware-defined error code
 *	reason2		kdb's current reason code.
 *			Initially error but can change
 *			according to kdb state.
 *	db_result	Result code from break or debug point.
 *	regs		The exception frame at time of fault/breakpoint.
 *			should always be valid.
 * Returns:
 *	0	KDB was invoked for an event which it wasn't responsible
 *	1	KDB handled the event for which it was invoked.
 */
int kdb_main_loop(kdb_reason_t reason, kdb_reason_t reason2, int error,
	      kdb_dbtrap_t db_result, struct pt_regs *regs)
{
	int result = 1;
	/* Stay in kdb() until 'go', 'ss[b]' or an error */
	while (1) {
		/*
		 * All processors except the one that is in control
		 * will spin here.
		 */
		KDB_DEBUG_STATE("kdb_main_loop 1", reason);
		while (KDB_STATE(HOLD_CPU)) {
			/* state KDB is turned off by kdb_cpu to see if the
			 * other cpus are still live, each cpu in this loop
			 * turns it back on.
			 */
			if (!KDB_STATE(KDB))
				KDB_STATE_SET(KDB);
		}

		KDB_STATE_CLEAR(SUPPRESS);
		KDB_DEBUG_STATE("kdb_main_loop 2", reason);
		if (KDB_STATE(LEAVING))
			break;	/* Another cpu said 'go' */
		/* Still using kdb, this processor is in control */
		result = kdb_local(reason2, error, regs, db_result);
		KDB_DEBUG_STATE("kdb_main_loop 3", result);

		if (result == KDB_CMD_CPU)
			break;

		if (result == KDB_CMD_SS) {
			KDB_STATE_SET(DOING_SS);
			break;
		}

		if (result == KDB_CMD_KGDB) {
			if (!KDB_STATE(DOING_KGDB))
				kdb_printf("Entering please attach debugger "
					   "or use $D#44+ or $3#33\n");
			break;
		}
		if (result && result != 1 && result != KDB_CMD_GO)
			kdb_printf("\nUnexpected kdb_local return code %d\n",
				   result);
		KDB_DEBUG_STATE("kdb_main_loop 4", reason);
		break;
	}
	if (KDB_STATE(DOING_SS))
		KDB_STATE_CLEAR(SSBPT);

	/* Clean up any keyboard devices before leaving */
	kdb_kbd_cleanup_state();

	return result;
}

/*
 * kdb_mdr - This function implements the guts of the 'mdr', memory
 * read command.
 *	mdr  <addr arg>,<byte count>
 * Inputs:
 *	addr	Start address
 *	count	Number of bytes
 * Returns:
 *	Always 0.  Any errors are detected and printed by kdb_getarea.
 */
static int kdb_mdr(unsigned long addr, unsigned int count)
{
	unsigned char c;
	while (count--) {
		if (kdb_getarea(c, addr))
			return 0;
		kdb_printf("%02x", c);
		addr++;
	}
	kdb_printf("\n");
	return 0;
}

/*
 * kdb_md - This function implements the 'md', 'md1', 'md2', 'md4',
 *	'md8' 'mdr' and 'mds' commands.
 *
 *	md|mds  [<addr arg> [<line count> [<radix>]]]
 *	mdWcN	[<addr arg> [<line count> [<radix>]]]
 *		where W = is the width (1, 2, 4 or 8) and N is the count.
 *		for eg., md1c20 reads 20 bytes, 1 at a time.
 *	mdr  <addr arg>,<byte count>
 */
static void kdb_md_line(const char *fmtstr, unsigned long addr,
			int symbolic, int nosect, int bytesperword,
			int num, int repeat, int phys)
{
	/* print just one line of data */
	kdb_symtab_t symtab;
	char cbuf[32];
	char *c = cbuf;
	int i;
	int j;
	unsigned long word;

	memset(cbuf, '\0', sizeof(cbuf));
	if (phys)
		kdb_printf("phys " kdb_machreg_fmt0 " ", addr);
	else
		kdb_printf(kdb_machreg_fmt0 " ", addr);

	for (i = 0; i < num && repeat--; i++) {
		if (phys) {
			if (kdb_getphysword(&word, addr, bytesperword))
				break;
		} else if (kdb_getword(&word, addr, bytesperword))
			break;
		kdb_printf(fmtstr, word);
		if (symbolic)
			kdbnearsym(word, &symtab);
		else
			memset(&symtab, 0, sizeof(symtab));
		if (symtab.sym_name) {
			kdb_symbol_print(word, &symtab, 0);
			if (!nosect) {
				kdb_printf("\n");
				kdb_printf("                       %s %s "
					   kdb_machreg_fmt " "
					   kdb_machreg_fmt " "
					   kdb_machreg_fmt, symtab.mod_name,
					   symtab.sec_name, symtab.sec_start,
					   symtab.sym_start, symtab.sym_end);
			}
			addr += bytesperword;
		} else {
			union {
				u64 word;
				unsigned char c[8];
			} wc;
			unsigned char *cp;
#ifdef	__BIG_ENDIAN
			cp = wc.c + 8 - bytesperword;
#else
			cp = wc.c;
#endif
			wc.word = word;
#define printable_char(c) \
	({unsigned char __c = c; isascii(__c) && isprint(__c) ? __c : '.'; })
			for (j = 0; j < bytesperword; j++)
				*c++ = printable_char(*cp++);
			addr += bytesperword;
#undef printable_char
		}
	}
	kdb_printf("%*s %s\n", (int)((num-i)*(2*bytesperword + 1)+1),
		   " ", cbuf);
}

static int kdb_md(int argc, const char **argv)
{
	static unsigned long last_addr;
	static int last_radix, last_bytesperword, last_repeat;
	int radix = 16, mdcount = 8, bytesperword = KDB_WORD_SIZE, repeat;
	int nosect = 0;
	char fmtchar, fmtstr[64];
	unsigned long addr;
	unsigned long word;
	long offset = 0;
	int symbolic = 0;
	int valid = 0;
	int phys = 0;
	int raw = 0;

	kdbgetintenv("MDCOUNT", &mdcount);
	kdbgetintenv("RADIX", &radix);
	kdbgetintenv("BYTESPERWORD", &bytesperword);

	/* Assume 'md <addr>' and start with environment values */
	repeat = mdcount * 16 / bytesperword;

	if (strcmp(argv[0], "mdr") == 0) {
		if (argc == 2 || (argc == 0 && last_addr != 0))
			valid = raw = 1;
		else
			return KDB_ARGCOUNT;
	} else if (isdigit(argv[0][2])) {
		bytesperword = (int)(argv[0][2] - '0');
		if (bytesperword == 0) {
			bytesperword = last_bytesperword;
			if (bytesperword == 0)
				bytesperword = 4;
		}
		last_bytesperword = bytesperword;
		repeat = mdcount * 16 / bytesperword;
		if (!argv[0][3])
			valid = 1;
		else if (argv[0][3] == 'c' && argv[0][4]) {
			char *p;
			repeat = simple_strtoul(argv[0] + 4, &p, 10);
			mdcount = ((repeat * bytesperword) + 15) / 16;
			valid = !*p;
		}
		last_repeat = repeat;
	} else if (strcmp(argv[0], "md") == 0)
		valid = 1;
	else if (strcmp(argv[0], "mds") == 0)
		valid = 1;
	else if (strcmp(argv[0], "mdp") == 0) {
		phys = valid = 1;
	}
	if (!valid)
		return KDB_NOTFOUND;

	if (argc == 0) {
		if (last_addr == 0)
			return KDB_ARGCOUNT;
		addr = last_addr;
		radix = last_radix;
		bytesperword = last_bytesperword;
		repeat = last_repeat;
		if (raw)
			mdcount = repeat;
		else
			mdcount = ((repeat * bytesperword) + 15) / 16;
	}

	if (argc) {
		unsigned long val;
		int diag, nextarg = 1;
		diag = kdbgetaddrarg(argc, argv, &nextarg, &addr,
				     &offset, NULL);
		if (diag)
			return diag;
		if (argc > nextarg+2)
			return KDB_ARGCOUNT;

		if (argc >= nextarg) {
			diag = kdbgetularg(argv[nextarg], &val);
			if (!diag) {
				mdcount = (int) val;
				if (raw)
					repeat = mdcount;
				else
					repeat = mdcount * 16 / bytesperword;
			}
		}
		if (argc >= nextarg+1) {
			diag = kdbgetularg(argv[nextarg+1], &val);
			if (!diag)
				radix = (int) val;
		}
	}

	if (strcmp(argv[0], "mdr") == 0) {
		int ret;
		last_addr = addr;
		ret = kdb_mdr(addr, mdcount);
		last_addr += mdcount;
		last_repeat = mdcount;
		last_bytesperword = bytesperword; // to make REPEAT happy
		return ret;
	}

	switch (radix) {
	case 10:
		fmtchar = 'd';
		break;
	case 16:
		fmtchar = 'x';
		break;
	case 8:
		fmtchar = 'o';
		break;
	default:
		return KDB_BADRADIX;
	}

	last_radix = radix;

	if (bytesperword > KDB_WORD_SIZE)
		return KDB_BADWIDTH;

	switch (bytesperword) {
	case 8:
		sprintf(fmtstr, "%%16.16l%c ", fmtchar);
		break;
	case 4:
		sprintf(fmtstr, "%%8.8l%c ", fmtchar);
		break;
	case 2:
		sprintf(fmtstr, "%%4.4l%c ", fmtchar);
		break;
	case 1:
		sprintf(fmtstr, "%%2.2l%c ", fmtchar);
		break;
	default:
		return KDB_BADWIDTH;
	}

	last_repeat = repeat;
	last_bytesperword = bytesperword;

	if (strcmp(argv[0], "mds") == 0) {
		symbolic = 1;
		/* Do not save these changes as last_*, they are temporary mds
		 * overrides.
		 */
		bytesperword = KDB_WORD_SIZE;
		repeat = mdcount;
		kdbgetintenv("NOSECT", &nosect);
	}

	/* Round address down modulo BYTESPERWORD */

	addr &= ~(bytesperword-1);

	while (repeat > 0) {
		unsigned long a;
		int n, z, num = (symbolic ? 1 : (16 / bytesperword));

		if (KDB_FLAG(CMD_INTERRUPT))
			return 0;
		for (a = addr, z = 0; z < repeat; a += bytesperword, ++z) {
			if (phys) {
				if (kdb_getphysword(&word, a, bytesperword)
						|| word)
					break;
			} else if (kdb_getword(&word, a, bytesperword) || word)
				break;
		}
		n = min(num, repeat);
		kdb_md_line(fmtstr, addr, symbolic, nosect, bytesperword,
			    num, repeat, phys);
		addr += bytesperword * n;
		repeat -= n;
		z = (z + num - 1) / num;
		if (z > 2) {
			int s = num * (z-2);
			kdb_printf(kdb_machreg_fmt0 "-" kdb_machreg_fmt0
				   " zero suppressed\n",
				addr, addr + bytesperword * s - 1);
			addr += bytesperword * s;
			repeat -= s;
		}
	}
	last_addr = addr;

	return 0;
}

/*
 * kdb_mm - This function implements the 'mm' command.
 *	mm address-expression new-value
 * Remarks:
 *	mm works on machine words, mmW works on bytes.
 */
static int kdb_mm(int argc, const char **argv)
{
	int diag;
	unsigned long addr;
	long offset = 0;
	unsigned long contents;
	int nextarg;
	int width;

	if (argv[0][2] && !isdigit(argv[0][2]))
		return KDB_NOTFOUND;

	if (argc < 2)
		return KDB_ARGCOUNT;

	nextarg = 1;
	diag = kdbgetaddrarg(argc, argv, &nextarg, &addr, &offset, NULL);
	if (diag)
		return diag;

	if (nextarg > argc)
		return KDB_ARGCOUNT;
	diag = kdbgetaddrarg(argc, argv, &nextarg, &contents, NULL, NULL);
	if (diag)
		return diag;

	if (nextarg != argc + 1)
		return KDB_ARGCOUNT;

	width = argv[0][2] ? (argv[0][2] - '0') : (KDB_WORD_SIZE);
	diag = kdb_putword(addr, contents, width);
	if (diag)
		return diag;

	kdb_printf(kdb_machreg_fmt " = " kdb_machreg_fmt "\n", addr, contents);

	return 0;
}

/*
 * kdb_go - This function implements the 'go' command.
 *	go [address-expression]
 */
static int kdb_go(int argc, const char **argv)
{
	unsigned long addr;
	int diag;
	int nextarg;
	long offset;

	if (raw_smp_processor_id() != kdb_initial_cpu) {
		kdb_printf("go must execute on the entry cpu, "
			   "please use \"cpu %d\" and then execute go\n",
			   kdb_initial_cpu);
		return KDB_BADCPUNUM;
	}
	if (argc == 1) {
		nextarg = 1;
		diag = kdbgetaddrarg(argc, argv, &nextarg,
				     &addr, &offset, NULL);
		if (diag)
			return diag;
	} else if (argc) {
		return KDB_ARGCOUNT;
	}

	diag = KDB_CMD_GO;
	if (KDB_FLAG(CATASTROPHIC)) {
		kdb_printf("Catastrophic error detected\n");
		kdb_printf("kdb_continue_catastrophic=%d, ",
			kdb_continue_catastrophic);
		if (kdb_continue_catastrophic == 0 && kdb_go_count++ == 0) {
			kdb_printf("type go a second time if you really want "
				   "to continue\n");
			return 0;
		}
		if (kdb_continue_catastrophic == 2) {
			kdb_printf("forcing reboot\n");
			kdb_reboot(0, NULL);
		}
		kdb_printf("attempting to continue\n");
	}
	return diag;
}

/*
 * kdb_rd - This function implements the 'rd' command.
 */
static int kdb_rd(int argc, const char **argv)
{
	int len = kdb_check_regs();
#if DBG_MAX_REG_NUM > 0
	int i;
	char *rname;
	int rsize;
	u64 reg64;
	u32 reg32;
	u16 reg16;
	u8 reg8;

	if (len)
		return len;

	for (i = 0; i < DBG_MAX_REG_NUM; i++) {
		rsize = dbg_reg_def[i].size * 2;
		if (rsize > 16)
			rsize = 2;
		if (len + strlen(dbg_reg_def[i].name) + 4 + rsize > 80) {
			len = 0;
			kdb_printf("\n");
		}
		if (len)
			len += kdb_printf("  ");
		switch(dbg_reg_def[i].size * 8) {
		case 8:
			rname = dbg_get_reg(i, &reg8, kdb_current_regs);
			if (!rname)
				break;
			len += kdb_printf("%s: %02x", rname, reg8);
			break;
		case 16:
			rname = dbg_get_reg(i, &reg16, kdb_current_regs);
			if (!rname)
				break;
			len += kdb_printf("%s: %04x", rname, reg16);
			break;
		case 32:
			rname = dbg_get_reg(i, &reg32, kdb_current_regs);
			if (!rname)
				break;
			len += kdb_printf("%s: %08x", rname, reg32);
			break;
		case 64:
			rname = dbg_get_reg(i, &reg64, kdb_current_regs);
			if (!rname)
				break;
			len += kdb_printf("%s: %016llx", rname, reg64);
			break;
		default:
			len += kdb_printf("%s: ??", dbg_reg_def[i].name);
		}
	}
	kdb_printf("\n");
#else
	if (len)
		return len;

	kdb_dumpregs(kdb_current_regs);
#endif
	return 0;
}

/*
 * kdb_rm - This function implements the 'rm' (register modify)  command.
 *	rm register-name new-contents
 * Remarks:
 *	Allows register modification with the same restrictions as gdb
 */
static int kdb_rm(int argc, const char **argv)
{
#if DBG_MAX_REG_NUM > 0
	int diag;
	const char *rname;
	int i;
	u64 reg64;
	u32 reg32;
	u16 reg16;
	u8 reg8;

	if (argc != 2)
		return KDB_ARGCOUNT;
	/*
	 * Allow presence or absence of leading '%' symbol.
	 */
	rname = argv[1];
	if (*rname == '%')
		rname++;

	diag = kdbgetu64arg(argv[2], &reg64);
	if (diag)
		return diag;

	diag = kdb_check_regs();
	if (diag)
		return diag;

	diag = KDB_BADREG;
	for (i = 0; i < DBG_MAX_REG_NUM; i++) {
		if (strcmp(rname, dbg_reg_def[i].name) == 0) {
			diag = 0;
			break;
		}
	}
	if (!diag) {
		switch(dbg_reg_def[i].size * 8) {
		case 8:
			reg8 = reg64;
			dbg_set_reg(i, &reg8, kdb_current_regs);
			break;
		case 16:
			reg16 = reg64;
			dbg_set_reg(i, &reg16, kdb_current_regs);
			break;
		case 32:
			reg32 = reg64;
			dbg_set_reg(i, &reg32, kdb_current_regs);
			break;
		case 64:
			dbg_set_reg(i, &reg64, kdb_current_regs);
			break;
		}
	}
	return diag;
#else
	kdb_printf("ERROR: Register set currently not implemented\n");
    return 0;
#endif
}

#if defined(CONFIG_MAGIC_SYSRQ)
/*
 * kdb_sr - This function implements the 'sr' (SYSRQ key) command
 *	which interfaces to the soi-disant MAGIC SYSRQ functionality.
 *		sr <magic-sysrq-code>
 */
static int kdb_sr(int argc, const char **argv)
{
	bool check_mask =
	    !kdb_check_flags(KDB_ENABLE_ALL, kdb_cmd_enabled, false);

	if (argc != 1)
		return KDB_ARGCOUNT;

	kdb_trap_printk++;
	__handle_sysrq(*argv[1], check_mask);
	kdb_trap_printk--;

	return 0;
}
#endif	/* CONFIG_MAGIC_SYSRQ */

/*
 * kdb_ef - This function implements the 'regs' (display exception
 *	frame) command.  This command takes an address and expects to
 *	find an exception frame at that address, formats and prints
 *	it.
 *		regs address-expression
 * Remarks:
 *	Not done yet.
 */
static int kdb_ef(int argc, const char **argv)
{
	int diag;
	unsigned long addr;
	long offset;
	int nextarg;

	if (argc != 1)
		return KDB_ARGCOUNT;

	nextarg = 1;
	diag = kdbgetaddrarg(argc, argv, &nextarg, &addr, &offset, NULL);
	if (diag)
		return diag;
	show_regs((struct pt_regs *)addr);
	return 0;
}

#if defined(CONFIG_MODULES)
/*
 * kdb_lsmod - This function implements the 'lsmod' command.  Lists
 *	currently loaded kernel modules.
 *	Mostly taken from userland lsmod.
 */
static int kdb_lsmod(int argc, const char **argv)
{
	struct module *mod;

	if (argc != 0)
		return KDB_ARGCOUNT;

	kdb_printf("Module                  Size  modstruct     Used by\n");
	list_for_each_entry(mod, kdb_modules, list) {
		if (mod->state == MODULE_STATE_UNFORMED)
			continue;

		kdb_printf("%-20s%8u  0x%px ", mod->name,
			   mod->core_layout.size, (void *)mod);
#ifdef CONFIG_MODULE_UNLOAD
		kdb_printf("%4d ", module_refcount(mod));
#endif
		if (mod->state == MODULE_STATE_GOING)
			kdb_printf(" (Unloading)");
		else if (mod->state == MODULE_STATE_COMING)
			kdb_printf(" (Loading)");
		else
			kdb_printf(" (Live)");
		kdb_printf(" 0x%px", mod->core_layout.base);

#ifdef CONFIG_MODULE_UNLOAD
		{
			struct module_use *use;
			kdb_printf(" [ ");
			list_for_each_entry(use, &mod->source_list,
					    source_list)
				kdb_printf("%s ", use->target->name);
			kdb_printf("]\n");
		}
#endif
	}

	return 0;
}

#endif	/* CONFIG_MODULES */

/*
 * kdb_env - This function implements the 'env' command.  Display the
 *	current environment variables.
 */

static int kdb_env(int argc, const char **argv)
{
	kdb_printenv();

	if (KDB_DEBUG(MASK))
		kdb_printf("KDBDEBUG=0x%x\n",
			(kdb_flags & KDB_DEBUG(MASK)) >> KDB_DEBUG_FLAG_SHIFT);

	return 0;
}

#ifdef CONFIG_PRINTK
/*
 * kdb_dmesg - This function implements the 'dmesg' command to display
 *	the contents of the syslog buffer.
 *		dmesg [lines] [adjust]
 */
static int kdb_dmesg(int argc, const char **argv)
{
	int diag;
	int logging;
	int lines = 0;
	int adjust = 0;
	int n = 0;
	int skip = 0;
	struct kmsg_dump_iter iter;
	size_t len;
	char buf[201];

	if (argc > 2)
		return KDB_ARGCOUNT;
	if (argc) {
		char *cp;
		lines = simple_strtol(argv[1], &cp, 0);
		if (*cp)
			lines = 0;
		if (argc > 1) {
			adjust = simple_strtoul(argv[2], &cp, 0);
			if (*cp || adjust < 0)
				adjust = 0;
		}
	}

	/* disable LOGGING if set */
	diag = kdbgetintenv("LOGGING", &logging);
	if (!diag && logging) {
		const char *setargs[] = { "set", "LOGGING", "0" };
		kdb_set(2, setargs);
	}

	kmsg_dump_rewind(&iter);
	while (kmsg_dump_get_line(&iter, 1, NULL, 0, NULL))
		n++;

	if (lines < 0) {
		if (adjust >= n)
			kdb_printf("buffer only contains %d lines, nothing "
				   "printed\n", n);
		else if (adjust - lines >= n)
			kdb_printf("buffer only contains %d lines, last %d "
				   "lines printed\n", n, n - adjust);
		skip = adjust;
		lines = abs(lines);
	} else if (lines > 0) {
		skip = n - lines - adjust;
		lines = abs(lines);
		if (adjust >= n) {
			kdb_printf("buffer only contains %d lines, "
				   "nothing printed\n", n);
			skip = n;
		} else if (skip < 0) {
			lines += skip;
			skip = 0;
			kdb_printf("buffer only contains %d lines, first "
				   "%d lines printed\n", n, lines);
		}
	} else {
		lines = n;
	}

	if (skip >= n || skip < 0)
		return 0;

	kmsg_dump_rewind(&iter);
	while (kmsg_dump_get_line(&iter, 1, buf, sizeof(buf), &len)) {
		if (skip) {
			skip--;
			continue;
		}
		if (!lines--)
			break;
		if (KDB_FLAG(CMD_INTERRUPT))
			return 0;

		kdb_printf("%.*s\n", (int)len - 1, buf);
	}

	return 0;
}
#endif /* CONFIG_PRINTK */

/* Make sure we balance enable/disable calls, must disable first. */
static atomic_t kdb_nmi_disabled;

static int kdb_disable_nmi(int argc, const char *argv[])
{
	if (atomic_read(&kdb_nmi_disabled))
		return 0;
	atomic_set(&kdb_nmi_disabled, 1);
	arch_kgdb_ops.enable_nmi(0);
	return 0;
}

static int kdb_param_enable_nmi(const char *val, const struct kernel_param *kp)
{
	if (!atomic_add_unless(&kdb_nmi_disabled, -1, 0))
		return -EINVAL;
	arch_kgdb_ops.enable_nmi(1);
	return 0;
}

static const struct kernel_param_ops kdb_param_ops_enable_nmi = {
	.set = kdb_param_enable_nmi,
};
module_param_cb(enable_nmi, &kdb_param_ops_enable_nmi, NULL, 0600);

/*
 * kdb_cpu - This function implements the 'cpu' command.
 *	cpu	[<cpunum>]
 * Returns:
 *	KDB_CMD_CPU for success, a kdb diagnostic if error
 */
static void kdb_cpu_status(void)
{
	int i, start_cpu, first_print = 1;
	char state, prev_state = '?';

	kdb_printf("Currently on cpu %d\n", raw_smp_processor_id());
	kdb_printf("Available cpus: ");
	for (start_cpu = -1, i = 0; i < NR_CPUS; i++) {
		if (!cpu_online(i)) {
			state = 'F';	/* cpu is offline */
		} else if (!kgdb_info[i].enter_kgdb) {
			state = 'D';	/* cpu is online but unresponsive */
		} else {
			state = ' ';	/* cpu is responding to kdb */
			if (kdb_task_state_char(KDB_TSK(i)) == '-')
				state = '-';	/* idle task */
		}
		if (state != prev_state) {
			if (prev_state != '?') {
				if (!first_print)
					kdb_printf(", ");
				first_print = 0;
				kdb_printf("%d", start_cpu);
				if (start_cpu < i-1)
					kdb_printf("-%d", i-1);
				if (prev_state != ' ')
					kdb_printf("(%c)", prev_state);
			}
			prev_state = state;
			start_cpu = i;
		}
	}
	/* print the trailing cpus, ignoring them if they are all offline */
	if (prev_state != 'F') {
		if (!first_print)
			kdb_printf(", ");
		kdb_printf("%d", start_cpu);
		if (start_cpu < i-1)
			kdb_printf("-%d", i-1);
		if (prev_state != ' ')
			kdb_printf("(%c)", prev_state);
	}
	kdb_printf("\n");
}

static int kdb_cpu(int argc, const char **argv)
{
	unsigned long cpunum;
	int diag;

	if (argc == 0) {
		kdb_cpu_status();
		return 0;
	}

	if (argc != 1)
		return KDB_ARGCOUNT;

	diag = kdbgetularg(argv[1], &cpunum);
	if (diag)
		return diag;

	/*
	 * Validate cpunum
	 */
	if ((cpunum >= CONFIG_NR_CPUS) || !kgdb_info[cpunum].enter_kgdb)
		return KDB_BADCPUNUM;

	dbg_switch_cpu = cpunum;

	/*
	 * Switch to other cpu
	 */
	return KDB_CMD_CPU;
}

/* The user may not realize that ps/bta with no parameters does not print idle
 * or sleeping system daemon processes, so tell them how many were suppressed.
 */
void kdb_ps_suppressed(void)
{
	int idle = 0, daemon = 0;
	unsigned long cpu;
	const struct task_struct *p, *g;
	for_each_online_cpu(cpu) {
		p = kdb_curr_task(cpu);
		if (kdb_task_state(p, "-"))
			++idle;
	}
	for_each_process_thread(g, p) {
		if (kdb_task_state(p, "ims"))
			++daemon;
	}
	if (idle || daemon) {
		if (idle)
			kdb_printf("%d idle process%s (state -)%s\n",
				   idle, idle == 1 ? "" : "es",
				   daemon ? " and " : "");
		if (daemon)
			kdb_printf("%d sleeping system daemon (state [ims]) "
				   "process%s", daemon,
				   daemon == 1 ? "" : "es");
		kdb_printf(" suppressed,\nuse 'ps A' to see all.\n");
	}
}

void kdb_ps1(const struct task_struct *p)
{
	int cpu;
	unsigned long tmp;

	if (!p ||
	    copy_from_kernel_nofault(&tmp, (char *)p, sizeof(unsigned long)))
		return;

	cpu = kdb_process_cpu(p);
	kdb_printf("0x%px %8d %8d  %d %4d   %c  0x%px %c%s\n",
		   (void *)p, p->pid, p->parent->pid,
		   kdb_task_has_cpu(p), kdb_process_cpu(p),
		   kdb_task_state_char(p),
		   (void *)(&p->thread),
		   p == kdb_curr_task(raw_smp_processor_id()) ? '*' : ' ',
		   p->comm);
	if (kdb_task_has_cpu(p)) {
		if (!KDB_TSK(cpu)) {
			kdb_printf("  Error: no saved data for this cpu\n");
		} else {
			if (KDB_TSK(cpu) != p)
				kdb_printf("  Error: does not match running "
				   "process table (0x%px)\n", KDB_TSK(cpu));
		}
	}
}

/*
 * kdb_ps - This function implements the 'ps' command which shows a
 *	    list of the active processes.
 *
 * ps [<state_chars>]   Show processes, optionally selecting only those whose
 *                      state character is found in <state_chars>.
 */
static int kdb_ps(int argc, const char **argv)
{
	struct task_struct *g, *p;
	const char *mask;
	unsigned long cpu;

	if (argc == 0)
		kdb_ps_suppressed();
	kdb_printf("%-*s      Pid   Parent [*] cpu State %-*s Command\n",
		(int)(2*sizeof(void *))+2, "Task Addr",
		(int)(2*sizeof(void *))+2, "Thread");
	mask = argc ? argv[1] : kdbgetenv("PS");
	/* Run the active tasks first */
	for_each_online_cpu(cpu) {
		if (KDB_FLAG(CMD_INTERRUPT))
			return 0;
		p = kdb_curr_task(cpu);
		if (kdb_task_state(p, mask))
			kdb_ps1(p);
	}
	kdb_printf("\n");
	/* Now the real tasks */
	for_each_process_thread(g, p) {
		if (KDB_FLAG(CMD_INTERRUPT))
			return 0;
		if (kdb_task_state(p, mask))
			kdb_ps1(p);
	}

	return 0;
}

/*
 * kdb_pid - This function implements the 'pid' command which switches
 *	the currently active process.
 *		pid [<pid> | R]
 */
static int kdb_pid(int argc, const char **argv)
{
	struct task_struct *p;
	unsigned long val;
	int diag;

	if (argc > 1)
		return KDB_ARGCOUNT;

	if (argc) {
		if (strcmp(argv[1], "R") == 0) {
			p = KDB_TSK(kdb_initial_cpu);
		} else {
			diag = kdbgetularg(argv[1], &val);
			if (diag)
				return KDB_BADINT;

			p = find_task_by_pid_ns((pid_t)val,	&init_pid_ns);
			if (!p) {
				kdb_printf("No task with pid=%d\n", (pid_t)val);
				return 0;
			}
		}
		kdb_set_current_task(p);
	}
	kdb_printf("KDB current process is %s(pid=%d)\n",
		   kdb_current_task->comm,
		   kdb_current_task->pid);

	return 0;
}

static int kdb_kgdb(int argc, const char **argv)
{
	return KDB_CMD_KGDB;
}

/*
 * kdb_help - This function implements the 'help' and '?' commands.
 */
static int kdb_help(int argc, const char **argv)
{
	kdbtab_t *kt;

	kdb_printf("%-15.15s %-20.20s %s\n", "Command", "Usage", "Description");
	kdb_printf("-----------------------------"
		   "-----------------------------\n");
	list_for_each_entry(kt, &kdb_cmds_head, list_node) {
		char *space = "";
		if (KDB_FLAG(CMD_INTERRUPT))
			return 0;
		if (!kdb_check_flags(kt->flags, kdb_cmd_enabled, true))
			continue;
		if (strlen(kt->usage) > 20)
			space = "\n                                    ";
		kdb_printf("%-15.15s %-20s%s%s\n", kt->name,
			   kt->usage, space, kt->help);
	}
	return 0;
}

/*
 * kdb_kill - This function implements the 'kill' commands.
 */
static int kdb_kill(int argc, const char **argv)
{
	long sig, pid;
	char *endp;
	struct task_struct *p;

	if (argc != 2)
		return KDB_ARGCOUNT;

	sig = simple_strtol(argv[1], &endp, 0);
	if (*endp)
		return KDB_BADINT;
	if ((sig >= 0) || !valid_signal(-sig)) {
		kdb_printf("Invalid signal parameter.<-signal>\n");
		return 0;
	}
	sig = -sig;

	pid = simple_strtol(argv[2], &endp, 0);
	if (*endp)
		return KDB_BADINT;
	if (pid <= 0) {
		kdb_printf("Process ID must be large than 0.\n");
		return 0;
	}

	/* Find the process. */
	p = find_task_by_pid_ns(pid, &init_pid_ns);
	if (!p) {
		kdb_printf("The specified process isn't found.\n");
		return 0;
	}
	p = p->group_leader;
	kdb_send_sig(p, sig);
	return 0;
}

/*
 * Most of this code has been lifted from kernel/timer.c::sys_sysinfo().
 * I cannot call that code directly from kdb, it has an unconditional
 * cli()/sti() and calls routines that take locks which can stop the debugger.
 */
static void kdb_sysinfo(struct sysinfo *val)
{
	u64 uptime = ktime_get_mono_fast_ns();

	memset(val, 0, sizeof(*val));
	val->uptime = div_u64(uptime, NSEC_PER_SEC);
	val->loads[0] = avenrun[0];
	val->loads[1] = avenrun[1];
	val->loads[2] = avenrun[2];
	val->procs = nr_threads-1;
	si_meminfo(val);

	return;
}

/*
 * kdb_summary - This function implements the 'summary' command.
 */
static int kdb_summary(int argc, const char **argv)
{
	time64_t now;
	struct sysinfo val;

	if (argc)
		return KDB_ARGCOUNT;

	kdb_printf("sysname    %s\n", init_uts_ns.name.sysname);
	kdb_printf("release    %s\n", init_uts_ns.name.release);
	kdb_printf("version    %s\n", init_uts_ns.name.version);
	kdb_printf("machine    %s\n", init_uts_ns.name.machine);
	kdb_printf("nodename   %s\n", init_uts_ns.name.nodename);
	kdb_printf("domainname %s\n", init_uts_ns.name.domainname);

	now = __ktime_get_real_seconds();
	kdb_printf("date       %ptTs tz_minuteswest %d\n", &now, sys_tz.tz_minuteswest);
	kdb_sysinfo(&val);
	kdb_printf("uptime     ");
	if (val.uptime > (24*60*60)) {
		int days = val.uptime / (24*60*60);
		val.uptime %= (24*60*60);
		kdb_printf("%d day%s ", days, days == 1 ? "" : "s");
	}
	kdb_printf("%02ld:%02ld\n", val.uptime/(60*60), (val.uptime/60)%60);

	kdb_printf("load avg   %ld.%02ld %ld.%02ld %ld.%02ld\n",
		LOAD_INT(val.loads[0]), LOAD_FRAC(val.loads[0]),
		LOAD_INT(val.loads[1]), LOAD_FRAC(val.loads[1]),
		LOAD_INT(val.loads[2]), LOAD_FRAC(val.loads[2]));

	/* Display in kilobytes */
#define K(x) ((x) << (PAGE_SHIFT - 10))
	kdb_printf("\nMemTotal:       %8lu kB\nMemFree:        %8lu kB\n"
		   "Buffers:        %8lu kB\n",
		   K(val.totalram), K(val.freeram), K(val.bufferram));
	return 0;
}

/*
 * kdb_per_cpu - This function implements the 'per_cpu' command.
 */
static int kdb_per_cpu(int argc, const char **argv)
{
	char fmtstr[64];
	int cpu, diag, nextarg = 1;
	unsigned long addr, symaddr, val, bytesperword = 0, whichcpu = ~0UL;

	if (argc < 1 || argc > 3)
		return KDB_ARGCOUNT;

	diag = kdbgetaddrarg(argc, argv, &nextarg, &symaddr, NULL, NULL);
	if (diag)
		return diag;

	if (argc >= 2) {
		diag = kdbgetularg(argv[2], &bytesperword);
		if (diag)
			return diag;
	}
	if (!bytesperword)
		bytesperword = KDB_WORD_SIZE;
	else if (bytesperword > KDB_WORD_SIZE)
		return KDB_BADWIDTH;
	sprintf(fmtstr, "%%0%dlx ", (int)(2*bytesperword));
	if (argc >= 3) {
		diag = kdbgetularg(argv[3], &whichcpu);
		if (diag)
			return diag;
		if (whichcpu >= nr_cpu_ids || !cpu_online(whichcpu)) {
			kdb_printf("cpu %ld is not online\n", whichcpu);
			return KDB_BADCPUNUM;
		}
	}

	/* Most architectures use __per_cpu_offset[cpu], some use
	 * __per_cpu_offset(cpu), smp has no __per_cpu_offset.
	 */
#ifdef	__per_cpu_offset
#define KDB_PCU(cpu) __per_cpu_offset(cpu)
#else
#ifdef	CONFIG_SMP
#define KDB_PCU(cpu) __per_cpu_offset[cpu]
#else
#define KDB_PCU(cpu) 0
#endif
#endif
	for_each_online_cpu(cpu) {
		if (KDB_FLAG(CMD_INTERRUPT))
			return 0;

		if (whichcpu != ~0UL && whichcpu != cpu)
			continue;
		addr = symaddr + KDB_PCU(cpu);
		diag = kdb_getword(&val, addr, bytesperword);
		if (diag) {
			kdb_printf("%5d " kdb_bfd_vma_fmt0 " - unable to "
				   "read, diag=%d\n", cpu, addr, diag);
			continue;
		}
		kdb_printf("%5d ", cpu);
		kdb_md_line(fmtstr, addr,
			bytesperword == KDB_WORD_SIZE,
			1, bytesperword, 1, 1, 0);
	}
#undef KDB_PCU
	return 0;
}

/*
 * display help for the use of cmd | grep pattern
 */
static int kdb_grep_help(int argc, const char **argv)
{
	kdb_printf("Usage of  cmd args | grep pattern:\n");
	kdb_printf("  Any command's output may be filtered through an ");
	kdb_printf("emulated 'pipe'.\n");
	kdb_printf("  'grep' is just a key word.\n");
	kdb_printf("  The pattern may include a very limited set of "
		   "metacharacters:\n");
	kdb_printf("   pattern or ^pattern or pattern$ or ^pattern$\n");
	kdb_printf("  And if there are spaces in the pattern, you may "
		   "quote it:\n");
	kdb_printf("   \"pat tern\" or \"^pat tern\" or \"pat tern$\""
		   " or \"^pat tern$\"\n");
	return 0;
}

/**
 * kdb_register() - This function is used to register a kernel debugger
 *                  command.
 * @cmd: pointer to kdb command
 *
 * Note that it's the job of the caller to keep the memory for the cmd
 * allocated until unregister is called.
 */
int kdb_register(kdbtab_t *cmd)
{
	kdbtab_t *kp;

	list_for_each_entry(kp, &kdb_cmds_head, list_node) {
		if (strcmp(kp->name, cmd->name) == 0) {
			kdb_printf("Duplicate kdb cmd: %s, func %p help %s\n",
				   cmd->name, cmd->func, cmd->help);
			return 1;
		}
	}

	list_add_tail(&cmd->list_node, &kdb_cmds_head);
	return 0;
}
EXPORT_SYMBOL_GPL(kdb_register);

/**
 * kdb_register_table() - This function is used to register a kdb command
 *                        table.
 * @kp: pointer to kdb command table
 * @len: length of kdb command table
 */
void kdb_register_table(kdbtab_t *kp, size_t len)
{
	while (len--) {
		list_add_tail(&kp->list_node, &kdb_cmds_head);
		kp++;
	}
}

/**
 * kdb_unregister() - This function is used to unregister a kernel debugger
 *                    command. It is generally called when a module which
 *                    implements kdb command is unloaded.
 * @cmd: pointer to kdb command
 */
void kdb_unregister(kdbtab_t *cmd)
{
	list_del(&cmd->list_node);
}
EXPORT_SYMBOL_GPL(kdb_unregister);

static kdbtab_t maintab[] = {
	{	.name = "md",
		.func = kdb_md,
		.usage = "<vaddr>",
		.help = "Display Memory Contents, also mdWcN, e.g. md8c1",
		.minlen = 1,
		.flags = KDB_ENABLE_MEM_READ | KDB_REPEAT_NO_ARGS,
	},
	{	.name = "mdr",
		.func = kdb_md,
		.usage = "<vaddr> <bytes>",
		.help = "Display Raw Memory",
		.flags = KDB_ENABLE_MEM_READ | KDB_REPEAT_NO_ARGS,
	},
	{	.name = "mdp",
		.func = kdb_md,
		.usage = "<paddr> <bytes>",
		.help = "Display Physical Memory",
		.flags = KDB_ENABLE_MEM_READ | KDB_REPEAT_NO_ARGS,
	},
	{	.name = "mds",
		.func = kdb_md,
		.usage = "<vaddr>",
		.help = "Display Memory Symbolically",
		.flags = KDB_ENABLE_MEM_READ | KDB_REPEAT_NO_ARGS,
	},
	{	.name = "mm",
		.func = kdb_mm,
		.usage = "<vaddr> <contents>",
		.help = "Modify Memory Contents",
		.flags = KDB_ENABLE_MEM_WRITE | KDB_REPEAT_NO_ARGS,
	},
	{	.name = "go",
		.func = kdb_go,
		.usage = "[<vaddr>]",
		.help = "Continue Execution",
		.minlen = 1,
		.flags = KDB_ENABLE_REG_WRITE |
			     KDB_ENABLE_ALWAYS_SAFE_NO_ARGS,
	},
	{	.name = "rd",
		.func = kdb_rd,
		.usage = "",
		.help = "Display Registers",
		.flags = KDB_ENABLE_REG_READ,
	},
	{	.name = "rm",
		.func = kdb_rm,
		.usage = "<reg> <contents>",
		.help = "Modify Registers",
		.flags = KDB_ENABLE_REG_WRITE,
	},
	{	.name = "ef",
		.func = kdb_ef,
		.usage = "<vaddr>",
		.help = "Display exception frame",
		.flags = KDB_ENABLE_MEM_READ,
	},
	{	.name = "bt",
		.func = kdb_bt,
		.usage = "[<vaddr>]",
		.help = "Stack traceback",
		.minlen = 1,
		.flags = KDB_ENABLE_MEM_READ | KDB_ENABLE_INSPECT_NO_ARGS,
	},
	{	.name = "btp",
		.func = kdb_bt,
		.usage = "<pid>",
		.help = "Display stack for process <pid>",
		.flags = KDB_ENABLE_INSPECT,
	},
	{	.name = "bta",
		.func = kdb_bt,
		.usage = "[<state_chars>|A]",
		.help = "Backtrace all processes whose state matches",
		.flags = KDB_ENABLE_INSPECT,
	},
	{	.name = "btc",
		.func = kdb_bt,
		.usage = "",
		.help = "Backtrace current process on each cpu",
		.flags = KDB_ENABLE_INSPECT,
	},
	{	.name = "btt",
		.func = kdb_bt,
		.usage = "<vaddr>",
		.help = "Backtrace process given its struct task address",
		.flags = KDB_ENABLE_MEM_READ | KDB_ENABLE_INSPECT_NO_ARGS,
	},
	{	.name = "env",
		.func = kdb_env,
		.usage = "",
		.help = "Show environment variables",
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
	{	.name = "set",
		.func = kdb_set,
		.usage = "",
		.help = "Set environment variables",
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
	{	.name = "help",
		.func = kdb_help,
		.usage = "",
		.help = "Display Help Message",
		.minlen = 1,
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
	{	.name = "?",
		.func = kdb_help,
		.usage = "",
		.help = "Display Help Message",
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
	{	.name = "cpu",
		.func = kdb_cpu,
		.usage = "<cpunum>",
		.help = "Switch to new cpu",
		.flags = KDB_ENABLE_ALWAYS_SAFE_NO_ARGS,
	},
	{	.name = "kgdb",
		.func = kdb_kgdb,
		.usage = "",
		.help = "Enter kgdb mode",
		.flags = 0,
	},
	{	.name = "ps",
		.func = kdb_ps,
		.usage = "[<state_chars>|A]",
		.help = "Display active task list",
		.flags = KDB_ENABLE_INSPECT,
	},
	{	.name = "pid",
		.func = kdb_pid,
		.usage = "<pidnum>",
		.help = "Switch to another task",
		.flags = KDB_ENABLE_INSPECT,
	},
	{	.name = "reboot",
		.func = kdb_reboot,
		.usage = "",
		.help = "Reboot the machine immediately",
		.flags = KDB_ENABLE_REBOOT,
	},
#if defined(CONFIG_MODULES)
	{	.name = "lsmod",
		.func = kdb_lsmod,
		.usage = "",
		.help = "List loaded kernel modules",
		.flags = KDB_ENABLE_INSPECT,
	},
#endif
#if defined(CONFIG_MAGIC_SYSRQ)
	{	.name = "sr",
		.func = kdb_sr,
		.usage = "<key>",
		.help = "Magic SysRq key",
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
#endif
#if defined(CONFIG_PRINTK)
	{	.name = "dmesg",
		.func = kdb_dmesg,
		.usage = "[lines]",
		.help = "Display syslog buffer",
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
#endif
	{	.name = "defcmd",
		.func = kdb_defcmd,
		.usage = "name \"usage\" \"help\"",
		.help = "Define a set of commands, down to endefcmd",
		/*
		 * Macros are always safe because when executed each
		 * internal command re-enters kdb_parse() and is safety
		 * checked individually.
		 */
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
	{	.name = "kill",
		.func = kdb_kill,
		.usage = "<-signal> <pid>",
		.help = "Send a signal to a process",
		.flags = KDB_ENABLE_SIGNAL,
	},
	{	.name = "summary",
		.func = kdb_summary,
		.usage = "",
		.help = "Summarize the system",
		.minlen = 4,
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
	{	.name = "per_cpu",
		.func = kdb_per_cpu,
		.usage = "<sym> [<bytes>] [<cpu>]",
		.help = "Display per_cpu variables",
		.minlen = 3,
		.flags = KDB_ENABLE_MEM_READ,
	},
	{	.name = "grephelp",
		.func = kdb_grep_help,
		.usage = "",
		.help = "Display help on | grep",
		.flags = KDB_ENABLE_ALWAYS_SAFE,
	},
};

static kdbtab_t nmicmd = {
	.name = "disable_nmi",
	.func = kdb_disable_nmi,
	.usage = "",
	.help = "Disable NMI entry to KDB",
	.flags = KDB_ENABLE_ALWAYS_SAFE,
};

/* Initialize the kdb command table. */
static void __init kdb_inittab(void)
{
	kdb_register_table(maintab, ARRAY_SIZE(maintab));
	if (arch_kgdb_ops.enable_nmi)
		kdb_register_table(&nmicmd, 1);
}

/* Execute any commands defined in kdb_cmds.  */
static void __init kdb_cmd_init(void)
{
	int i, diag;
	for (i = 0; kdb_cmds[i]; ++i) {
		diag = kdb_parse(kdb_cmds[i]);
		if (diag)
			kdb_printf("kdb command %s failed, kdb diag %d\n",
				kdb_cmds[i], diag);
	}
	if (defcmd_in_progress) {
		kdb_printf("Incomplete 'defcmd' set, forcing endefcmd\n");
		kdb_parse("endefcmd");
	}
}

/* Initialize kdb_printf, breakpoint tables and kdb state */
void __init kdb_init(int lvl)
{
	static int kdb_init_lvl = KDB_NOT_INITIALIZED;
	int i;

	if (kdb_init_lvl == KDB_INIT_FULL || lvl <= kdb_init_lvl)
		return;
	for (i = kdb_init_lvl; i < lvl; i++) {
		switch (i) {
		case KDB_NOT_INITIALIZED:
			kdb_inittab();		/* Initialize Command Table */
			kdb_initbptab();	/* Initialize Breakpoints */
			break;
		case KDB_INIT_EARLY:
			kdb_cmd_init();		/* Build kdb_cmds tables */
			break;
		}
	}
	kdb_init_lvl = lvl;
}