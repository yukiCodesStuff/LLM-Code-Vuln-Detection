	if (unlikely(rec->counter == 0)) {
		ret = -EBUSY;
		goto out;
	}

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	avg = rec->time;
	do_div(avg, rec->counter);
	if (tracing_thresh && (avg < tracing_thresh))
		goto out;
#endif

	kallsyms_lookup(rec->ip, NULL, NULL, NULL, str);
	seq_printf(m, "  %-30.30s  %10lu", str, rec->counter);

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	seq_puts(m, "    ");

	/* Sample standard deviation (s^2) */
	if (rec->counter <= 1)
		stddev = 0;
	else {
		/*
		 * Apply Welford's method:
		 * s^2 = 1 / (n * (n-1)) * (n * \Sum (x_i)^2 - (\Sum x_i)^2)
		 */
		stddev = rec->counter * rec->time_squared -
			 rec->time * rec->time;

		/*
		 * Divide only 1000 for ns^2 -> us^2 conversion.
		 * trace_print_graph_duration will divide 1000 again.
		 */
		do_div(stddev, rec->counter * (rec->counter - 1) * 1000);
	}
	else {
		/*
		 * Apply Welford's method:
		 * s^2 = 1 / (n * (n-1)) * (n * \Sum (x_i)^2 - (\Sum x_i)^2)
		 */
		stddev = rec->counter * rec->time_squared -
			 rec->time * rec->time;

		/*
		 * Divide only 1000 for ns^2 -> us^2 conversion.
		 * trace_print_graph_duration will divide 1000 again.
		 */
		do_div(stddev, rec->counter * (rec->counter - 1) * 1000);
	}

	trace_seq_init(&s);
	trace_print_graph_duration(rec->time, &s);
	trace_seq_puts(&s, "    ");
	trace_print_graph_duration(avg, &s);
	trace_seq_puts(&s, "    ");
	trace_print_graph_duration(stddev, &s);
	trace_print_seq(m, &s);
#endif
	seq_putc(m, '\n');
out:
	mutex_unlock(&ftrace_profile_lock);

	return ret;
}

static void ftrace_profile_reset(struct ftrace_profile_stat *stat)
{
	struct ftrace_profile_page *pg;

	pg = stat->pages = stat->start;

	while (pg) {
		memset(pg->records, 0, PROFILE_RECORDS_SIZE);
		pg->index = 0;
		pg = pg->next;
	}

	memset(stat->hash, 0,
	       FTRACE_PROFILE_HASH_SIZE * sizeof(struct hlist_head));
}

int ftrace_profile_pages_init(struct ftrace_profile_stat *stat)
{
	struct ftrace_profile_page *pg;
	int functions;
	int pages;
	int i;

	/* If we already allocated, do nothing */
	if (stat->pages)
		return 0;

	stat->pages = (void *)get_zeroed_page(GFP_KERNEL);
	if (!stat->pages)
		return -ENOMEM;

#ifdef CONFIG_DYNAMIC_FTRACE
	functions = ftrace_update_tot_cnt;
#else
	/*
	 * We do not know the number of functions that exist because
	 * dynamic tracing is what counts them. With past experience
	 * we have around 20K functions. That should be more than enough.
	 * It is highly unlikely we will execute every function in
	 * the kernel.
	 */
	functions = 20000;
#endif

	pg = stat->start = stat->pages;

	pages = DIV_ROUND_UP(functions, PROFILES_PER_PAGE);

	for (i = 1; i < pages; i++) {
		pg->next = (void *)get_zeroed_page(GFP_KERNEL);
		if (!pg->next)
			goto out_free;
		pg = pg->next;
	}

	return 0;

 out_free:
	pg = stat->start;
	while (pg) {
		unsigned long tmp = (unsigned long)pg;

		pg = pg->next;
		free_page(tmp);
	}

	stat->pages = NULL;
	stat->start = NULL;

	return -ENOMEM;
}

