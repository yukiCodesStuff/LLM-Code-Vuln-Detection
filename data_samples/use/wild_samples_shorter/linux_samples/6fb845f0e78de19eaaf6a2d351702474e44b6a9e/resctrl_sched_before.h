#ifndef _ASM_X86_RESCTRL_SCHED_H
#define _ASM_X86_RESCTRL_SCHED_H

#ifdef CONFIG_X86_RESCTRL

#include <linux/sched.h>
#include <linux/jump_label.h>


static inline void resctrl_sched_in(void) {}

#endif /* CONFIG_X86_RESCTRL */

#endif /* _ASM_X86_RESCTRL_SCHED_H */