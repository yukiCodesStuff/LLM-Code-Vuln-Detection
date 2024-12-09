{
	if (fp->jited)
		module_free(NULL, fp->bpf_func);

	bpf_prog_unlock_free(fp);
}