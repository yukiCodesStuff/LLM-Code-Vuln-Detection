#define PHP_ICONV_MIME_DECODE_STRICT            (1<<0)
#define PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR (1<<1)

/* {{{ prototypes */
static php_iconv_err_t _php_iconv_appendl(smart_str *d, const char *s, size_t l, iconv_t cd);
static php_iconv_err_t _php_iconv_appendc(smart_str *d, const char c, iconv_t cd);

static void _php_iconv_show_error(php_iconv_err_t err, const char *out_charset, const char *in_charset TSRMLS_DC);
	{
		static char buf[16];
		snprintf(buf, sizeof(buf), "%d.%d",
		    ((_libiconv_version >> 8) & 0x0f), (_libiconv_version & 0x0f));
		version = buf;
	}
#elif HAVE_GLIBC_ICONV
	version = (char *)gnu_get_libc_version();

		if (mimetype != NULL && !(output_context->op & PHP_OUTPUT_HANDLER_CLEAN)) {
			int len;
			char *p = strstr(ICONVG(output_encoding), "//");

			if (p) {
				len = spprintf(&content_type, 0, "Content-Type:%.*s; charset=%.*s", mimetype_len ? mimetype_len : (int) strlen(mimetype), mimetype, (int)(p - ICONVG(output_encoding)), ICONVG(output_encoding));
			} else {

	return SUCCESS;
}

/* {{{ _php_iconv_appendl() */
static php_iconv_err_t _php_iconv_appendl(smart_str *d, const char *s, size_t l, iconv_t cd)
{
	const char *in_p = s;

			if (iconv(cd, (char **)&in_p, &in_left, (char **) &out_p, &out_left) == (size_t)-1) {
#if ICONV_SUPPORTS_ERRNO
				switch (errno) {
					case EINVAL:
						return PHP_ICONV_ERR_ILLEGAL_CHAR;

					case EILSEQ:
				}
#else
				if (prev_in_left == in_left) {
					return PHP_ICONV_ERR_UNKNOWN;
				}
#endif
			}
#if !ICONV_SUPPORTS_ERRNO
#else
				if (out_left != 0) {
					return PHP_ICONV_ERR_UNKNOWN;
				}
#endif
			}
			(d)->len += (buf_growth - out_left);
			buf_growth <<= 1;
	in_size = in_len;

	cd = iconv_open(out_charset, in_charset);

	if (cd == (iconv_t)(-1)) {
		return PHP_ICONV_ERR_UNKNOWN;
	}

	out_buffer = (char *) emalloc(out_size + 1);
	out_p = out_buffer;

#ifdef NETWARE
	result = iconv(cd, (char **) &in_p, &in_size, (char **)
#else
	result = iconv(cd, (const char **) &in_p, &in_size, (char **)
#endif
				&out_p, &out_left);

	if (result == (size_t)(-1)) {
		efree(out_buffer);
		return PHP_ICONV_ERR_UNKNOWN;
	}

	if (out_left < 8) {
		size_t pos = out_p - out_buffer;
		out_buffer = (char *) safe_erealloc(out_buffer, out_size, 1, 8);
		out_p = out_buffer+pos;
		out_size += 7;
		out_left += 7;
	}

	/* flush the shift-out sequences */
	result = iconv(cd, NULL, NULL, &out_p, &out_left);

	if (result == (size_t)(-1)) {
		efree(out_buffer);
		}
	}
	in_left= in_len;
	out_left = in_len + 32; /* Avoid realloc() most cases */
	out_size = 0;
	bsz = out_left;
	out_buf = (char *) emalloc(bsz+1);
	out_p = out_buf;

	while (in_left > 0) {
		result = iconv(cd, (char **) &in_p, &in_left, (char **) &out_p, &out_left);
				out_p = out_buf = tmp_buf;
				out_p += out_size;
				out_left = bsz - out_size;
				continue;
			}
		}
		break;
	}

	if (result != (size_t)(-1)) {
		/* flush the shift-out sequences */
		for (;;) {
		   	result = iconv(cd, NULL, NULL, (char **) &out_p, &out_left);
			out_size = bsz - out_left;

			if (errno == E2BIG) {
				bsz += 16;
				tmp_buf = (char *) erealloc(out_buf, bsz);

				out_p = out_buf = tmp_buf;
				out_p += out_size;
				out_left = bsz - out_size;
			} else {
	}

	if (out_left > 0) {
		cnt -= out_left / GENERIC_SUPERSET_NBYTES;
	}

#if ICONV_SUPPORTS_ERRNO
	switch (errno) {

	unsigned int cnt;
	int total_len;

	err = _php_iconv_strlen(&total_len, str, nbytes, enc);
	if (err != PHP_ICONV_ERR_SUCCESS) {
		return err;
	}

	if (len < 0) {
		if ((len += (total_len - offset)) < 0) {
			return PHP_ICONV_ERR_SUCCESS;
		}
		smart_str_0(pretval);
		return PHP_ICONV_ERR_SUCCESS;
	}

	cd1 = iconv_open(GENERIC_SUPERSET_NAME, enc);

	if (cd1 == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO

	if (cd2 != (iconv_t)NULL) {
		iconv_close(cd2);
	}
	return err;
}

/* }}} */
	if (ndl_buf) {
		efree(ndl_buf);
	}

	iconv_close(cd);

	return err;
}
	if ((fname_nbytes + 2) >= max_line_len
		|| (out_charset_len + 12) >= max_line_len) {
		/* field name is too long */
		err = PHP_ICONV_ERR_TOO_BIG;
		goto out;
	}

	cd_pl = iconv_open(ICONV_ASCII_ENCODING, enc);
	char_cnt -= 2;

	in_p = fval;
	in_left = fval_nbytes;

	do {
		size_t prev_in_left;
		size_t out_size;
			smart_str_appendl(pretval, lfchars, lfchars_len);
			smart_str_appendc(pretval, ' ');
			char_cnt = max_line_len - 1;
		}

		smart_str_appendl(pretval, "=?", sizeof("=?") - 1);
		char_cnt -= 2;
		smart_str_appendl(pretval, out_charset, out_charset_len);
		char_cnt -= out_charset_len;
									goto out;
								}
								break;

							default:
								err = PHP_ICONV_ERR_UNKNOWN;
								goto out;
						}
									goto out;
								}
								break;

							default:
								err = PHP_ICONV_ERR_UNKNOWN;
								goto out;
						}
	}
	if (encoded != NULL) {
		efree(encoded);
	}
	if (buf != NULL) {
		efree(buf);
	}
	return err;
	size_t str_left;
	unsigned int scan_stat = 0;
	const char *csname = NULL;
	size_t csname_len;
	const char *encoded_text = NULL;
	size_t encoded_text_len = 0;
	const char *encoded_word = NULL;
	const char *spaces = NULL;
						break;

					case '\n':
						scan_stat = 8;
						break;

					case '=': /* first letter of an encoded chunk */
						encoded_word = p1;

			case 1: /* expecting a delimiter */
				if (*p1 != '?') {
					err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
					if (err != PHP_ICONV_ERR_SUCCESS) {
						goto out;
					}
					encoded_word = NULL;
				csname = p1 + 1;
				scan_stat = 2;
				break;

			case 2: /* expecting a charset name */
				switch (*p1) {
					case '?': /* normal delimiter: encoding scheme follows */
						scan_stat = 3;
					case '*': /* new style delimiter: locale id follows */
						scan_stat = 10;
						break;
				}
				if (scan_stat != 2) {
					char tmpbuf[80];

					if (csname == NULL) {

					if (csname_len > sizeof(tmpbuf) - 1) {
						if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
							err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
							if (err != PHP_ICONV_ERR_SUCCESS) {
								goto out;
							}
							encoded_word = NULL;
								--str_left;
							}

							err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
							if (err != PHP_ICONV_ERR_SUCCESS) {
								goto out;
							}

						}
				}
				break;

			case 4: /* expecting a delimiter */
				if (*p1 != '?') {
					if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
						/* pass the entire chunk through the converter */
						err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
						if (err != PHP_ICONV_ERR_SUCCESS) {
							goto out;
						}
						encoded_word = NULL;
				if (*p1 != '=') {
					if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
						/* pass the entire chunk through the converter */
						err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
						if (err != PHP_ICONV_ERR_SUCCESS) {
							goto out;
						}
						encoded_word = NULL;
				switch (*p1) {
					default:
						/* Handle non-RFC-compliant formats
						 *
						 * RFC2047 requires the character that comes right
						 * after an encoded word (chunk) to be a whitespace,
						 * while there are lots of broken implementations that
						 * generate such malformed headers that don't fulfill
						 * that requirement.
						 */
						if (!eos) {
							if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
								/* pass the entire chunk through the converter */
								err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
								if (err != PHP_ICONV_ERR_SUCCESS) {
									goto out;
								}
								scan_stat = 12;
						if (decoded_text == NULL) {
							if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
								/* pass the entire chunk through the converter */
								err = _php_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
								if (err != PHP_ICONV_ERR_SUCCESS) {
									goto out;
								}
								encoded_word = NULL;
						if (err != PHP_ICONV_ERR_SUCCESS) {
							if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
								/* pass the entire chunk through the converter */
								err = _php_iconv_appendl(pretval, encoded_word, (size_t)(p1 - encoded_word), cd_pl);
								encoded_word = NULL;
								if (err != PHP_ICONV_ERR_SUCCESS) {
									break;
								}
	char *charset = ICONVG(internal_encoding);
	int charset_len = 0;
	char *str;
	int str_len;

	php_iconv_err_t err;

	unsigned int retval;
		RETURN_FALSE;
	}

	err = _php_iconv_strlen(&retval, str, str_len, charset);
	_php_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset TSRMLS_CC);
	if (err == PHP_ICONV_ERR_SUCCESS) {
		RETVAL_LONG(retval);
	} else {
	char *charset = ICONVG(internal_encoding);
	int charset_len = 0;
	char *str;
	int str_len;
	long offset, length = 0;

	php_iconv_err_t err;

	}

	if (ZEND_NUM_ARGS() < 3) {
		length = str_len;
	}

	err = _php_iconv_substr(&retval, str, str_len, offset, length, charset);
	_php_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset TSRMLS_CC);

	if (err == PHP_ICONV_ERR_SUCCESS && str != NULL && retval.c != NULL) {
		RETURN_STRINGL(retval.c, retval.len, 0);
	char *charset = ICONVG(internal_encoding);
	int charset_len = 0;
	char *haystk;
	int haystk_len;
	char *ndl;
	int ndl_len;
	long offset = 0;

	}

	err = _php_iconv_strpos(&retval, haystk, haystk_len, ndl, ndl_len,
	                        offset, charset);
	_php_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset TSRMLS_CC);

	if (err == PHP_ICONV_ERR_SUCCESS && retval != (unsigned int)-1) {
		RETVAL_LONG((long)retval);
	char *charset = ICONVG(internal_encoding);
	int charset_len = 0;
	char *haystk;
	int haystk_len;
	char *ndl;
	int ndl_len;

	php_iconv_err_t err;
	}

	err = _php_iconv_strpos(&retval, haystk, haystk_len, ndl, ndl_len,
	                        -1, charset);
	_php_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset TSRMLS_CC);

	if (err == PHP_ICONV_ERR_SUCCESS && retval != (unsigned int)-1) {
		RETVAL_LONG((long)retval);
	char *charset = ICONVG(internal_encoding);
	int charset_len = 0;
	long mode = 0;

	smart_str retval = {0};

	php_iconv_err_t err;

	char *charset = ICONVG(internal_encoding);
	int charset_len = 0;
	long mode = 0;

	php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ls",
		&encoded_str, &encoded_str_len, &mode, &charset, &charset_len) == FAILURE) {
					zend_hash_update(Z_ARRVAL_P(return_value), header_name, header_name_len, (void *)&new_elem, sizeof(new_elem), NULL);

					elem = &new_elem;
				}
				add_next_index_stringl(*elem, header_value, header_value_len, 1);
			} else {
				add_assoc_stringl_ex(return_value, header_name, header_name_len, header_value, header_value_len, 1);
			}
		}
		encoded_str_len -= next_pos - encoded_str;
		encoded_str = next_pos;

		smart_str_free(&decoded_header);
	}

	size_t out_len;
	int in_charset_len = 0, out_charset_len = 0, in_buffer_len;
	php_iconv_err_t err;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss",
		&in_charset, &in_charset_len, &out_charset, &out_charset_len, &in_buffer, &in_buffer_len) == FAILURE)
		return;


	err = php_iconv_string(in_buffer, (size_t)in_buffer_len,
		&out_buffer, &out_len, out_charset, in_charset);
	_php_iconv_show_error(err, out_charset, in_charset TSRMLS_CC);
	if (err == PHP_ICONV_ERR_SUCCESS && out_buffer != NULL) {
		RETVAL_STRINGL(out_buffer, out_len, 0);
	} else {
		if (out_buffer != NULL) {
		icnt = buf_len;
	}

	out_buf_size = ocnt = prev_ocnt = initial_out_buf_size;
	if (NULL == (out_buf = pemalloc(out_buf_size, persistent))) {
		return FAILURE;
	}
