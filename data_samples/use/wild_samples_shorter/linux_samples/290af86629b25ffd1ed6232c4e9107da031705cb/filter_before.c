		 */
		goto out_err_free;

	/* We are guaranteed to never error here with cBPF to eBPF
	 * transitions, since there's no issue with type compatibility
	 * checks on program arrays.
	 */
	fp = bpf_prog_select_runtime(fp, &err);

	kfree(old_prog);
	return fp;
