void bpf_jit_free(struct bpf_prog *fp)
{
	if (fp->jited)
		module_free(NULL, fp->bpf_func);

	bpf_prog_unlock_free(fp);
}