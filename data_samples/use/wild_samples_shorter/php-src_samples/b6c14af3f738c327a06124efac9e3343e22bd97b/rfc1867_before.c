static int multipart_buffer_headers(multipart_buffer *self, zend_llist *header TSRMLS_DC)
{
	char *line;
	mime_header_entry prev_entry, entry;
	int prev_len, cur_len;

	/* didn't find boundary, abort */
	if (!find_boundary(self, self->boundary TSRMLS_CC)) {

			{
				zval file_size, error_type;

				error_type.value.lval = cancel_upload;
				error_type.type = IS_LONG;

				/* Add $foo[error] */
				if (cancel_upload) {
					file_size.value.lval = 0;
					file_size.type = IS_LONG;
				} else {
					file_size.value.lval = total_bytes;
					file_size.type = IS_LONG;
				}

				if (is_arr_upload) {
					snprintf(lbuf, llen, "%s[error][%s]", abuf, array_index);
					snprintf(lbuf, llen, "%s_size", param);
				}
				if (!is_anonymous) {
					safe_php_register_variable_ex(lbuf, &file_size, NULL, 0 TSRMLS_CC);
				}

				/* Add $foo[size] */
				if (is_arr_upload) {
				} else {
					snprintf(lbuf, llen, "%s[size]", param);
				}
				register_http_post_files_variable_ex(lbuf, &file_size, http_post_files, 0 TSRMLS_CC);
			}
			efree(param);
		}
	}