	local_irq_restore(flags);
}

/* Some archs may not define MCOUNT_INSN_SIZE */
#ifndef MCOUNT_INSN_SIZE
# define MCOUNT_INSN_SIZE 0
#endif

static void
stack_trace_call(unsigned long ip, unsigned long parent_ip,
		 struct ftrace_ops *op, struct pt_regs *pt_regs)
{