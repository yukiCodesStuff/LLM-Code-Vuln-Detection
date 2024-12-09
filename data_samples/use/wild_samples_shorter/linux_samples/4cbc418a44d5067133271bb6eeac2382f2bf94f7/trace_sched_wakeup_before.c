	if (ret) {
		pr_info("wakeup trace: Couldn't activate tracepoint"
			" probe to kernel_sched_migrate_task\n");
		return;
	}

	wakeup_reset(tr);

		printk(KERN_ERR "failed to start wakeup tracer\n");

	return;
fail_deprobe_wake_new:
	unregister_trace_sched_wakeup_new(probe_wakeup, NULL);
fail_deprobe:
	unregister_trace_sched_wakeup(probe_wakeup, NULL);