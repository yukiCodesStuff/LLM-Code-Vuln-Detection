	if (task_stack_end_corrupted(current)) {
		print_max_stack();
		BUG();
	}

 out:
	arch_spin_unlock(&stack_trace_max_lock);
	local_irq_restore(flags);
}

static void
stack_trace_call(unsigned long ip, unsigned long parent_ip,
		 struct ftrace_ops *op, struct pt_regs *pt_regs)
{
	unsigned long stack;

	preempt_disable_notrace();

	/* no atomic needed, we only modify this variable by this cpu */
	__this_cpu_inc(disable_stack_tracer);
	if (__this_cpu_read(disable_stack_tracer) != 1)
		goto out;

	/* If rcu is not watching, then save stack trace can fail */
	if (!rcu_is_watching())
		goto out;

	ip += MCOUNT_INSN_SIZE;

	check_stack(ip, &stack);

 out:
	__this_cpu_dec(disable_stack_tracer);
	/* prevent recursion in schedule */
	preempt_enable_notrace();
}

static struct ftrace_ops trace_ops __read_mostly =
{
	.func = stack_trace_call,
	.flags = FTRACE_OPS_FL_RECURSION_SAFE,
};

static ssize_t
stack_max_size_read(struct file *filp, char __user *ubuf,
		    size_t count, loff_t *ppos)
{
	unsigned long *ptr = filp->private_data;
	char buf[64];
	int r;

	r = snprintf(buf, sizeof(buf), "%ld\n", *ptr);
	if (r > sizeof(buf))
		r = sizeof(buf);
	return simple_read_from_buffer(ubuf, count, ppos, buf, r);
}