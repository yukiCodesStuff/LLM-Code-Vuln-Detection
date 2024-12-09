	if (current->curr_ret_stack == FTRACE_RETFUNC_DEPTH - 1) {
		atomic_inc(&current->trace_overrun);
		return -EBUSY;
	}

	calltime = trace_clock_local();

	index = ++current->curr_ret_stack;
	barrier();
	current->ret_stack[index].ret = ret;
	current->ret_stack[index].func = func;
	current->ret_stack[index].calltime = calltime;
#ifdef HAVE_FUNCTION_GRAPH_FP_TEST
	current->ret_stack[index].fp = frame_pointer;
#endif
#ifdef HAVE_FUNCTION_GRAPH_RET_ADDR_PTR
	current->ret_stack[index].retp = retp;
#endif
	return 0;
}

int function_graph_enter(unsigned long ret, unsigned long func,
			 unsigned long frame_pointer, unsigned long *retp)
{
	struct ftrace_graph_ent trace;

	/*
	 * Skip graph tracing if the return location is served by direct trampoline,
	 * since call sequence and return addresses is unpredicatable anymore.
	 * Ex: BPF trampoline may call original function and may skip frame
	 * depending on type of BPF programs attached.
	 */
	if (ftrace_direct_func_count &&
	    ftrace_find_rec_direct(ret - MCOUNT_INSN_SIZE))
		return -EBUSY;
	trace.func = func;
	trace.depth = ++current->curr_ret_depth;

	if (ftrace_push_return_trace(ret, func, frame_pointer, retp))
		goto out;

	/* Only trace if the calling function expects to */
	if (!ftrace_graph_entry(&trace))
		goto out_ret;

	return 0;
 out_ret:
	current->curr_ret_stack--;
 out:
	current->curr_ret_depth--;
	return -EBUSY;
}

/* Retrieve a function return address to the trace stack on thread info.*/
static void
ftrace_pop_return_trace(struct ftrace_graph_ret *trace, unsigned long *ret,
			unsigned long frame_pointer)
{
	int index;

	index = current->curr_ret_stack;

	if (unlikely(index < 0 || index >= FTRACE_RETFUNC_DEPTH)) {
		ftrace_graph_stop();
		WARN_ON(1);
		/* Might as well panic, otherwise we have no where to go */
		*ret = (unsigned long)panic;
		return;
	}