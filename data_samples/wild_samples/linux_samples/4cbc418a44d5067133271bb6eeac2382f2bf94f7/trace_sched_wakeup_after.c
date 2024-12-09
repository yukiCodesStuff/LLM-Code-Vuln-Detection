	if (ret) {
		pr_info("wakeup trace: Couldn't activate tracepoint"
			" probe to kernel_sched_migrate_task\n");
		goto fail_deprobe_sched_switch;
	}

	wakeup_reset(tr);

	/*
	 * Don't let the tracer_enabled = 1 show up before
	 * the wakeup_task is reset. This may be overkill since
	 * wakeup_reset does a spin_unlock after setting the
	 * wakeup_task to NULL, but I want to be safe.
	 * This is a slow path anyway.
	 */
	smp_wmb();

	if (start_func_tracer(tr, is_graph(tr)))
		printk(KERN_ERR "failed to start wakeup tracer\n");

	return;
fail_deprobe_sched_switch:
	unregister_trace_sched_switch(probe_wakeup_sched_switch, NULL);
fail_deprobe_wake_new:
	unregister_trace_sched_wakeup_new(probe_wakeup, NULL);
fail_deprobe:
	unregister_trace_sched_wakeup(probe_wakeup, NULL);
}

static void stop_wakeup_tracer(struct trace_array *tr)
{
	tracer_enabled = 0;
	stop_func_tracer(tr, is_graph(tr));
	unregister_trace_sched_switch(probe_wakeup_sched_switch, NULL);
	unregister_trace_sched_wakeup_new(probe_wakeup, NULL);
	unregister_trace_sched_wakeup(probe_wakeup, NULL);
	unregister_trace_sched_migrate_task(probe_wakeup_migrate_task, NULL);
}

static bool wakeup_busy;

static int __wakeup_tracer_init(struct trace_array *tr)
{
	save_flags = tr->trace_flags;

	/* non overwrite screws up the latency tracers */
	set_tracer_flag(tr, TRACE_ITER_OVERWRITE, 1);
	set_tracer_flag(tr, TRACE_ITER_LATENCY_FMT, 1);

	tr->max_latency = 0;
	wakeup_trace = tr;
	ftrace_init_array_ops(tr, wakeup_tracer_call);
	start_wakeup_tracer(tr);

	wakeup_busy = true;
	return 0;
}

static int wakeup_tracer_init(struct trace_array *tr)
{
	if (wakeup_busy)
		return -EBUSY;

	wakeup_dl = 0;
	wakeup_rt = 0;
	return __wakeup_tracer_init(tr);
}

static int wakeup_rt_tracer_init(struct trace_array *tr)
{
	if (wakeup_busy)
		return -EBUSY;

	wakeup_dl = 0;
	wakeup_rt = 1;
	return __wakeup_tracer_init(tr);
}

static int wakeup_dl_tracer_init(struct trace_array *tr)
{
	if (wakeup_busy)
		return -EBUSY;

	wakeup_dl = 1;
	wakeup_rt = 0;
	return __wakeup_tracer_init(tr);
}

static void wakeup_tracer_reset(struct trace_array *tr)
{
	int lat_flag = save_flags & TRACE_ITER_LATENCY_FMT;
	int overwrite_flag = save_flags & TRACE_ITER_OVERWRITE;

	stop_wakeup_tracer(tr);
	/* make sure we put back any tasks we are tracing */
	wakeup_reset(tr);

	set_tracer_flag(tr, TRACE_ITER_LATENCY_FMT, lat_flag);
	set_tracer_flag(tr, TRACE_ITER_OVERWRITE, overwrite_flag);
	ftrace_reset_array_ops(tr);
	wakeup_busy = false;
}

static void wakeup_tracer_start(struct trace_array *tr)
{
	wakeup_reset(tr);
	tracer_enabled = 1;
}

static void wakeup_tracer_stop(struct trace_array *tr)
{
	tracer_enabled = 0;
}

