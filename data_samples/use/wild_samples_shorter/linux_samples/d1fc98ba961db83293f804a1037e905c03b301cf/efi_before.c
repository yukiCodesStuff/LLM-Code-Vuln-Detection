				       int depth, void *data)
{
	struct param_info *info = data;
	void *prop, *dest;
	unsigned long len;
	u64 val;
	int i;

	if (depth != 1 ||
	    (strcmp(uname, "chosen") != 0 && strcmp(uname, "chosen@0") != 0))
		return 0;