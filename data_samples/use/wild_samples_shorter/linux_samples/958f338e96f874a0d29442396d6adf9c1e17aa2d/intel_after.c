	}

	check_mpx_erratum(c);

	/*
	 * Get the number of SMT siblings early from the extended topology
	 * leaf, if available. Otherwise try the legacy SMT detection.
	 */
	if (detect_extended_topology_early(c) < 0)
		detect_ht_early(c);
}

#ifdef CONFIG_X86_32
/*