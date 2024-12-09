	size_t prev_patch_len;
	int  is_static_file = 0;

	if (!buf) {
		return;
	}

	memmove(p, document_root, document_root_len);
	p += document_root_len;
	vpath = p;
	if (request->vpath_len > 0 && request->vpath[0] != '/') {
	php_cli_server_client *client = parser->data;
	if (!client->request.content) {
		client->request.content = pemalloc(parser->content_length, 1);
		if (!client->request.content) {
			return -1;
		}
		client->request.content_len = 0;
	}
	memmove(client->request.content + client->request.content_len, at, length);
	client->request.content_len += length;
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