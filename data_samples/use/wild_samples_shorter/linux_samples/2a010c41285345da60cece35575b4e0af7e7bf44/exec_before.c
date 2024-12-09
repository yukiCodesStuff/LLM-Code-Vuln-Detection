			 path_noexec(&file->f_path)))
		goto exit;

	err = deny_write_access(file);
	if (err)
		goto exit;

out:
	return file;

exit:
 *
 * Returns ERR_PTR on failure or allocated struct file on success.
 *
 * As this is a wrapper for the internal do_open_execat(), callers
 * must call allow_write_access() before fput() on release. Also see
 * do_close_execat().
 */
struct file *open_exec(const char *name)
{
/* Matches do_open_execat() */
static void do_close_execat(struct file *file)
{
	if (!file)
		return;
	allow_write_access(file);
	fput(file);
}

static void free_bprm(struct linux_binprm *bprm)
{
		bprm->file = bprm->interpreter;
		bprm->interpreter = NULL;

		allow_write_access(exec);
		if (unlikely(bprm->have_execfd)) {
			if (bprm->executable) {
				fput(exec);
				return -ENOEXEC;