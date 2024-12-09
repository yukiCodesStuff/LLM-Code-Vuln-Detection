	local_irq_restore(flags);
}

static void
stack_trace_call(unsigned long ip, unsigned long parent_ip,
		 struct ftrace_ops *op, struct pt_regs *pt_regs)
{