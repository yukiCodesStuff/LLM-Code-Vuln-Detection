{
	if (fp->jited)
		module_memfree(fp->bpf_func);

	bpf_prog_unlock_free(fp);
}