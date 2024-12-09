#include <linux/sched/sysctl.h>
#include <linux/kexec.h>
#include <linux/bpf.h>

#include <asm/uaccess.h>
#include <asm/processor.h>

		.mode		= 0644,
		.proc_handler	= proc_doulongvec_minmax,
	},
	{ }
};

static struct ctl_table debug_table[] = {