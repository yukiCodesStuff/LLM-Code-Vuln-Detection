static int multipart_buffer_headers(multipart_buffer *self, zend_llist *header TSRMLS_DC)
{
	char *line;
	mime_header_entry prev_entry, entry;
	int prev_len, cur_len;

	/* didn't find boundary, abort */
	if (!find_boundary(self, self->boundary TSRMLS_CC)) {