static struct tracer wakeup_tracer __read_mostly =
{
	.name		= "wakeup",
	.init		= wakeup_tracer_init,
	.reset		= wakeup_tracer_reset,
	.start		= wakeup_tracer_start,
	.stop		= wakeup_tracer_stop,
	.print_max	= true,
	.print_header	= wakeup_print_header,
	.print_line	= wakeup_print_line,
	.flag_changed	= wakeup_flag_changed,
#ifdef CONFIG_FTRACE_SELFTEST
	.selftest    = trace_selftest_startup_wakeup,
#endif
	.open		= wakeup_trace_open,
	.close		= wakeup_trace_close,
	.allow_instances = true,
	.use_max_tr	= true,
};

static struct tracer wakeup_rt_tracer __read_mostly =
{
	.name		= "wakeup_rt",
	.init		= wakeup_rt_tracer_init,
	.reset		= wakeup_tracer_reset,
	.start		= wakeup_tracer_start,
	.stop		= wakeup_tracer_stop,
	.print_max	= true,
	.print_header	= wakeup_print_header,
	.print_line	= wakeup_print_line,
	.flag_changed	= wakeup_flag_changed,
#ifdef CONFIG_FTRACE_SELFTEST
	.selftest    = trace_selftest_startup_wakeup,
#endif
	.open		= wakeup_trace_open,
	.close		= wakeup_trace_close,
	.allow_instances = true,
	.use_max_tr	= true,
};

static struct tracer wakeup_dl_tracer __read_mostly =
{
	.name		= "wakeup_dl",
	.init		= wakeup_dl_tracer_init,
	.reset		= wakeup_tracer_reset,
	.start		= wakeup_tracer_start,
	.stop		= wakeup_tracer_stop,
	.print_max	= true,
	.print_header	= wakeup_print_header,
	.print_line	= wakeup_print_line,
	.flag_changed	= wakeup_flag_changed,
#ifdef CONFIG_FTRACE_SELFTEST
	.selftest    = trace_selftest_startup_wakeup,
#endif
	.open		= wakeup_trace_open,
	.close		= wakeup_trace_close,
	.allow_instances = true,
	.use_max_tr	= true,
};

__init static int init_wakeup_tracer(void)
{
	int ret;

	ret = register_tracer(&wakeup_tracer);
	if (ret)
		return ret;

	ret = register_tracer(&wakeup_rt_tracer);
	if (ret)
		return ret;

	ret = register_tracer(&wakeup_dl_tracer);
	if (ret)
		return ret;

	return 0;
}
core_initcall(init_wakeup_tracer);
	if (ret) {
		pr_info("wakeup trace: Couldn't activate tracepoint"
			" probe to kernel_sched_migrate_task\n");
		goto fail_deprobe_sched_switch;
	}

	wakeup_reset(tr);

	/*
	 * Don't let the tracer_enabled = 1 show up before
	 * the wakeup_task is reset. This may be overkill since
	 * wakeup_reset does a spin_unlock after setting the
	 * wakeup_task to NULL, but I want to be safe.
	 * This is a slow path anyway.
	 */
	smp_wmb();

	if (start_func_tracer(tr, is_graph(tr)))
		printk(KERN_ERR "failed to start wakeup tracer\n");

	return;
fail_deprobe_sched_switch:
	unregister_trace_sched_switch(probe_wakeup_sched_switch, NULL);
fail_deprobe_wake_new:
	unregister_trace_sched_wakeup_new(probe_wakeup, NULL);
fail_deprobe:
	unregister_trace_sched_wakeup(probe_wakeup, NULL);
}

static void stop_wakeup_tracer(struct trace_array *tr)
{
	tracer_enabled = 0;
	stop_func_tracer(tr, is_graph(tr));
	unregister_trace_sched_switch(probe_wakeup_sched_switch, NULL);
	unregister_trace_sched_wakeup_new(probe_wakeup, NULL);
	unregister_trace_sched_wakeup(probe_wakeup, NULL);
	unregister_trace_sched_migrate_task(probe_wakeup_migrate_task, NULL);
}

static bool wakeup_busy;

static int __wakeup_tracer_init(struct trace_array *tr)
{
	save_flags = tr->trace_flags;

	/* non overwrite screws up the latency tracers */
	set_tracer_flag(tr, TRACE_ITER_OVERWRITE, 1);
	set_tracer_flag(tr, TRACE_ITER_LATENCY_FMT, 1);

	tr->max_latency = 0;
	wakeup_trace = tr;
	ftrace_init_array_ops(tr, wakeup_tracer_call);
	start_wakeup_tracer(tr);

	wakeup_busy = true;
	return 0;
}

