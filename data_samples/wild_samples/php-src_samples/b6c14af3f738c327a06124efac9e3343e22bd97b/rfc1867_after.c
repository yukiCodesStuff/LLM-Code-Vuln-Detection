{
	char *line;
	mime_header_entry prev_entry = {0}, entry;
	int prev_len, cur_len;

	/* didn't find boundary, abort */
	if (!find_boundary(self, self->boundary TSRMLS_CC)) {
		return 0;
	}

	/* get lines of text, or CRLF_CRLF */

	while( (line = get_line(self TSRMLS_CC)) && line[0] != '\0' )
	{
		/* add header to table */
		char *key = line;
		char *value = NULL;

		if (php_rfc1867_encoding_translation(TSRMLS_C)) {
			self->input_encoding = zend_multibyte_encoding_detector(line, strlen(line), self->detect_order, self->detect_order_size TSRMLS_CC);
		}

		/* space in the beginning means same header */
		if (!isspace(line[0])) {
			value = strchr(line, ':');
		}

		if (value) {
			*value = 0;
			do { value++; } while(isspace(*value));

			entry.value = estrdup(value);
			entry.key = estrdup(key);

		} else if (zend_llist_count(header)) { /* If no ':' on the line, add to previous line */

			prev_len = strlen(prev_entry.value);
			cur_len = strlen(line);

			entry.value = emalloc(prev_len + cur_len + 1);
			memcpy(entry.value, prev_entry.value, prev_len);
			memcpy(entry.value + prev_len, line, cur_len);
			entry.value[cur_len + prev_len] = '\0';

			entry.key = estrdup(prev_entry.key);

			zend_llist_remove_tail(header);
		} else {
			continue;
		}

		zend_llist_add_element(header, &entry);
		prev_entry = entry;
	}

	return 1;
}

static char *php_mime_get_hdr_value(zend_llist header, char *key)
{
	mime_header_entry *entry;

	if (key == NULL) {
		return NULL;
	}

	entry = zend_llist_get_first(&header);
	while (entry) {
		if (!strcasecmp(entry->key, key)) {
			return entry->value;
		}
		entry = zend_llist_get_next(&header);
	}

	return NULL;
}

static char *php_ap_getword(const zend_encoding *encoding, char **line, char stop TSRMLS_DC)
{
	char *pos = *line, quote;
	char *res;

	while (*pos && *pos != stop) {
		if ((quote = *pos) == '"' || quote == '\'') {
			++pos;
			while (*pos && *pos != quote) {
				if (*pos == '\\' && pos[1] && pos[1] == quote) {
					pos += 2;
				} else {
					++pos;
				}
			}
			if (*pos) {
				++pos;
			}
		} else ++pos;
	}
	if (*pos == '\0') {
		res = estrdup(*line);
		*line += strlen(*line);
		return res;
	}

	res = estrndup(*line, pos - *line);

	while (*pos == stop) {
		++pos;
	}

	*line = pos;
	return res;
}

static char *substring_conf(char *start, int len, char quote)
{
	char *result = emalloc(len + 1);
	char *resp = result;
	int i;

	for (i = 0; i < len && start[i] != quote; ++i) {
		if (start[i] == '\\' && (start[i + 1] == '\\' || (quote && start[i + 1] == quote))) {
			*resp++ = start[++i];
		} else {
			*resp++ = start[i];
		}
	}

	*resp = '\0';
	return result;
}

static char *php_ap_getword_conf(const zend_encoding *encoding, char *str TSRMLS_DC)
{
	while (*str && isspace(*str)) {
		++str;
	}

	if (!*str) {
		return estrdup("");
	}

	if (*str == '"' || *str == '\'') {
		char quote = *str;

		str++;
		return substring_conf(str, strlen(str), quote);
	} else {
		char *strend = str;

		while (*strend && !isspace(*strend)) {
			++strend;
		}
		return substring_conf(str, strend - str, 0);
	}
}

