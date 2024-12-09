static int multipart_buffer_headers(multipart_buffer *self, zend_llist *header TSRMLS_DC)
{
	char *line;
	mime_header_entry prev_entry = {0}, entry;
	int prev_len, cur_len;

	/* didn't find boundary, abort */
	if (!find_boundary(self, self->boundary TSRMLS_CC)) {

			{
				zval file_size, error_type;
				int size_overflow = 0;
				char file_size_buf[65];

				ZVAL_LONG(&error_type, cancel_upload);

				/* Add $foo[error] */
				if (cancel_upload) {
					ZVAL_LONG(&file_size, 0);
				} else {
					if (total_bytes > LONG_MAX) {
#ifdef PHP_WIN32
						if (_i64toa_s(total_bytes, file_size_buf, 65, 10)) {
							file_size_buf[0] = '0';
							file_size_buf[1] = '\0';
						}
#else
						{
							int __len = snprintf(file_size_buf, 65, "%lld", total_bytes);
							file_size_buf[__len] = '\0';
						}
#endif
						size_overflow = 1;

					} else {
						ZVAL_LONG(&file_size, total_bytes);
					}
				}

				if (is_arr_upload) {
					snprintf(lbuf, llen, "%s[error][%s]", abuf, array_index);
					snprintf(lbuf, llen, "%s_size", param);
				}
				if (!is_anonymous) {
					if (size_overflow) {
						ZVAL_STRING(&file_size, file_size_buf, 1);
					}
					safe_php_register_variable_ex(lbuf, &file_size, NULL, size_overflow TSRMLS_CC);
				}

				/* Add $foo[size] */
				if (is_arr_upload) {
				} else {
					snprintf(lbuf, llen, "%s[size]", param);
				}
				if (size_overflow) {
					ZVAL_STRING(&file_size, file_size_buf, 1);
				}
				register_http_post_files_variable_ex(lbuf, &file_size, http_post_files, size_overflow TSRMLS_CC);
			}
			efree(param);
		}
	}