static int wakeup_tracer_init(struct trace_array *tr)
{
	if (wakeup_busy)
		return -EBUSY;

	wakeup_dl = 0;
	wakeup_rt = 0;
	return __wakeup_tracer_init(tr);
}

static int wakeup_rt_tracer_init(struct trace_array *tr)
{
	if (wakeup_busy)
		return -EBUSY;

	wakeup_dl = 0;
	wakeup_rt = 1;
	return __wakeup_tracer_init(tr);
}

static int wakeup_dl_tracer_init(struct trace_array *tr)
{
	if (wakeup_busy)
		return -EBUSY;

	wakeup_dl = 1;
	wakeup_rt = 0;
	return __wakeup_tracer_init(tr);
}

static void wakeup_tracer_reset(struct trace_array *tr)
{
	int lat_flag = save_flags & TRACE_ITER_LATENCY_FMT;
	int overwrite_flag = save_flags & TRACE_ITER_OVERWRITE;

	stop_wakeup_tracer(tr);
	/* make sure we put back any tasks we are tracing */
	wakeup_reset(tr);

	set_tracer_flag(tr, TRACE_ITER_LATENCY_FMT, lat_flag);
	set_tracer_flag(tr, TRACE_ITER_OVERWRITE, overwrite_flag);
	ftrace_reset_array_ops(tr);
	wakeup_busy = false;
}

static void wakeup_tracer_start(struct trace_array *tr)
{
	wakeup_reset(tr);
	tracer_enabled = 1;
}

static void wakeup_tracer_stop(struct trace_array *tr)
{
	tracer_enabled = 0;
}

static struct tracer wakeup_tracer __read_mostly =
{
	.name		= "wakeup",
	.init		= wakeup_tracer_init,
	.reset		= wakeup_tracer_reset,
	.start		= wakeup_tracer_start,
	.stop		= wakeup_tracer_stop,
	.print_max	= true,
	.print_header	= wakeup_print_header,
	.print_line	= wakeup_print_line,
	.flag_changed	= wakeup_flag_changed,
#ifdef CONFIG_FTRACE_SELFTEST
	.selftest    = trace_selftest_startup_wakeup,
#endif
	.open		= wakeup_trace_open,
	.close		= wakeup_trace_close,
	.allow_instances = true,
	.use_max_tr	= true,
};

static struct tracer wakeup_rt_tracer __read_mostly =
{
	.name		= "wakeup_rt",
	.init		= wakeup_rt_tracer_init,
	.reset		= wakeup_tracer_reset,
	.start		= wakeup_tracer_start,
	.stop		= wakeup_tracer_stop,
	.print_max	= true,
	.print_header	= wakeup_print_header,
	.print_line	= wakeup_print_line,
	.flag_changed	= wakeup_flag_changed,
#ifdef CONFIG_FTRACE_SELFTEST
	.selftest    = trace_selftest_startup_wakeup,
#endif
	.open		= wakeup_trace_open,
	.close		= wakeup_trace_close,
	.allow_instances = true,
	.use_max_tr	= true,
};

static struct tracer wakeup_dl_tracer __read_mostly =
{
	.name		= "wakeup_dl",
	.init		= wakeup_dl_tracer_init,
	.reset		= wakeup_tracer_reset,
	.start		= wakeup_tracer_start,
	.stop		= wakeup_tracer_stop,
	.print_max	= true,
	.print_header	= wakeup_print_header,
	.print_line	= wakeup_print_line,
	.flag_changed	= wakeup_flag_changed,
#ifdef CONFIG_FTRACE_SELFTEST
	.selftest    = trace_selftest_startup_wakeup,
#endif
	.open		= wakeup_trace_open,
	.close		= wakeup_trace_close,
	.allow_instances = true,
	.use_max_tr	= true,
};

__init static int init_wakeup_tracer(void)
{
	int ret;

	ret = register_tracer(&wakeup_tracer);
	if (ret)
		return ret;

	ret = register_tracer(&wakeup_rt_tracer);
	if (ret)
		return ret;

	ret = register_tracer(&wakeup_dl_tracer);
	if (ret)
		return ret;

	return 0;
}
core_initcall(init_wakeup_tracer);