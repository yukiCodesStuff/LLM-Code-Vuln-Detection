				       int depth, void *data)
{
	struct param_info *info = data;
	const void *prop;
	void *dest;
	u64 val;
	int i, len;

	if (depth != 1 ||
	    (strcmp(uname, "chosen") != 0 && strcmp(uname, "chosen@0") != 0))
		return 0;