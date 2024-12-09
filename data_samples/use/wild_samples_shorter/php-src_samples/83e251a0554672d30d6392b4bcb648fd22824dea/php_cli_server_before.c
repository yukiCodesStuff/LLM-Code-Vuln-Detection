	static const char *index_files[] = { "index.php", "index.html", NULL };
	char *buf = safe_pemalloc(1, request->vpath_len, 1 + document_root_len + 1 + sizeof("index.html"), 1);
	char *p = buf, *prev_path = NULL, *q, *vpath;
	size_t prev_path_len;
	int  is_static_file = 0;

	if (!buf) {
		return;