static int ftrace_profile_init_cpu(int cpu)
{
	struct ftrace_profile_stat *stat;
	int size;

	stat = &per_cpu(ftrace_profile_stats, cpu);

	if (stat->hash) {
		/* If the profile is already created, simply reset it */
		ftrace_profile_reset(stat);
		return 0;
	}

	/*
	 * We are profiling all functions, but usually only a few thousand
	 * functions are hit. We'll make a hash of 1024 items.
	 */
	size = FTRACE_PROFILE_HASH_SIZE;

	stat->hash = kcalloc(size, sizeof(struct hlist_head), GFP_KERNEL);

	if (!stat->hash)
		return -ENOMEM;

	/* Preallocate the function profiling pages */
	if (ftrace_profile_pages_init(stat) < 0) {
		kfree(stat->hash);
		stat->hash = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int ftrace_profile_init(void)
{
	int cpu;
	int ret = 0;

	for_each_possible_cpu(cpu) {
		ret = ftrace_profile_init_cpu(cpu);
		if (ret)
			break;
	}

	return ret;
}

/* interrupts must be disabled */
static struct ftrace_profile *
ftrace_find_profiled_func(struct ftrace_profile_stat *stat, unsigned long ip)
{
	struct ftrace_profile *rec;
	struct hlist_head *hhd;
	unsigned long key;

	key = hash_long(ip, FTRACE_PROFILE_HASH_BITS);
	hhd = &stat->hash[key];

	if (hlist_empty(hhd))
		return NULL;

	hlist_for_each_entry_rcu_notrace(rec, hhd, node) {
		if (rec->ip == ip)
			return rec;
	}

	return NULL;
}

static void ftrace_add_profile(struct ftrace_profile_stat *stat,
			       struct ftrace_profile *rec)
{
	unsigned long key;

	key = hash_long(rec->ip, FTRACE_PROFILE_HASH_BITS);
	hlist_add_head_rcu(&rec->node, &stat->hash[key]);
}

/*
 * The memory is already allocated, this simply finds a new record to use.
 */
static struct ftrace_profile *
ftrace_profile_alloc(struct ftrace_profile_stat *stat, unsigned long ip)
{
	struct ftrace_profile *rec = NULL;

	/* prevent recursion (from NMIs) */
	if (atomic_inc_return(&stat->disabled) != 1)
		goto out;

	/*
	 * Try to find the function again since an NMI
	 * could have added it
	 */
	rec = ftrace_find_profiled_func(stat, ip);
	if (rec)
		goto out;

	if (stat->pages->index == PROFILES_PER_PAGE) {
		if (!stat->pages->next)
			goto out;
		stat->pages = stat->pages->next;
	}

	rec = &stat->pages->records[stat->pages->index++];
	rec->ip = ip;
	ftrace_add_profile(stat, rec);

 out:
	atomic_dec(&stat->disabled);

	return rec;
}

static void
function_profile_call(unsigned long ip, unsigned long parent_ip,
		      struct ftrace_ops *ops, struct pt_regs *regs)
{
	struct ftrace_profile_stat *stat;
	struct ftrace_profile *rec;
	unsigned long flags;

	if (!ftrace_profile_enabled)
		return;

	local_irq_save(flags);

	stat = this_cpu_ptr(&ftrace_profile_stats);
	if (!stat->hash || !ftrace_profile_enabled)
		goto out;

	rec = ftrace_find_profiled_func(stat, ip);
	if (!rec) {
		rec = ftrace_profile_alloc(stat, ip);
		if (!rec)
			goto out;
	}

	rec->counter++;
 out:
	local_irq_restore(flags);
}

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
static bool fgraph_graph_time = true;

void ftrace_graph_graph_time_control(bool enable)
{
	fgraph_graph_time = enable;
}

static int profile_graph_entry(struct ftrace_graph_ent *trace)
{
	struct ftrace_ret_stack *ret_stack;

	function_profile_call(trace->func, 0, NULL, NULL);

	/* If function graph is shutting down, ret_stack can be NULL */
	if (!current->ret_stack)
		return 0;

	ret_stack = ftrace_graph_get_ret_stack(current, 0);
	if (ret_stack)
		ret_stack->subtime = 0;

	return 1;
}

static void profile_graph_return(struct ftrace_graph_ret *trace)
{
	struct ftrace_ret_stack *ret_stack;
	struct ftrace_profile_stat *stat;
	unsigned long long calltime;
	struct ftrace_profile *rec;
	unsigned long flags;

	local_irq_save(flags);
	stat = this_cpu_ptr(&ftrace_profile_stats);
	if (!stat->hash || !ftrace_profile_enabled)
		goto out;

	/* If the calltime was zero'd ignore it */
	if (!trace->calltime)
		goto out;

	calltime = trace->rettime - trace->calltime;

	if (!fgraph_graph_time) {

		/* Append this call time to the parent time to subtract */
		ret_stack = ftrace_graph_get_ret_stack(current, 1);
		if (ret_stack)
			ret_stack->subtime += calltime;

		ret_stack = ftrace_graph_get_ret_stack(current, 0);
		if (ret_stack && ret_stack->subtime < calltime)
			calltime -= ret_stack->subtime;
		else
			calltime = 0;
	}

	rec = ftrace_find_profiled_func(stat, trace->func);
	if (rec) {
		rec->time += calltime;
		rec->time_squared += calltime * calltime;
	}

 out:
	local_irq_restore(flags);
}

static struct fgraph_ops fprofiler_ops = {
	.entryfunc = &profile_graph_entry,
	.retfunc = &profile_graph_return,
};

static int register_ftrace_profiler(void)
{
	return register_ftrace_graph(&fprofiler_ops);
}

static void unregister_ftrace_profiler(void)
{
	unregister_ftrace_graph(&fprofiler_ops);
}
#else
static struct ftrace_ops ftrace_profile_ops __read_mostly = {
	.func		= function_profile_call,
	.flags		= FTRACE_OPS_FL_RECURSION_SAFE | FTRACE_OPS_FL_INITIALIZED,
	INIT_OPS_HASH(ftrace_profile_ops)
};

static int register_ftrace_profiler(void)
{
	return register_ftrace_function(&ftrace_profile_ops);
}