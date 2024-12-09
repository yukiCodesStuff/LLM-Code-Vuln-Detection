		num_nodes, (num_nodes > 1 ? "s" : ""),
		num_cpus,  (num_cpus  > 1 ? "s" : ""));

	/* Final decision about SMT support */
	cpu_smt_check_topology();
	/* Any cleanup work */
	smp_cpus_done(setup_max_cpus);
}