static char *php_ap_basename(const zend_encoding *encoding, char *path TSRMLS_DC)
{
	char *s = strrchr(path, '\\');
	char *s2 = strrchr(path, '/');

	if (s && s2) {
		if (s > s2) {
			++s;
		} else {
			s = ++s2;
		}
		return s;
	} else if (s) {
		return ++s;
	} else if (s2) {
		return ++s2;
	}
	return path;
}

/*
 * Search for a string in a fixed-length byte string.
 * If partial is true, partial matches are allowed at the end of the buffer.
 * Returns NULL if not found, or a pointer to the start of the first match.
 */
static void *php_ap_memstr(char *haystack, int haystacklen, char *needle, int needlen, int partial)
{
	int len = haystacklen;
	char *ptr = haystack;

	/* iterate through first character matches */
	while( (ptr = memchr(ptr, needle[0], len)) ) {

		/* calculate length after match */
		len = haystacklen - (ptr - (char *)haystack);

		/* done if matches up to capacity of buffer */
		if (memcmp(needle, ptr, needlen < len ? needlen : len) == 0 && (partial || len >= needlen)) {
			break;
		}

		/* next character */
		ptr++; len--;
	}

	return ptr;
}

/* read until a boundary condition */
static int multipart_buffer_read(multipart_buffer *self, char *buf, int bytes, int *end TSRMLS_DC)
{
	int len, max;
	char *bound;

	/* fill buffer if needed */
	if (bytes > self->bytes_in_buffer) {
		fill_buffer(self TSRMLS_CC);
	}

	/* look for a potential boundary match, only read data up to that point */
	if ((bound = php_ap_memstr(self->buf_begin, self->bytes_in_buffer, self->boundary_next, self->boundary_next_len, 1))) {
		max = bound - self->buf_begin;
		if (end && php_ap_memstr(self->buf_begin, self->bytes_in_buffer, self->boundary_next, self->boundary_next_len, 0)) {
			*end = 1;
		}
	} else {
		max = self->bytes_in_buffer;
	}

	/* maximum number of bytes we are reading */
	len = max < bytes-1 ? max : bytes-1;

	/* if we read any data... */
	if (len > 0) {

		/* copy the data */
		memcpy(buf, self->buf_begin, len);
		buf[len] = 0;

		if (bound && len > 0 && buf[len-1] == '\r') {
			buf[--len] = 0;
		}

		/* update the buffer */
		self->bytes_in_buffer -= len;
		self->buf_begin += len;
	}

	return len;
}

/*
  XXX: this is horrible memory-usage-wise, but we only expect
  to do this on small pieces of form data.
*/
static char *multipart_buffer_read_body(multipart_buffer *self, unsigned int *len TSRMLS_DC)
{
	char buf[FILLUNIT], *out=NULL;
	int total_bytes=0, read_bytes=0;

	while((read_bytes = multipart_buffer_read(self, buf, sizeof(buf), NULL TSRMLS_CC))) {
		out = erealloc(out, total_bytes + read_bytes + 1);
		memcpy(out + total_bytes, buf, read_bytes);
		total_bytes += read_bytes;
	}

	if (out) {
		out[total_bytes] = '\0';
	}
	*len = total_bytes;

	return out;
}
/* }}} */

/*
 * The combined READER/HANDLER
 *
 */

