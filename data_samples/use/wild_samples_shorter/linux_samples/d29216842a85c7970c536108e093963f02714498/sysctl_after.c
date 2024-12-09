#include <linux/sched/sysctl.h>
#include <linux/kexec.h>
#include <linux/bpf.h>
#include <linux/mount.h>

#include <asm/uaccess.h>
#include <asm/processor.h>

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
};

static struct ctl_table debug_table[] = {