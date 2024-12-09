	for_each_possible_cpu(cpu) {
		cpu_state = per_cpu_ptr(sha1_mb_alg_state.alg_cstate, cpu);
		kfree(cpu_state->mgr);
	}
	free_percpu(sha1_mb_alg_state.alg_cstate);
}

module_init(sha1_mb_mod_init);
module_exit(sha1_mb_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SHA1 Secure Hash Algorithm, multi buffer accelerated");

MODULE_ALIAS("sha1");