SAPI_API SAPI_POST_HANDLER_FUNC(rfc1867_post_handler) /* {{{ */
{
	char *boundary, *s = NULL, *boundary_end = NULL, *start_arr = NULL, *array_index = NULL;
	char *temp_filename = NULL, *lbuf = NULL, *abuf = NULL;
	int boundary_len = 0, cancel_upload = 0, is_arr_upload = 0, array_len = 0;
	int64_t total_bytes = 0, max_file_size = 0;
	int skip_upload = 0, anonindex = 0, is_anonymous;
	zval *http_post_files = NULL;
	HashTable *uploaded_files = NULL;
	multipart_buffer *mbuff;
	zval *array_ptr = (zval *) arg;
	int fd = -1;
	zend_llist header;
	void *event_extra_data = NULL;
	unsigned int llen = 0;
	int upload_cnt = INI_INT("max_file_uploads");
	const zend_encoding *internal_encoding = zend_multibyte_get_internal_encoding(TSRMLS_C);
	php_rfc1867_getword_t getword;
	php_rfc1867_getword_conf_t getword_conf;
	php_rfc1867_basename_t _basename;
	long count = 0;

	if (php_rfc1867_encoding_translation(TSRMLS_C) && internal_encoding) {
		getword = php_rfc1867_getword;
		getword_conf = php_rfc1867_getword_conf;
		_basename = php_rfc1867_basename;
	} else {
		getword = php_ap_getword;
		getword_conf = php_ap_getword_conf;
		_basename = php_ap_basename;
	}

	if (SG(post_max_size) > 0 && SG(request_info).content_length > SG(post_max_size)) {
		sapi_module.sapi_error(E_WARNING, "POST Content-Length of %ld bytes exceeds the limit of %ld bytes", SG(request_info).content_length, SG(post_max_size));
		return;
	}

	/* Get the boundary */
	boundary = strstr(content_type_dup, "boundary");
	if (!boundary) {
		int content_type_len = strlen(content_type_dup);
		char *content_type_lcase = estrndup(content_type_dup, content_type_len);

		php_strtolower(content_type_lcase, content_type_len);
		boundary = strstr(content_type_lcase, "boundary");
		if (boundary) {
			boundary = content_type_dup + (boundary - content_type_lcase);
		}
		efree(content_type_lcase);
	}

	if (!boundary || !(boundary = strchr(boundary, '='))) {
		sapi_module.sapi_error(E_WARNING, "Missing boundary in multipart/form-data POST data");
		return;
	}

	boundary++;
	boundary_len = strlen(boundary);

	if (boundary[0] == '"') {
		boundary++;
		boundary_end = strchr(boundary, '"');
		if (!boundary_end) {
			sapi_module.sapi_error(E_WARNING, "Invalid boundary in multipart/form-data POST data");
			return;
		}
	} else {
		/* search for the end of the boundary */
		boundary_end = strpbrk(boundary, ",;");
	}
	if (boundary_end) {
		boundary_end[0] = '\0';
		boundary_len = boundary_end-boundary;
	}

	/* Initialize the buffer */
	if (!(mbuff = multipart_buffer_new(boundary, boundary_len TSRMLS_CC))) {
		sapi_module.sapi_error(E_WARNING, "Unable to initialize the input buffer");
		return;
	}

	/* Initialize $_FILES[] */
	zend_hash_init(&PG(rfc1867_protected_variables), 5, NULL, NULL, 0);

	ALLOC_HASHTABLE(uploaded_files);
	zend_hash_init(uploaded_files, 5, NULL, (dtor_func_t) free_estring, 0);
	SG(rfc1867_uploaded_files) = uploaded_files;

	ALLOC_ZVAL(http_post_files);
	array_init(http_post_files);
	INIT_PZVAL(http_post_files);
	PG(http_globals)[TRACK_VARS_FILES] = http_post_files;

	zend_llist_init(&header, sizeof(mime_header_entry), (llist_dtor_func_t) php_free_hdr_entry, 0);

	if (php_rfc1867_callback != NULL) {
		multipart_event_start event_start;

		event_start.content_length = SG(request_info).content_length;
		if (php_rfc1867_callback(MULTIPART_EVENT_START, &event_start, &event_extra_data TSRMLS_CC) == FAILURE) {
			goto fileupload_done;
		}
	}

	while (!multipart_buffer_eof(mbuff TSRMLS_CC))
	{
		char buff[FILLUNIT];
		char *cd = NULL, *param = NULL, *filename = NULL, *tmp = NULL;
		size_t blen = 0, wlen = 0;
		off_t offset;

		zend_llist_clean(&header);

		if (!multipart_buffer_headers(mbuff, &header TSRMLS_CC)) {
			goto fileupload_done;
		}

		if ((cd = php_mime_get_hdr_value(header, "Content-Disposition"))) {
			char *pair = NULL;
			int end = 0;

			while (isspace(*cd)) {
				++cd;
			}

			while (*cd && (pair = getword(mbuff->input_encoding, &cd, ';' TSRMLS_CC)))
			{
				char *key = NULL, *word = pair;

				while (isspace(*cd)) {
					++cd;
				}

				if (strchr(pair, '=')) {
					key = getword(mbuff->input_encoding, &pair, '=' TSRMLS_CC);

					if (!strcasecmp(key, "name")) {
						if (param) {
							efree(param);
						}
						param = getword_conf(mbuff->input_encoding, pair TSRMLS_CC);
						if (mbuff->input_encoding && internal_encoding) {
							unsigned char *new_param;
							size_t new_param_len;
							if ((size_t)-1 != zend_multibyte_encoding_converter(&new_param, &new_param_len, (unsigned char *)param, strlen(param), internal_encoding, mbuff->input_encoding TSRMLS_CC)) {
								efree(param);
								param = (char *)new_param;
							}
						}
					} else if (!strcasecmp(key, "filename")) {
						if (filename) {
							efree(filename);
						}
						filename = getword_conf(mbuff->input_encoding, pair TSRMLS_CC);
						if (mbuff->input_encoding && internal_encoding) {
							unsigned char *new_filename;
							size_t new_filename_len;
							if ((size_t)-1 != zend_multibyte_encoding_converter(&new_filename, &new_filename_len, (unsigned char *)filename, strlen(filename), internal_encoding, mbuff->input_encoding TSRMLS_CC)) {
								efree(filename);
								filename = (char *)new_filename;
							}
						}
					}
				}
				if (key) {
					efree(key);
				}
				efree(word);
			}

			/* Normal form variable, safe to read all data into memory */
			if (!filename && param) {
				unsigned int value_len;
				char *value = multipart_buffer_read_body(mbuff, &value_len TSRMLS_CC);
				unsigned int new_val_len; /* Dummy variable */

				if (!value) {
					value = estrdup("");
					value_len = 0;
				}

				if (mbuff->input_encoding && internal_encoding) {
					unsigned char *new_value;
					size_t new_value_len;
					if ((size_t)-1 != zend_multibyte_encoding_converter(&new_value, &new_value_len, (unsigned char *)value, value_len, internal_encoding, mbuff->input_encoding TSRMLS_CC)) {
						efree(value);
						value = (char *)new_value;
						value_len = new_value_len;
					}
				}

				if (++count <= PG(max_input_vars) && sapi_module.input_filter(PARSE_POST, param, &value, value_len, &new_val_len TSRMLS_CC)) {
					if (php_rfc1867_callback != NULL) {
						multipart_event_formdata event_formdata;
						size_t newlength = new_val_len;

						event_formdata.post_bytes_processed = SG(read_post_bytes);
						event_formdata.name = param;
						event_formdata.value = &value;
						event_formdata.length = new_val_len;
						event_formdata.newlength = &newlength;
						if (php_rfc1867_callback(MULTIPART_EVENT_FORMDATA, &event_formdata, &event_extra_data TSRMLS_CC) == FAILURE) {
							efree(param);
							efree(value);
							continue;
						}
						new_val_len = newlength;
					}
					safe_php_register_variable(param, value, new_val_len, array_ptr, 0 TSRMLS_CC);
				} else {
					if (count == PG(max_input_vars) + 1) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Input variables exceeded %ld. To increase the limit change max_input_vars in php.ini.", PG(max_input_vars));
					}
				
					if (php_rfc1867_callback != NULL) {
						multipart_event_formdata event_formdata;

						event_formdata.post_bytes_processed = SG(read_post_bytes);
						event_formdata.name = param;
						event_formdata.value = &value;
						event_formdata.length = value_len;
						event_formdata.newlength = NULL;
						php_rfc1867_callback(MULTIPART_EVENT_FORMDATA, &event_formdata, &event_extra_data TSRMLS_CC);
					}
				}

				if (!strcasecmp(param, "MAX_FILE_SIZE")) {
					max_file_size = atoll(value);
				}

				efree(param);
				efree(value);
				continue;
			}

			/* If file_uploads=off, skip the file part */
			if (!PG(file_uploads)) {
				skip_upload = 1;
			} else if (upload_cnt <= 0) {
				skip_upload = 1;
				sapi_module.sapi_error(E_WARNING, "Maximum number of allowable file uploads has been exceeded");
			}

			/* Return with an error if the posted data is garbled */
			if (!param && !filename) {
				sapi_module.sapi_error(E_WARNING, "File Upload Mime headers garbled");
				goto fileupload_done;
			}

			if (!param) {
				is_anonymous = 1;
				param = emalloc(MAX_SIZE_ANONNAME);
				snprintf(param, MAX_SIZE_ANONNAME, "%u", anonindex++);
			} else {
				is_anonymous = 0;
			}

			/* New Rule: never repair potential malicious user input */
			if (!skip_upload) {
				long c = 0;
				tmp = param;

				while (*tmp) {
					if (*tmp == '[') {
						c++;
					} else if (*tmp == ']') {
						c--;
						if (tmp[1] && tmp[1] != '[') {
							skip_upload = 1;
							break;
						}
					}
					if (c < 0) {
						skip_upload = 1;
						break;
					}
					tmp++;
				}
				/* Brackets should always be closed */
				if(c != 0) {
					skip_upload = 1;
				}
			}

			total_bytes = cancel_upload = 0;
			temp_filename = NULL;
			fd = -1;

			if (!skip_upload && php_rfc1867_callback != NULL) {
				multipart_event_file_start event_file_start;

				event_file_start.post_bytes_processed = SG(read_post_bytes);
				event_file_start.name = param;
				event_file_start.filename = &filename;
				if (php_rfc1867_callback(MULTIPART_EVENT_FILE_START, &event_file_start, &event_extra_data TSRMLS_CC) == FAILURE) {
					temp_filename = "";
					efree(param);
					efree(filename);
					continue;
				}
			}

			if (skip_upload) {
				efree(param);
				efree(filename);
				continue;
			}

			if (filename[0] == '\0') {
#if DEBUG_FILE_UPLOAD
				sapi_module.sapi_error(E_NOTICE, "No file uploaded");
#endif
				cancel_upload = UPLOAD_ERROR_D;
			}

			offset = 0;
			end = 0;

			if (!cancel_upload) {
				/* only bother to open temp file if we have data */
				blen = multipart_buffer_read(mbuff, buff, sizeof(buff), &end TSRMLS_CC);
#if DEBUG_FILE_UPLOAD
				if (blen > 0) {
#else
				/* in non-debug mode we have no problem with 0-length files */
				{
#endif
					fd = php_open_temporary_fd_ex(PG(upload_tmp_dir), "php", &temp_filename, 1 TSRMLS_CC);
					upload_cnt--;
					if (fd == -1) {
						sapi_module.sapi_error(E_WARNING, "File upload error - unable to create a temporary file");
						cancel_upload = UPLOAD_ERROR_E;
					}
				}
			}

			while (!cancel_upload && (blen > 0))
			{
				if (php_rfc1867_callback != NULL) {
					multipart_event_file_data event_file_data;

					event_file_data.post_bytes_processed = SG(read_post_bytes);
					event_file_data.offset = offset;
					event_file_data.data = buff;
					event_file_data.length = blen;
					event_file_data.newlength = &blen;
					if (php_rfc1867_callback(MULTIPART_EVENT_FILE_DATA, &event_file_data, &event_extra_data TSRMLS_CC) == FAILURE) {
						cancel_upload = UPLOAD_ERROR_X;
						continue;
					}
				}

				if (PG(upload_max_filesize) > 0 && (long)(total_bytes+blen) > PG(upload_max_filesize)) {
#if DEBUG_FILE_UPLOAD
					sapi_module.sapi_error(E_NOTICE, "upload_max_filesize of %ld bytes exceeded - file [%s=%s] not saved", PG(upload_max_filesize), param, filename);
#endif
					cancel_upload = UPLOAD_ERROR_A;
				} else if (max_file_size && ((long)(total_bytes+blen) > max_file_size)) {
#if DEBUG_FILE_UPLOAD
					sapi_module.sapi_error(E_NOTICE, "MAX_FILE_SIZE of %ld bytes exceeded - file [%s=%s] not saved", max_file_size, param, filename);
#endif
					cancel_upload = UPLOAD_ERROR_B;
				} else if (blen > 0) {
					wlen = write(fd, buff, blen);

					if (wlen == -1) {
						/* write failed */
#if DEBUG_FILE_UPLOAD
						sapi_module.sapi_error(E_NOTICE, "write() failed - %s", strerror(errno));
#endif
						cancel_upload = UPLOAD_ERROR_F;
					} else if (wlen < blen) {
#if DEBUG_FILE_UPLOAD
						sapi_module.sapi_error(E_NOTICE, "Only %d bytes were written, expected to write %d", wlen, blen);
#endif
						cancel_upload = UPLOAD_ERROR_F;
					} else {
						total_bytes += wlen;
					}
					offset += wlen;
				}

				/* read data for next iteration */
				blen = multipart_buffer_read(mbuff, buff, sizeof(buff), &end TSRMLS_CC);
			}

			if (fd != -1) { /* may not be initialized if file could not be created */
				close(fd);
			}

			if (!cancel_upload && !end) {
#if DEBUG_FILE_UPLOAD
				sapi_module.sapi_error(E_NOTICE, "Missing mime boundary at the end of the data for file %s", filename[0] != '\0' ? filename : "");
#endif
				cancel_upload = UPLOAD_ERROR_C;
			}
#if DEBUG_FILE_UPLOAD
			if (filename[0] != '\0' && total_bytes == 0 && !cancel_upload) {
				sapi_module.sapi_error(E_WARNING, "Uploaded file size 0 - file [%s=%s] not saved", param, filename);
				cancel_upload = 5;
			}
#endif
			if (php_rfc1867_callback != NULL) {
				multipart_event_file_end event_file_end;

				event_file_end.post_bytes_processed = SG(read_post_bytes);
				event_file_end.temp_filename = temp_filename;
				event_file_end.cancel_upload = cancel_upload;
				if (php_rfc1867_callback(MULTIPART_EVENT_FILE_END, &event_file_end, &event_extra_data TSRMLS_CC) == FAILURE) {
					cancel_upload = UPLOAD_ERROR_X;
				}
			}

			if (cancel_upload) {
				if (temp_filename) {
					if (cancel_upload != UPLOAD_ERROR_E) { /* file creation failed */
						unlink(temp_filename);
					}
					efree(temp_filename);
				}
				temp_filename = "";
			} else {
				zend_hash_add(SG(rfc1867_uploaded_files), temp_filename, strlen(temp_filename) + 1, &temp_filename, sizeof(char *), NULL);
			}

			/* is_arr_upload is true when name of file upload field
			 * ends in [.*]
			 * start_arr is set to point to 1st [ */
			is_arr_upload =	(start_arr = strchr(param,'[')) && (param[strlen(param)-1] == ']');

			if (is_arr_upload) {
				array_len = strlen(start_arr);
				if (array_index) {
					efree(array_index);
				}
				array_index = estrndup(start_arr + 1, array_len - 2);
			}

			/* Add $foo_name */
			if (llen < strlen(param) + MAX_SIZE_OF_INDEX + 1) {
				llen = strlen(param);
				lbuf = (char *) safe_erealloc(lbuf, llen, 1, MAX_SIZE_OF_INDEX + 1);
				llen += MAX_SIZE_OF_INDEX + 1;
			}

			if (is_arr_upload) {
				if (abuf) efree(abuf);
				abuf = estrndup(param, strlen(param)-array_len);
				snprintf(lbuf, llen, "%s_name[%s]", abuf, array_index);
			} else {
				snprintf(lbuf, llen, "%s_name", param);
			}

			/* The \ check should technically be needed for win32 systems only where
			 * it is a valid path separator. However, IE in all it's wisdom always sends
			 * the full path of the file on the user's filesystem, which means that unless
			 * the user does basename() they get a bogus file name. Until IE's user base drops
			 * to nill or problem is fixed this code must remain enabled for all systems. */
			s = _basename(internal_encoding, filename TSRMLS_CC);
			if (!s) {
				s = filename;
			}

			if (!is_anonymous) {
				safe_php_register_variable(lbuf, s, strlen(s), NULL, 0 TSRMLS_CC);
			}

			/* Add $foo[name] */
			if (is_arr_upload) {
				snprintf(lbuf, llen, "%s[name][%s]", abuf, array_index);
			} else {
				snprintf(lbuf, llen, "%s[name]", param);
			}
			register_http_post_files_variable(lbuf, s, http_post_files, 0 TSRMLS_CC);
			efree(filename);
			s = NULL;

			/* Possible Content-Type: */
			if (cancel_upload || !(cd = php_mime_get_hdr_value(header, "Content-Type"))) {
				cd = "";
			} else {
				/* fix for Opera 6.01 */
				s = strchr(cd, ';');
				if (s != NULL) {
					*s = '\0';
				}
			}

			/* Add $foo_type */
			if (is_arr_upload) {
				snprintf(lbuf, llen, "%s_type[%s]", abuf, array_index);
			} else {
				snprintf(lbuf, llen, "%s_type", param);
			}
			if (!is_anonymous) {
				safe_php_register_variable(lbuf, cd, strlen(cd), NULL, 0 TSRMLS_CC);
			}

			/* Add $foo[type] */
			if (is_arr_upload) {
				snprintf(lbuf, llen, "%s[type][%s]", abuf, array_index);
			} else {
				snprintf(lbuf, llen, "%s[type]", param);
			}
			register_http_post_files_variable(lbuf, cd, http_post_files, 0 TSRMLS_CC);

			/* Restore Content-Type Header */
			if (s != NULL) {
				*s = ';';
			}
			s = "";

			{
				/* store temp_filename as-is (in case upload_tmp_dir
				 * contains escapeable characters. escape only the variable name.) */
				zval zfilename;

				/* Initialize variables */
				add_protected_variable(param TSRMLS_CC);

				/* if param is of form xxx[.*] this will cut it to xxx */
				if (!is_anonymous) {
					ZVAL_STRING(&zfilename, temp_filename, 1);
					safe_php_register_variable_ex(param, &zfilename, NULL, 1 TSRMLS_CC);
				}

				/* Add $foo[tmp_name] */
				if (is_arr_upload) {
					snprintf(lbuf, llen, "%s[tmp_name][%s]", abuf, array_index);
				} else {
					snprintf(lbuf, llen, "%s[tmp_name]", param);
				}
				add_protected_variable(lbuf TSRMLS_CC);
				ZVAL_STRING(&zfilename, temp_filename, 1);
				register_http_post_files_variable_ex(lbuf, &zfilename, http_post_files, 1 TSRMLS_CC);
			}

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
				} else {
					snprintf(lbuf, llen, "%s[error]", param);
				}
				register_http_post_files_variable_ex(lbuf, &error_type, http_post_files, 0 TSRMLS_CC);

				/* Add $foo_size */
				if (is_arr_upload) {
					snprintf(lbuf, llen, "%s_size[%s]", abuf, array_index);
				} else {
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
					snprintf(lbuf, llen, "%s[size][%s]", abuf, array_index);
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

fileupload_done:
	if (php_rfc1867_callback != NULL) {
		multipart_event_end event_end;

		event_end.post_bytes_processed = SG(read_post_bytes);
		php_rfc1867_callback(MULTIPART_EVENT_END, &event_end, &event_extra_data TSRMLS_CC);
	}

	if (lbuf) efree(lbuf);
	if (abuf) efree(abuf);
	if (array_index) efree(array_index);
	zend_hash_destroy(&PG(rfc1867_protected_variables));
	zend_llist_destroy(&header);
	if (mbuff->boundary_next) efree(mbuff->boundary_next);
	if (mbuff->boundary) efree(mbuff->boundary);
	if (mbuff->buffer) efree(mbuff->buffer);
	if (mbuff) efree(mbuff);
}
/* }}} */

SAPI_API void php_rfc1867_set_multibyte_callbacks(
					php_rfc1867_encoding_translation_t encoding_translation,
					php_rfc1867_get_detect_order_t get_detect_order,
					php_rfc1867_set_input_encoding_t set_input_encoding,
					php_rfc1867_getword_t getword,
					php_rfc1867_getword_conf_t getword_conf,
					php_rfc1867_basename_t basename) /* {{{ */
{
	php_rfc1867_encoding_translation = encoding_translation;
	php_rfc1867_get_detect_order = get_detect_order;
	php_rfc1867_set_input_encoding = set_input_encoding;
	php_rfc1867_getword = getword;
	php_rfc1867_getword_conf = getword_conf;
	php_rfc1867_basename = basename;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
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
				} else {
					snprintf(lbuf, llen, "%s[error]", param);
				}
				register_http_post_files_variable_ex(lbuf, &error_type, http_post_files, 0 TSRMLS_CC);

				/* Add $foo_size */
				if (is_arr_upload) {
					snprintf(lbuf, llen, "%s_size[%s]", abuf, array_index);
				} else {
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
					snprintf(lbuf, llen, "%s[size][%s]", abuf, array_index);
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

fileupload_done:
	if (php_rfc1867_callback != NULL) {
		multipart_event_end event_end;

		event_end.post_bytes_processed = SG(read_post_bytes);
		php_rfc1867_callback(MULTIPART_EVENT_END, &event_end, &event_extra_data TSRMLS_CC);
	}

	if (lbuf) efree(lbuf);
	if (abuf) efree(abuf);
	if (array_index) efree(array_index);
	zend_hash_destroy(&PG(rfc1867_protected_variables));
	zend_llist_destroy(&header);
	if (mbuff->boundary_next) efree(mbuff->boundary_next);
	if (mbuff->boundary) efree(mbuff->boundary);
	if (mbuff->buffer) efree(mbuff->buffer);
	if (mbuff) efree(mbuff);
}
/* }}} */

SAPI_API void php_rfc1867_set_multibyte_callbacks(
					php_rfc1867_encoding_translation_t encoding_translation,
					php_rfc1867_get_detect_order_t get_detect_order,
					php_rfc1867_set_input_encoding_t set_input_encoding,
					php_rfc1867_getword_t getword,
					php_rfc1867_getword_conf_t getword_conf,
					php_rfc1867_basename_t basename) /* {{{ */
{
	php_rfc1867_encoding_translation = encoding_translation;
	php_rfc1867_get_detect_order = get_detect_order;
	php_rfc1867_set_input_encoding = set_input_encoding;
	php_rfc1867_getword = getword;
	php_rfc1867_getword_conf = getword_conf;
	php_rfc1867_basename = basename;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
				} else {
					snprintf(lbuf, llen, "%s_size", param);
				}
				if (!is_anonymous) {
					if (size_overflow) {
						ZVAL_STRING(&file_size, file_size_buf, 1);
					}
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

fileupload_done:
	if (php_rfc1867_callback != NULL) {
		multipart_event_end event_end;

		event_end.post_bytes_processed = SG(read_post_bytes);
		php_rfc1867_callback(MULTIPART_EVENT_END, &event_end, &event_extra_data TSRMLS_CC);
	}

	if (lbuf) efree(lbuf);
	if (abuf) efree(abuf);
	if (array_index) efree(array_index);
	zend_hash_destroy(&PG(rfc1867_protected_variables));
	zend_llist_destroy(&header);
	if (mbuff->boundary_next) efree(mbuff->boundary_next);
	if (mbuff->boundary) efree(mbuff->boundary);
	if (mbuff->buffer) efree(mbuff->buffer);
	if (mbuff) efree(mbuff);
}
/* }}} */

SAPI_API void php_rfc1867_set_multibyte_callbacks(
					php_rfc1867_encoding_translation_t encoding_translation,
					php_rfc1867_get_detect_order_t get_detect_order,
					php_rfc1867_set_input_encoding_t set_input_encoding,
					php_rfc1867_getword_t getword,
					php_rfc1867_getword_conf_t getword_conf,
					php_rfc1867_basename_t basename) /* {{{ */
{
	php_rfc1867_encoding_translation = encoding_translation;
	php_rfc1867_get_detect_order = get_detect_order;
	php_rfc1867_set_input_encoding = set_input_encoding;
	php_rfc1867_getword = getword;
	php_rfc1867_getword_conf = getword_conf;
	php_rfc1867_basename = basename;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */