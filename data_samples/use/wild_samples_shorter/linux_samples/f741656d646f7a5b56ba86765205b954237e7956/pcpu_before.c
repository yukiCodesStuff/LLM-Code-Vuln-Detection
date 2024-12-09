	 * Only those at cpu present map has its sys interface.
	 */
	if (info->flags & XEN_PCPU_FLAGS_INVALID) {
		if (pcpu)
			unregister_and_remove_pcpu(pcpu);
		return 0;
	}

	if (!pcpu) {