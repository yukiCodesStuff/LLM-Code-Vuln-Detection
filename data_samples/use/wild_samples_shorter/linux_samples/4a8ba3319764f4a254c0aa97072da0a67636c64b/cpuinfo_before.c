	cpuinfo.has_div = fcpu_has(cpu, "altr,has-div");
	cpuinfo.has_mul = fcpu_has(cpu, "altr,has-mul");
	cpuinfo.has_mulx = fcpu_has(cpu, "altr,has-mulx");

	if (IS_ENABLED(CONFIG_NIOS2_HW_DIV_SUPPORT) && !cpuinfo.has_div)
		err_cpu("DIV");
