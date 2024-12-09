{
	struct stat sb;
	static const char *index_files[] = { "index.php", "index.html", NULL };
	char *buf = safe_pemalloc(1, request->vpath_len, 1 + document_root_len + 1 + sizeof("index.html"), 1);
	char *p = buf, *prev_patch = 0, *q, *vpath;
	size_t prev_patch_len;
	int  is_static_file = 0;

	if (!buf) {
		return;
	}
	if (!client->request.content) {
		client->request.content = pemalloc(parser->content_length, 1);
		if (!client->request.content) {
			return -1;
		}
		client->request.content_len = 0;
	}
	if (client->current_header_name) {
		char *header_name = safe_pemalloc(client->current_header_name_len, 1, 1, 1);
		if (!header_name) {
			return -1;
		}
		memmove(header_name, client->current_header_name, client->current_header_name_len);
		client->current_header_name = header_name;
		client->current_header_name_allocated = 1;
	}