		num_nodes, (num_nodes > 1 ? "s" : ""),
		num_cpus,  (num_cpus  > 1 ? "s" : ""));

	/* Any cleanup work */
	smp_cpus_done(setup_max_cpus);
}
