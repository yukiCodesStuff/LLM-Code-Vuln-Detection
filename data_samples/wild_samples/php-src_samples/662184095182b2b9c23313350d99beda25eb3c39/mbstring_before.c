{
	const char *ps;
	size_t icnt;
	int state = 0;
	int crlf_state = -1;
	char *token = NULL;
	size_t token_pos;
	zend_string *fld_name, *fld_val;

	ps = str;
	icnt = str_len;
	fld_name = fld_val = NULL;

	/*
	 *             C o n t e n t - T y p e :   t e x t / h t m l \r\n
	 *             ^ ^^^^^^^^^^^^^^^^^^^^^ ^^^ ^^^^^^^^^^^^^^^^^ ^^^^
	 *      state  0            1           2          3
	 *
	 *             C o n t e n t - T y p e :   t e x t / h t m l \r\n
	 *             ^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^
	 * crlf_state -1                       0                     1 -1
	 *
	 */

	while (icnt > 0) {
		switch (*ps) {
			case ':':
				if (crlf_state == 1) {
					token_pos++;
				}

				if (state == 0 || state == 1) {
					if(token) {
						fld_name = zend_string_init(token, token_pos, 0);
					}
					state = 2;
				} else {
					token_pos++;
				}

				crlf_state = 0;
				break;

			case '\n':
				if (crlf_state == -1) {
					goto out;
				}
				crlf_state = -1;
				break;

			case '\r':
				if (crlf_state == 1) {
					token_pos++;
				} else {
					crlf_state = 1;
				}
				break;

			case ' ': case '\t':
				if (crlf_state == -1) {
					if (state == 3) {
						/* continuing from the previous line */
						state = 4;
					} else {
						/* simply skipping this new line */
						state = 5;
					}
				} else {
					if (crlf_state == 1) {
						token_pos++;
					}
					if (state == 1 || state == 3) {
						token_pos++;
					}
				}
				crlf_state = 0;
				break;

			default:
				switch (state) {
					case 0:
						token = (char*)ps;
						token_pos = 0;
						state = 1;
						break;

					case 2:
						if (crlf_state != -1) {
							token = (char*)ps;
							token_pos = 0;

							state = 3;
							break;
						}
						/* break is missing intentionally */

					case 3:
						if (crlf_state == -1) {
							if(token) {
								fld_val = zend_string_init(token, token_pos, 0);
							}

							if (fld_name != NULL && fld_val != NULL) {
								zval val;
								/* FIXME: some locale free implementation is
								 * really required here,,, */
								php_strtoupper(fld_name->val, fld_name->len);
								ZVAL_STR(&val, fld_val);

								zend_hash_update(ht, fld_name, &val);

								zend_string_release(fld_name);
							}

							fld_name = fld_val = NULL;
							token = (char*)ps;
							token_pos = 0;

							state = 1;
						}
						break;

					case 4:
						token_pos++;
						state = 3;
						break;
				}

				if (crlf_state == 1) {
					token_pos++;
				}

				token_pos++;

				crlf_state = 0;
				break;
		}
		ps++, icnt--;
	}
out:
	if (state == 2) {
		token = "";
		token_pos = 0;

		state = 3;
	}
	if (state == 3) {
		if(token) {
			fld_val = zend_string_init(token, token_pos, 0);
		}
		if (fld_name != NULL && fld_val != NULL) {
			zval val;
			/* FIXME: some locale free implementation is
			 * really required here,,, */
			php_strtoupper(fld_name->val, fld_name->len);
			ZVAL_STR(&val, fld_val);

			zend_hash_update(ht, fld_name, &val);

			zend_string_release(fld_name);
		}
	}
	return state;
}

PHP_FUNCTION(mb_send_mail)
{
	int n;
	char *to = NULL;
	size_t to_len;
	char *message = NULL;
	size_t message_len;
	char *headers = NULL;
	size_t headers_len;
	char *subject = NULL;
	zend_string *extra_cmd = NULL;
	size_t subject_len;
	int i;
	char *to_r = NULL;
	char *force_extra_parameters = INI_STR("mail.force_extra_parameters");
	struct {
		int cnt_type:1;
		int cnt_trans_enc:1;
	} suppressed_hdrs = { 0, 0 };

	char *message_buf = NULL, *subject_buf = NULL, *p;
	mbfl_string orig_str, conv_str;
	mbfl_string *pstr;	/* pointer to mbfl string for return value */
	enum mbfl_no_encoding
		tran_cs,	/* transfar text charset */
		head_enc,	/* header transfar encoding */
		body_enc;	/* body transfar encoding */
	mbfl_memory_device device;	/* automatic allocateable buffer for additional header */
	const mbfl_language *lang;
	int err = 0;
	HashTable ht_headers;
	zval *s;
	extern void mbfl_memory_device_unput(mbfl_memory_device *device);
	char *pp, *ee;

	/* initialize */
	mbfl_memory_device_init(&device, 0, 0);
	mbfl_string_init(&orig_str);
	mbfl_string_init(&conv_str);

	/* character-set, transfer-encoding */
	tran_cs = mbfl_no_encoding_utf8;
	head_enc = mbfl_no_encoding_base64;
	body_enc = mbfl_no_encoding_base64;
	lang = mbfl_no2language(MBSTRG(language));
	if (lang != NULL) {
		tran_cs = lang->mail_charset;
		head_enc = lang->mail_header_encoding;
		body_enc = lang->mail_body_encoding;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|sS", &to, &to_len, &subject, &subject_len, &message, &message_len, &headers, &headers_len, &extra_cmd) == FAILURE) {
		return;
	}

	/* ASCIIZ check */
	MAIL_ASCIIZ_CHECK_MBSTRING(to, to_len);
	MAIL_ASCIIZ_CHECK_MBSTRING(subject, subject_len);
	MAIL_ASCIIZ_CHECK_MBSTRING(message, message_len);
	if (headers) {
		MAIL_ASCIIZ_CHECK_MBSTRING(headers, headers_len);
	}
	if (extra_cmd) {
		MAIL_ASCIIZ_CHECK_MBSTRING(extra_cmd->val, extra_cmd->len);
	}

	zend_hash_init(&ht_headers, 0, NULL, ZVAL_PTR_DTOR, 0);

	if (headers != NULL) {
		_php_mbstr_parse_mail_headers(&ht_headers, headers, headers_len);
	}

	if ((s = zend_hash_str_find_ptr(&ht_headers, "CONTENT-TYPE", sizeof("CONTENT-TYPE") - 1))) {
		char *tmp;
		char *param_name;
		char *charset = NULL;

		p = strchr(Z_STRVAL_P(s), ';');

		if (p != NULL) {
			/* skipping the padded spaces */
			do {
				++p;
			} while (*p == ' ' || *p == '\t');

			if (*p != '\0') {
				if ((param_name = php_strtok_r(p, "= ", &tmp)) != NULL) {
					if (strcasecmp(param_name, "charset") == 0) {
						enum mbfl_no_encoding _tran_cs = tran_cs;

						charset = php_strtok_r(NULL, "= \"", &tmp);
						if (charset != NULL) {
							_tran_cs = mbfl_name2no_encoding(charset);
						}

						if (_tran_cs == mbfl_no_encoding_invalid) {
							php_error_docref(NULL, E_WARNING, "Unsupported charset \"%s\" - will be regarded as ascii", charset);
							_tran_cs = mbfl_no_encoding_ascii;
						}
						tran_cs = _tran_cs;
					}
				}
			}
		}
		suppressed_hdrs.cnt_type = 1;
	}

	if ((s = zend_hash_str_find_ptr(&ht_headers, "CONTENT-TRANSFER-ENCODING", sizeof("CONTENT-TRANSFER-ENCODING") - 1))) {
		enum mbfl_no_encoding _body_enc;

		_body_enc = mbfl_name2no_encoding(Z_STRVAL_P(s));
		switch (_body_enc) {
			case mbfl_no_encoding_base64:
			case mbfl_no_encoding_7bit:
			case mbfl_no_encoding_8bit:
				body_enc = _body_enc;
				break;

			default:
				php_error_docref(NULL, E_WARNING, "Unsupported transfer encoding \"%s\" - will be regarded as 8bit", Z_STRVAL_P(s));
				body_enc =	mbfl_no_encoding_8bit;
				break;
		}
		suppressed_hdrs.cnt_trans_enc = 1;
	}

	/* To: */
	if (to != NULL) {
		if (to_len > 0) {
			to_r = estrndup(to, to_len);
			for (; to_len; to_len--) {
				if (!isspace((unsigned char) to_r[to_len - 1])) {
					break;
				}
				to_r[to_len - 1] = '\0';
			}
			for (i = 0; to_r[i]; i++) {
			if (iscntrl((unsigned char) to_r[i])) {
				/* According to RFC 822, section 3.1.1 long headers may be separated into
				 * parts using CRLF followed at least one linear-white-space character ('\t' or ' ').
				 * To prevent these separators from being replaced with a space, we use the
				 * SKIP_LONG_HEADER_SEP_MBSTRING to skip over them.
				 */
				SKIP_LONG_HEADER_SEP_MBSTRING(to_r, i);
				to_r[i] = ' ';
			}
			}
		} else {
			to_r = to;
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Missing To: field");
		err = 1;
	}

	/* Subject: */
	if (subject != NULL) {
		orig_str.no_language = MBSTRG(language);
		orig_str.val = (unsigned char *)subject;
		orig_str.len = subject_len;
		orig_str.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;
		if (orig_str.no_encoding == mbfl_no_encoding_invalid || orig_str.no_encoding == mbfl_no_encoding_pass) {
			const mbfl_encoding *encoding = mbfl_identify_encoding2(&orig_str, MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
			orig_str.no_encoding = encoding ? encoding->no_encoding: mbfl_no_encoding_invalid;
		}
		pstr = mbfl_mime_header_encode(&orig_str, &conv_str, tran_cs, head_enc, "\n", sizeof("Subject: [PHP-jp nnnnnnnn]"));
		if (pstr != NULL) {
			subject_buf = subject = (char *)pstr->val;
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Missing Subject: field");
		err = 1;
	}

	/* message body */
	if (message != NULL) {
		orig_str.no_language = MBSTRG(language);
		orig_str.val = (unsigned char *)message;
		orig_str.len = (unsigned int)message_len;
		orig_str.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;

		if (orig_str.no_encoding == mbfl_no_encoding_invalid || orig_str.no_encoding == mbfl_no_encoding_pass) {
			const mbfl_encoding *encoding = mbfl_identify_encoding2(&orig_str, MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
			orig_str.no_encoding = encoding ? encoding->no_encoding: mbfl_no_encoding_invalid;
		}

		pstr = NULL;
		{
			mbfl_string tmpstr;

			if (mbfl_convert_encoding(&orig_str, &tmpstr, tran_cs) != NULL) {
				tmpstr.no_encoding=mbfl_no_encoding_8bit;
				pstr = mbfl_convert_encoding(&tmpstr, &conv_str, body_enc);
				efree(tmpstr.val);
			}
		}
		if (pstr != NULL) {
			message_buf = message = (char *)pstr->val;
		}
	} else {
		/* this is not really an error, so it is allowed. */
		php_error_docref(NULL, E_WARNING, "Empty message body");
		message = NULL;
	}

	/* other headers */
#define PHP_MBSTR_MAIL_MIME_HEADER1 "MIME-Version: 1.0"
#define PHP_MBSTR_MAIL_MIME_HEADER2 "Content-Type: text/plain"
#define PHP_MBSTR_MAIL_MIME_HEADER3 "; charset="
#define PHP_MBSTR_MAIL_MIME_HEADER4 "Content-Transfer-Encoding: "
	if (headers != NULL) {
		p = headers;
		n = headers_len;
		mbfl_memory_device_strncat(&device, p, n);
		if (n > 0 && p[n - 1] != '\n') {
			mbfl_memory_device_strncat(&device, "\n", 1);
		}
	}

	if (!zend_hash_str_exists(&ht_headers, "MIME-VERSION", sizeof("MIME-VERSION") - 1)) {
		mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER1, sizeof(PHP_MBSTR_MAIL_MIME_HEADER1) - 1);
		mbfl_memory_device_strncat(&device, "\n", 1);
	}

	if (!suppressed_hdrs.cnt_type) {
		mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER2, sizeof(PHP_MBSTR_MAIL_MIME_HEADER2) - 1);

		p = (char *)mbfl_no2preferred_mime_name(tran_cs);
		if (p != NULL) {
			mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER3, sizeof(PHP_MBSTR_MAIL_MIME_HEADER3) - 1);
			mbfl_memory_device_strcat(&device, p);
		}
		mbfl_memory_device_strncat(&device, "\n", 1);
	}
	if (!suppressed_hdrs.cnt_trans_enc) {
		mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER4, sizeof(PHP_MBSTR_MAIL_MIME_HEADER4) - 1);
		p = (char *)mbfl_no2preferred_mime_name(body_enc);
		if (p == NULL) {
			p = "7bit";
		}
		mbfl_memory_device_strcat(&device, p);
		mbfl_memory_device_strncat(&device, "\n", 1);
	}

	mbfl_memory_device_unput(&device);
	mbfl_memory_device_output('\0', &device);
	headers = (char *)device.buffer;

	if (force_extra_parameters) {
		extra_cmd = php_escape_shell_cmd(force_extra_parameters);
	} else if (extra_cmd) {
		extra_cmd = php_escape_shell_cmd(extra_cmd->val);
	}

	if (!err && php_mail(to_r, subject, message, headers, extra_cmd ? extra_cmd->val : NULL)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}

	if (extra_cmd) {
		zend_string_release(extra_cmd);
	}

	if (to_r != to) {
		efree(to_r);
	}
	if (subject_buf) {
		efree((void *)subject_buf);
	}
	if (message_buf) {
		efree((void *)message_buf);
	}
	mbfl_memory_device_clear(&device);
	zend_hash_destroy(&ht_headers);
}

#undef SKIP_LONG_HEADER_SEP_MBSTRING
#undef MAIL_ASCIIZ_CHECK_MBSTRING
#undef PHP_MBSTR_MAIL_MIME_HEADER1
#undef PHP_MBSTR_MAIL_MIME_HEADER2
#undef PHP_MBSTR_MAIL_MIME_HEADER3
#undef PHP_MBSTR_MAIL_MIME_HEADER4
/* }}} */

/* {{{ proto mixed mb_get_info([string type])
   Returns the current settings of mbstring */
PHP_FUNCTION(mb_get_info)
{
	char *typ = NULL;
	size_t typ_len;
	size_t n;
	char *name;
	const struct mb_overload_def *over_func;
	zval row1, row2;
	const mbfl_language *lang = mbfl_no2language(MBSTRG(language));
	const mbfl_encoding **entry;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &typ, &typ_len) == FAILURE) {
		return;
	}

	if (!typ || !strcasecmp("all", typ)) {
		array_init(return_value);
		if (MBSTRG(current_internal_encoding)) {
			add_assoc_string(return_value, "internal_encoding", (char *)MBSTRG(current_internal_encoding)->name);
		}
		if (MBSTRG(http_input_identify)) {
			add_assoc_string(return_value, "http_input", (char *)MBSTRG(http_input_identify)->name);
		}
		if (MBSTRG(current_http_output_encoding)) {
			add_assoc_string(return_value, "http_output", (char *)MBSTRG(current_http_output_encoding)->name);
		}
		if ((name = (char *)zend_ini_string("mbstring.http_output_conv_mimetypes", sizeof("mbstring.http_output_conv_mimetypes") - 1, 0)) != NULL) {
			add_assoc_string(return_value, "http_output_conv_mimetypes", name);
		}
		add_assoc_long(return_value, "func_overload", MBSTRG(func_overload));
		if (MBSTRG(func_overload)){
			over_func = &(mb_ovld[0]);
			array_init(&row1);
			while (over_func->type > 0) {
				if ((MBSTRG(func_overload) & over_func->type) == over_func->type ) {
					add_assoc_string(&row1, over_func->orig_func, over_func->ovld_func);
				}
				over_func++;
			}
			add_assoc_zval(return_value, "func_overload_list", &row1);
		} else {
			add_assoc_string(return_value, "func_overload_list", "no overload");
 		}
		if (lang != NULL) {
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_charset)) != NULL) {
				add_assoc_string(return_value, "mail_charset", name);
			}
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_header_encoding)) != NULL) {
				add_assoc_string(return_value, "mail_header_encoding", name);
			}
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_body_encoding)) != NULL) {
				add_assoc_string(return_value, "mail_body_encoding", name);
			}
		}
		add_assoc_long(return_value, "illegal_chars", MBSTRG(illegalchars));
		if (MBSTRG(encoding_translation)) {
			add_assoc_string(return_value, "encoding_translation", "On");
		} else {
			add_assoc_string(return_value, "encoding_translation", "Off");
		}
		if ((name = (char *)mbfl_no_language2name(MBSTRG(language))) != NULL) {
			add_assoc_string(return_value, "language", name);
		}
		n = MBSTRG(current_detect_order_list_size);
		entry = MBSTRG(current_detect_order_list);
		if (n > 0) {
			size_t i;
			array_init(&row2);
			for (i = 0; i < n; i++) {
				add_next_index_string(&row2, (*entry)->name);
				entry++;
			}
			add_assoc_zval(return_value, "detect_order", &row2);
		}
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			add_assoc_string(return_value, "substitute_character", "none");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			add_assoc_string(return_value, "substitute_character", "long");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			add_assoc_string(return_value, "substitute_character", "entity");
		} else {
			add_assoc_long(return_value, "substitute_character", MBSTRG(current_filter_illegal_substchar));
		}
		if (MBSTRG(strict_detection)) {
			add_assoc_string(return_value, "strict_detection", "On");
		} else {
			add_assoc_string(return_value, "strict_detection", "Off");
		}
	} else if (!strcasecmp("internal_encoding", typ)) {
		if (MBSTRG(current_internal_encoding)) {
			RETVAL_STRING((char *)MBSTRG(current_internal_encoding)->name);
		}
	} else if (!strcasecmp("http_input", typ)) {
		if (MBSTRG(http_input_identify)) {
			RETVAL_STRING((char *)MBSTRG(http_input_identify)->name);
		}
	} else if (!strcasecmp("http_output", typ)) {
		if (MBSTRG(current_http_output_encoding)) {
			RETVAL_STRING((char *)MBSTRG(current_http_output_encoding)->name);
		}
	} else if (!strcasecmp("http_output_conv_mimetypes", typ)) {
		if ((name = (char *)zend_ini_string("mbstring.http_output_conv_mimetypes", sizeof("mbstring.http_output_conv_mimetypes") - 1, 0)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("func_overload", typ)) {
 		RETVAL_LONG(MBSTRG(func_overload));
	} else if (!strcasecmp("func_overload_list", typ)) {
		if (MBSTRG(func_overload)){
				over_func = &(mb_ovld[0]);
				array_init(return_value);
				while (over_func->type > 0) {
					if ((MBSTRG(func_overload) & over_func->type) == over_func->type ) {
						add_assoc_string(return_value, over_func->orig_func, over_func->ovld_func);
					}
					over_func++;
				}
		} else {
			RETVAL_STRING("no overload");
		}
	} else if (!strcasecmp("mail_charset", typ)) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_charset)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("mail_header_encoding", typ)) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_header_encoding)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("mail_body_encoding", typ)) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_body_encoding)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("illegal_chars", typ)) {
		RETVAL_LONG(MBSTRG(illegalchars));
	} else if (!strcasecmp("encoding_translation", typ)) {
		if (MBSTRG(encoding_translation)) {
			RETVAL_STRING("On");
		} else {
			RETVAL_STRING("Off");
		}
	} else if (!strcasecmp("language", typ)) {
		if ((name = (char *)mbfl_no_language2name(MBSTRG(language))) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("detect_order", typ)) {
		n = MBSTRG(current_detect_order_list_size);
		entry = MBSTRG(current_detect_order_list);
		if (n > 0) {
			size_t i;
			array_init(return_value);
			for (i = 0; i < n; i++) {
				add_next_index_string(return_value, (*entry)->name);
				entry++;
			}
		}
	} else if (!strcasecmp("substitute_character", typ)) {
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			RETVAL_STRING("none");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			RETVAL_STRING("long");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			RETVAL_STRING("entity");
		} else {
			RETVAL_LONG(MBSTRG(current_filter_illegal_substchar));
		}
	} else if (!strcasecmp("strict_detection", typ)) {
		if (MBSTRG(strict_detection)) {
			RETVAL_STRING("On");
		} else {
			RETVAL_STRING("Off");
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool mb_check_encoding([string var[, string encoding]])
   Check if the string is valid for the specified encoding */
PHP_FUNCTION(mb_check_encoding)
{
	char *var = NULL;
	size_t var_len;
	char *enc = NULL;
	size_t enc_len;
	mbfl_buffer_converter *convd;
	const mbfl_encoding *encoding = MBSTRG(current_internal_encoding);
	mbfl_string string, result, *ret = NULL;
	long illegalchars = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ss", &var, &var_len, &enc, &enc_len) == FAILURE) {
		return;
	}

	if (var == NULL) {
		RETURN_BOOL(MBSTRG(illegalchars) == 0);
	}

	if (enc != NULL) {
		encoding = mbfl_name2encoding(enc);
		if (!encoding || encoding == &mbfl_encoding_pass) {
			php_error_docref(NULL, E_WARNING, "Invalid encoding \"%s\"", enc);
			RETURN_FALSE;
		}
	}

	convd = mbfl_buffer_converter_new2(encoding, encoding, 0);
	if (convd == NULL) {
		php_error_docref(NULL, E_WARNING, "Unable to create converter");
		RETURN_FALSE;
	}
	mbfl_buffer_converter_illegal_mode(convd, MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE);
	mbfl_buffer_converter_illegal_substchar(convd, 0);

	/* initialize string */
	mbfl_string_init_set(&string, mbfl_no_language_neutral, encoding->no_encoding);
	mbfl_string_init(&result);

	string.val = (unsigned char *)var;
	string.len = var_len;
	ret = mbfl_buffer_converter_feed_result(convd, &string, &result);
	illegalchars = mbfl_buffer_illegalchars(convd);
	mbfl_buffer_converter_delete(convd);

	RETVAL_FALSE;
	if (ret != NULL) {
		if (illegalchars == 0 && string.len == result.len && memcmp(string.val, result.val, string.len) == 0) {
			RETVAL_TRUE;
		}
		mbfl_string_clear(&result);
	}
}
/* }}} */

/* {{{ php_mb_populate_current_detect_order_list */
static void php_mb_populate_current_detect_order_list(void)
{
	const mbfl_encoding **entry = 0;
	size_t nentries;

	if (MBSTRG(current_detect_order_list)) {
		return;
	}

	if (MBSTRG(detect_order_list) && MBSTRG(detect_order_list_size)) {
		nentries = MBSTRG(detect_order_list_size);
		entry = (const mbfl_encoding **)safe_emalloc(nentries, sizeof(mbfl_encoding*), 0);
		memcpy(entry, MBSTRG(detect_order_list), sizeof(mbfl_encoding*) * nentries);
	} else {
		const enum mbfl_no_encoding *src = MBSTRG(default_detect_order_list);
		size_t i;
		nentries = MBSTRG(default_detect_order_list_size);
		entry = (const mbfl_encoding **)safe_emalloc(nentries, sizeof(mbfl_encoding*), 0);
		for (i = 0; i < nentries; i++) {
			entry[i] = mbfl_no2encoding(src[i]);
		}
	}
	MBSTRG(current_detect_order_list) = entry;
	MBSTRG(current_detect_order_list_size) = nentries;
}
/* }}} */

/* {{{ static int php_mb_encoding_translation() */
static int php_mb_encoding_translation(void)
{
	return MBSTRG(encoding_translation);
}
/* }}} */

/* {{{ MBSTRING_API size_t php_mb_mbchar_bytes_ex() */
MBSTRING_API size_t php_mb_mbchar_bytes_ex(const char *s, const mbfl_encoding *enc)
{
	if (enc != NULL) {
		if (enc->flag & MBFL_ENCTYPE_MBCS) {
			if (enc->mblen_table != NULL) {
				if (s != NULL) return enc->mblen_table[*(unsigned char *)s];
			}
		} else if (enc->flag & (MBFL_ENCTYPE_WCS2BE | MBFL_ENCTYPE_WCS2LE)) {
			return 2;
		} else if (enc->flag & (MBFL_ENCTYPE_WCS4BE | MBFL_ENCTYPE_WCS4LE)) {
			return 4;
		}
	}
	return 1;
}
/* }}} */

/* {{{ MBSTRING_API size_t php_mb_mbchar_bytes() */
MBSTRING_API size_t php_mb_mbchar_bytes(const char *s)
{
	return php_mb_mbchar_bytes_ex(s, MBSTRG(internal_encoding));
}
/* }}} */

/* {{{ MBSTRING_API char *php_mb_safe_strrchr_ex() */
MBSTRING_API char *php_mb_safe_strrchr_ex(const char *s, unsigned int c, size_t nbytes, const mbfl_encoding *enc)
{
	register const char *p = s;
	char *last=NULL;

	if (nbytes == (size_t)-1) {
		size_t nb = 0;

		while (*p != '\0') {
			if (nb == 0) {
				if ((unsigned char)*p == (unsigned char)c) {
					last = (char *)p;
				}
				nb = php_mb_mbchar_bytes_ex(p, enc);
				if (nb == 0) {
					return NULL; /* something is going wrong! */
				}
			}
			--nb;
			++p;
		}
	} else {
		register size_t bcnt = nbytes;
		register size_t nbytes_char;
		while (bcnt > 0) {
			if ((unsigned char)*p == (unsigned char)c) {
				last = (char *)p;
			}
			nbytes_char = php_mb_mbchar_bytes_ex(p, enc);
			if (bcnt < nbytes_char) {
				return NULL;
			}
			p += nbytes_char;
			bcnt -= nbytes_char;
		}
	}
	return last;
}
/* }}} */

/* {{{ MBSTRING_API char *php_mb_safe_strrchr() */
MBSTRING_API char *php_mb_safe_strrchr(const char *s, unsigned int c, size_t nbytes)
{
	return php_mb_safe_strrchr_ex(s, c, nbytes, MBSTRG(internal_encoding));
}
/* }}} */

/* {{{ MBSTRING_API int php_mb_stripos()
 */
MBSTRING_API int php_mb_stripos(int mode, const char *old_haystack, unsigned int old_haystack_len, const char *old_needle, unsigned int old_needle_len, long offset, const char *from_encoding)
{
	int n;
	mbfl_string haystack, needle;
	n = -1;

	mbfl_string_init(&haystack);
	mbfl_string_init(&needle);
	haystack.no_language = MBSTRG(language);
	haystack.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;
	needle.no_language = MBSTRG(language);
	needle.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;

	do {
		size_t len = 0;
		haystack.val = (unsigned char *)php_unicode_convert_case(PHP_UNICODE_CASE_UPPER, (char *)old_haystack, old_haystack_len, &len, from_encoding);
		haystack.len = len;

		if (!haystack.val) {
			break;
		}

		if (haystack.len <= 0) {
			break;
		}

		needle.val = (unsigned char *)php_unicode_convert_case(PHP_UNICODE_CASE_UPPER, (char *)old_needle, old_needle_len, &len, from_encoding);
		needle.len = len;

		if (!needle.val) {
			break;
		}

		if (needle.len <= 0) {
			break;
		}

		haystack.no_encoding = needle.no_encoding = mbfl_name2no_encoding(from_encoding);
		if (haystack.no_encoding == mbfl_no_encoding_invalid) {
			php_error_docref(NULL, E_WARNING, "Unknown encoding \"%s\"", from_encoding);
			break;
		}

 		{
 			int haystack_char_len = mbfl_strlen(&haystack);

 			if (mode) {
 				if ((offset > 0 && offset > haystack_char_len) ||
 					(offset < 0 && -offset > haystack_char_len)) {
 					php_error_docref(NULL, E_WARNING, "Offset is greater than the length of haystack string");
 					break;
 				}
 			} else {
 				if (offset < 0 || offset > haystack_char_len) {
 					php_error_docref(NULL, E_WARNING, "Offset not contained in string");
 					break;
 				}
 			}
		}

		n = mbfl_strpos(&haystack, &needle, offset, mode);
	} while(0);

	if (haystack.val) {
		efree(haystack.val);
	}

	if (needle.val) {
		efree(needle.val);
	}

	return n;
}
/* }}} */

static void php_mb_gpc_get_detect_order(const zend_encoding ***list, size_t *list_size) /* {{{ */
{
	*list = (const zend_encoding **)MBSTRG(http_input_list);
	*list_size = MBSTRG(http_input_list_size);
}
/* }}} */

static void php_mb_gpc_set_input_encoding(const zend_encoding *encoding) /* {{{ */
{
	MBSTRG(http_input_identify) = (const mbfl_encoding*)encoding;
}
/* }}} */

#endif	/* HAVE_MBSTRING */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
				if (crlf_state == 1) {
					token_pos++;
				}

				if (state == 0 || state == 1) {
					if(token) {
						fld_name = zend_string_init(token, token_pos, 0);
					}
					state = 2;
				} else {
					token_pos++;
				}

				crlf_state = 0;
				break;

			case '\n':
				if (crlf_state == -1) {
					goto out;
				}
				crlf_state = -1;
				break;

			case '\r':
				if (crlf_state == 1) {
					token_pos++;
				} else {
					crlf_state = 1;
				}
				break;

			case ' ': case '\t':
				if (crlf_state == -1) {
					if (state == 3) {
						/* continuing from the previous line */
						state = 4;
					} else {
						/* simply skipping this new line */
						state = 5;
					}
				} else {
					if (crlf_state == 1) {
						token_pos++;
					}
					if (state == 1 || state == 3) {
						token_pos++;
					}
				}
				crlf_state = 0;
				break;

			default:
				switch (state) {
					case 0:
						token = (char*)ps;
						token_pos = 0;
						state = 1;
						break;

					case 2:
						if (crlf_state != -1) {
							token = (char*)ps;
							token_pos = 0;

							state = 3;
							break;
						}
						/* break is missing intentionally */

					case 3:
						if (crlf_state == -1) {
							if(token) {
								fld_val = zend_string_init(token, token_pos, 0);
							}

							if (fld_name != NULL && fld_val != NULL) {
								zval val;
								/* FIXME: some locale free implementation is
								 * really required here,,, */
								php_strtoupper(fld_name->val, fld_name->len);
								ZVAL_STR(&val, fld_val);

								zend_hash_update(ht, fld_name, &val);

								zend_string_release(fld_name);
							}

							fld_name = fld_val = NULL;
							token = (char*)ps;
							token_pos = 0;

							state = 1;
						}
						break;

					case 4:
						token_pos++;
						state = 3;
						break;
				}

				if (crlf_state == 1) {
					token_pos++;
				}

				token_pos++;

				crlf_state = 0;
				break;
		}
		ps++, icnt--;
	}
out:
	if (state == 2) {
		token = "";
		token_pos = 0;

		state = 3;
	}
	if (state == 3) {
		if(token) {
			fld_val = zend_string_init(token, token_pos, 0);
		}
		if (fld_name != NULL && fld_val != NULL) {
			zval val;
			/* FIXME: some locale free implementation is
			 * really required here,,, */
			php_strtoupper(fld_name->val, fld_name->len);
			ZVAL_STR(&val, fld_val);

			zend_hash_update(ht, fld_name, &val);

			zend_string_release(fld_name);
		}
	}
	return state;
}

PHP_FUNCTION(mb_send_mail)
{
	int n;
	char *to = NULL;
	size_t to_len;
	char *message = NULL;
	size_t message_len;
	char *headers = NULL;
	size_t headers_len;
	char *subject = NULL;
	zend_string *extra_cmd = NULL;
	size_t subject_len;
	int i;
	char *to_r = NULL;
	char *force_extra_parameters = INI_STR("mail.force_extra_parameters");
	struct {
		int cnt_type:1;
		int cnt_trans_enc:1;
	} suppressed_hdrs = { 0, 0 };

	char *message_buf = NULL, *subject_buf = NULL, *p;
	mbfl_string orig_str, conv_str;
	mbfl_string *pstr;	/* pointer to mbfl string for return value */
	enum mbfl_no_encoding
		tran_cs,	/* transfar text charset */
		head_enc,	/* header transfar encoding */
		body_enc;	/* body transfar encoding */
	mbfl_memory_device device;	/* automatic allocateable buffer for additional header */
	const mbfl_language *lang;
	int err = 0;
	HashTable ht_headers;
	zval *s;
	extern void mbfl_memory_device_unput(mbfl_memory_device *device);
	char *pp, *ee;

	/* initialize */
	mbfl_memory_device_init(&device, 0, 0);
	mbfl_string_init(&orig_str);
	mbfl_string_init(&conv_str);

	/* character-set, transfer-encoding */
	tran_cs = mbfl_no_encoding_utf8;
	head_enc = mbfl_no_encoding_base64;
	body_enc = mbfl_no_encoding_base64;
	lang = mbfl_no2language(MBSTRG(language));
	if (lang != NULL) {
		tran_cs = lang->mail_charset;
		head_enc = lang->mail_header_encoding;
		body_enc = lang->mail_body_encoding;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|sS", &to, &to_len, &subject, &subject_len, &message, &message_len, &headers, &headers_len, &extra_cmd) == FAILURE) {
		return;
	}

	/* ASCIIZ check */
	MAIL_ASCIIZ_CHECK_MBSTRING(to, to_len);
	MAIL_ASCIIZ_CHECK_MBSTRING(subject, subject_len);
	MAIL_ASCIIZ_CHECK_MBSTRING(message, message_len);
	if (headers) {
		MAIL_ASCIIZ_CHECK_MBSTRING(headers, headers_len);
	}
	if (extra_cmd) {
		MAIL_ASCIIZ_CHECK_MBSTRING(extra_cmd->val, extra_cmd->len);
	}

	zend_hash_init(&ht_headers, 0, NULL, ZVAL_PTR_DTOR, 0);

	if (headers != NULL) {
		_php_mbstr_parse_mail_headers(&ht_headers, headers, headers_len);
	}

	if ((s = zend_hash_str_find_ptr(&ht_headers, "CONTENT-TYPE", sizeof("CONTENT-TYPE") - 1))) {
		char *tmp;
		char *param_name;
		char *charset = NULL;

		p = strchr(Z_STRVAL_P(s), ';');

		if (p != NULL) {
			/* skipping the padded spaces */
			do {
				++p;
			} while (*p == ' ' || *p == '\t');

			if (*p != '\0') {
				if ((param_name = php_strtok_r(p, "= ", &tmp)) != NULL) {
					if (strcasecmp(param_name, "charset") == 0) {
						enum mbfl_no_encoding _tran_cs = tran_cs;

						charset = php_strtok_r(NULL, "= \"", &tmp);
						if (charset != NULL) {
							_tran_cs = mbfl_name2no_encoding(charset);
						}

						if (_tran_cs == mbfl_no_encoding_invalid) {
							php_error_docref(NULL, E_WARNING, "Unsupported charset \"%s\" - will be regarded as ascii", charset);
							_tran_cs = mbfl_no_encoding_ascii;
						}
						tran_cs = _tran_cs;
					}
				}
			}
		}
		suppressed_hdrs.cnt_type = 1;
	}

	if ((s = zend_hash_str_find_ptr(&ht_headers, "CONTENT-TRANSFER-ENCODING", sizeof("CONTENT-TRANSFER-ENCODING") - 1))) {
		enum mbfl_no_encoding _body_enc;

		_body_enc = mbfl_name2no_encoding(Z_STRVAL_P(s));
		switch (_body_enc) {
			case mbfl_no_encoding_base64:
			case mbfl_no_encoding_7bit:
			case mbfl_no_encoding_8bit:
				body_enc = _body_enc;
				break;

			default:
				php_error_docref(NULL, E_WARNING, "Unsupported transfer encoding \"%s\" - will be regarded as 8bit", Z_STRVAL_P(s));
				body_enc =	mbfl_no_encoding_8bit;
				break;
		}
		suppressed_hdrs.cnt_trans_enc = 1;
	}

	/* To: */
	if (to != NULL) {
		if (to_len > 0) {
			to_r = estrndup(to, to_len);
			for (; to_len; to_len--) {
				if (!isspace((unsigned char) to_r[to_len - 1])) {
					break;
				}
				to_r[to_len - 1] = '\0';
			}
			for (i = 0; to_r[i]; i++) {
			if (iscntrl((unsigned char) to_r[i])) {
				/* According to RFC 822, section 3.1.1 long headers may be separated into
				 * parts using CRLF followed at least one linear-white-space character ('\t' or ' ').
				 * To prevent these separators from being replaced with a space, we use the
				 * SKIP_LONG_HEADER_SEP_MBSTRING to skip over them.
				 */
				SKIP_LONG_HEADER_SEP_MBSTRING(to_r, i);
				to_r[i] = ' ';
			}
			}
		} else {
			to_r = to;
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Missing To: field");
		err = 1;
	}

	/* Subject: */
	if (subject != NULL) {
		orig_str.no_language = MBSTRG(language);
		orig_str.val = (unsigned char *)subject;
		orig_str.len = subject_len;
		orig_str.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;
		if (orig_str.no_encoding == mbfl_no_encoding_invalid || orig_str.no_encoding == mbfl_no_encoding_pass) {
			const mbfl_encoding *encoding = mbfl_identify_encoding2(&orig_str, MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
			orig_str.no_encoding = encoding ? encoding->no_encoding: mbfl_no_encoding_invalid;
		}
		pstr = mbfl_mime_header_encode(&orig_str, &conv_str, tran_cs, head_enc, "\n", sizeof("Subject: [PHP-jp nnnnnnnn]"));
		if (pstr != NULL) {
			subject_buf = subject = (char *)pstr->val;
		}
	} else {
		php_error_docref(NULL, E_WARNING, "Missing Subject: field");
		err = 1;
	}

	/* message body */
	if (message != NULL) {
		orig_str.no_language = MBSTRG(language);
		orig_str.val = (unsigned char *)message;
		orig_str.len = (unsigned int)message_len;
		orig_str.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;

		if (orig_str.no_encoding == mbfl_no_encoding_invalid || orig_str.no_encoding == mbfl_no_encoding_pass) {
			const mbfl_encoding *encoding = mbfl_identify_encoding2(&orig_str, MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
			orig_str.no_encoding = encoding ? encoding->no_encoding: mbfl_no_encoding_invalid;
		}

		pstr = NULL;
		{
			mbfl_string tmpstr;

			if (mbfl_convert_encoding(&orig_str, &tmpstr, tran_cs) != NULL) {
				tmpstr.no_encoding=mbfl_no_encoding_8bit;
				pstr = mbfl_convert_encoding(&tmpstr, &conv_str, body_enc);
				efree(tmpstr.val);
			}
		}
		if (pstr != NULL) {
			message_buf = message = (char *)pstr->val;
		}
	} else {
		/* this is not really an error, so it is allowed. */
		php_error_docref(NULL, E_WARNING, "Empty message body");
		message = NULL;
	}

	/* other headers */
#define PHP_MBSTR_MAIL_MIME_HEADER1 "MIME-Version: 1.0"
#define PHP_MBSTR_MAIL_MIME_HEADER2 "Content-Type: text/plain"
#define PHP_MBSTR_MAIL_MIME_HEADER3 "; charset="
#define PHP_MBSTR_MAIL_MIME_HEADER4 "Content-Transfer-Encoding: "
	if (headers != NULL) {
		p = headers;
		n = headers_len;
		mbfl_memory_device_strncat(&device, p, n);
		if (n > 0 && p[n - 1] != '\n') {
			mbfl_memory_device_strncat(&device, "\n", 1);
		}
	}

	if (!zend_hash_str_exists(&ht_headers, "MIME-VERSION", sizeof("MIME-VERSION") - 1)) {
		mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER1, sizeof(PHP_MBSTR_MAIL_MIME_HEADER1) - 1);
		mbfl_memory_device_strncat(&device, "\n", 1);
	}

	if (!suppressed_hdrs.cnt_type) {
		mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER2, sizeof(PHP_MBSTR_MAIL_MIME_HEADER2) - 1);

		p = (char *)mbfl_no2preferred_mime_name(tran_cs);
		if (p != NULL) {
			mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER3, sizeof(PHP_MBSTR_MAIL_MIME_HEADER3) - 1);
			mbfl_memory_device_strcat(&device, p);
		}
		mbfl_memory_device_strncat(&device, "\n", 1);
	}
	if (!suppressed_hdrs.cnt_trans_enc) {
		mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER4, sizeof(PHP_MBSTR_MAIL_MIME_HEADER4) - 1);
		p = (char *)mbfl_no2preferred_mime_name(body_enc);
		if (p == NULL) {
			p = "7bit";
		}
		mbfl_memory_device_strcat(&device, p);
		mbfl_memory_device_strncat(&device, "\n", 1);
	}

	mbfl_memory_device_unput(&device);
	mbfl_memory_device_output('\0', &device);
	headers = (char *)device.buffer;

	if (force_extra_parameters) {
		extra_cmd = php_escape_shell_cmd(force_extra_parameters);
	} else if (extra_cmd) {
		extra_cmd = php_escape_shell_cmd(extra_cmd->val);
	}

	if (!err && php_mail(to_r, subject, message, headers, extra_cmd ? extra_cmd->val : NULL)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}

	if (extra_cmd) {
		zend_string_release(extra_cmd);
	}

	if (to_r != to) {
		efree(to_r);
	}
	if (subject_buf) {
		efree((void *)subject_buf);
	}
	if (message_buf) {
		efree((void *)message_buf);
	}
	mbfl_memory_device_clear(&device);
	zend_hash_destroy(&ht_headers);
}

#undef SKIP_LONG_HEADER_SEP_MBSTRING
#undef MAIL_ASCIIZ_CHECK_MBSTRING
#undef PHP_MBSTR_MAIL_MIME_HEADER1
#undef PHP_MBSTR_MAIL_MIME_HEADER2
#undef PHP_MBSTR_MAIL_MIME_HEADER3
#undef PHP_MBSTR_MAIL_MIME_HEADER4
/* }}} */

/* {{{ proto mixed mb_get_info([string type])
   Returns the current settings of mbstring */
PHP_FUNCTION(mb_get_info)
{
	char *typ = NULL;
	size_t typ_len;
	size_t n;
	char *name;
	const struct mb_overload_def *over_func;
	zval row1, row2;
	const mbfl_language *lang = mbfl_no2language(MBSTRG(language));
	const mbfl_encoding **entry;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &typ, &typ_len) == FAILURE) {
		return;
	}

	if (!typ || !strcasecmp("all", typ)) {
		array_init(return_value);
		if (MBSTRG(current_internal_encoding)) {
			add_assoc_string(return_value, "internal_encoding", (char *)MBSTRG(current_internal_encoding)->name);
		}
		if (MBSTRG(http_input_identify)) {
			add_assoc_string(return_value, "http_input", (char *)MBSTRG(http_input_identify)->name);
		}
		if (MBSTRG(current_http_output_encoding)) {
			add_assoc_string(return_value, "http_output", (char *)MBSTRG(current_http_output_encoding)->name);
		}
		if ((name = (char *)zend_ini_string("mbstring.http_output_conv_mimetypes", sizeof("mbstring.http_output_conv_mimetypes") - 1, 0)) != NULL) {
			add_assoc_string(return_value, "http_output_conv_mimetypes", name);
		}
		add_assoc_long(return_value, "func_overload", MBSTRG(func_overload));
		if (MBSTRG(func_overload)){
			over_func = &(mb_ovld[0]);
			array_init(&row1);
			while (over_func->type > 0) {
				if ((MBSTRG(func_overload) & over_func->type) == over_func->type ) {
					add_assoc_string(&row1, over_func->orig_func, over_func->ovld_func);
				}
				over_func++;
			}
			add_assoc_zval(return_value, "func_overload_list", &row1);
		} else {
			add_assoc_string(return_value, "func_overload_list", "no overload");
 		}
		if (lang != NULL) {
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_charset)) != NULL) {
				add_assoc_string(return_value, "mail_charset", name);
			}
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_header_encoding)) != NULL) {
				add_assoc_string(return_value, "mail_header_encoding", name);
			}
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_body_encoding)) != NULL) {
				add_assoc_string(return_value, "mail_body_encoding", name);
			}
		}
		add_assoc_long(return_value, "illegal_chars", MBSTRG(illegalchars));
		if (MBSTRG(encoding_translation)) {
			add_assoc_string(return_value, "encoding_translation", "On");
		} else {
			add_assoc_string(return_value, "encoding_translation", "Off");
		}
		if ((name = (char *)mbfl_no_language2name(MBSTRG(language))) != NULL) {
			add_assoc_string(return_value, "language", name);
		}
		n = MBSTRG(current_detect_order_list_size);
		entry = MBSTRG(current_detect_order_list);
		if (n > 0) {
			size_t i;
			array_init(&row2);
			for (i = 0; i < n; i++) {
				add_next_index_string(&row2, (*entry)->name);
				entry++;
			}
			add_assoc_zval(return_value, "detect_order", &row2);
		}
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			add_assoc_string(return_value, "substitute_character", "none");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			add_assoc_string(return_value, "substitute_character", "long");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			add_assoc_string(return_value, "substitute_character", "entity");
		} else {
			add_assoc_long(return_value, "substitute_character", MBSTRG(current_filter_illegal_substchar));
		}
		if (MBSTRG(strict_detection)) {
			add_assoc_string(return_value, "strict_detection", "On");
		} else {
			add_assoc_string(return_value, "strict_detection", "Off");
		}
	} else if (!strcasecmp("internal_encoding", typ)) {
		if (MBSTRG(current_internal_encoding)) {
			RETVAL_STRING((char *)MBSTRG(current_internal_encoding)->name);
		}
	} else if (!strcasecmp("http_input", typ)) {
		if (MBSTRG(http_input_identify)) {
			RETVAL_STRING((char *)MBSTRG(http_input_identify)->name);
		}
	} else if (!strcasecmp("http_output", typ)) {
		if (MBSTRG(current_http_output_encoding)) {
			RETVAL_STRING((char *)MBSTRG(current_http_output_encoding)->name);
		}
	} else if (!strcasecmp("http_output_conv_mimetypes", typ)) {
		if ((name = (char *)zend_ini_string("mbstring.http_output_conv_mimetypes", sizeof("mbstring.http_output_conv_mimetypes") - 1, 0)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("func_overload", typ)) {
 		RETVAL_LONG(MBSTRG(func_overload));
	} else if (!strcasecmp("func_overload_list", typ)) {
		if (MBSTRG(func_overload)){
				over_func = &(mb_ovld[0]);
				array_init(return_value);
				while (over_func->type > 0) {
					if ((MBSTRG(func_overload) & over_func->type) == over_func->type ) {
						add_assoc_string(return_value, over_func->orig_func, over_func->ovld_func);
					}
					over_func++;
				}
		} else {
			RETVAL_STRING("no overload");
		}
	} else if (!strcasecmp("mail_charset", typ)) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_charset)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("mail_header_encoding", typ)) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_header_encoding)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("mail_body_encoding", typ)) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_body_encoding)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("illegal_chars", typ)) {
		RETVAL_LONG(MBSTRG(illegalchars));
	} else if (!strcasecmp("encoding_translation", typ)) {
		if (MBSTRG(encoding_translation)) {
			RETVAL_STRING("On");
		} else {
			RETVAL_STRING("Off");
		}
	} else if (!strcasecmp("language", typ)) {
		if ((name = (char *)mbfl_no_language2name(MBSTRG(language))) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (!strcasecmp("detect_order", typ)) {
		n = MBSTRG(current_detect_order_list_size);
		entry = MBSTRG(current_detect_order_list);
		if (n > 0) {
			size_t i;
			array_init(return_value);
			for (i = 0; i < n; i++) {
				add_next_index_string(return_value, (*entry)->name);
				entry++;
			}
		}
	} else if (!strcasecmp("substitute_character", typ)) {
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			RETVAL_STRING("none");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			RETVAL_STRING("long");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			RETVAL_STRING("entity");
		} else {
			RETVAL_LONG(MBSTRG(current_filter_illegal_substchar));
		}
	} else if (!strcasecmp("strict_detection", typ)) {
		if (MBSTRG(strict_detection)) {
			RETVAL_STRING("On");
		} else {
			RETVAL_STRING("Off");
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool mb_check_encoding([string var[, string encoding]])
   Check if the string is valid for the specified encoding */
PHP_FUNCTION(mb_check_encoding)
{
	char *var = NULL;
	size_t var_len;
	char *enc = NULL;
	size_t enc_len;
	mbfl_buffer_converter *convd;
	const mbfl_encoding *encoding = MBSTRG(current_internal_encoding);
	mbfl_string string, result, *ret = NULL;
	long illegalchars = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ss", &var, &var_len, &enc, &enc_len) == FAILURE) {
		return;
	}

	if (var == NULL) {
		RETURN_BOOL(MBSTRG(illegalchars) == 0);
	}

	if (enc != NULL) {
		encoding = mbfl_name2encoding(enc);
		if (!encoding || encoding == &mbfl_encoding_pass) {
			php_error_docref(NULL, E_WARNING, "Invalid encoding \"%s\"", enc);
			RETURN_FALSE;
		}
	}

	convd = mbfl_buffer_converter_new2(encoding, encoding, 0);
	if (convd == NULL) {
		php_error_docref(NULL, E_WARNING, "Unable to create converter");
		RETURN_FALSE;
	}
	mbfl_buffer_converter_illegal_mode(convd, MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE);
	mbfl_buffer_converter_illegal_substchar(convd, 0);

	/* initialize string */
	mbfl_string_init_set(&string, mbfl_no_language_neutral, encoding->no_encoding);
	mbfl_string_init(&result);

	string.val = (unsigned char *)var;
	string.len = var_len;
	ret = mbfl_buffer_converter_feed_result(convd, &string, &result);
	illegalchars = mbfl_buffer_illegalchars(convd);
	mbfl_buffer_converter_delete(convd);

	RETVAL_FALSE;
	if (ret != NULL) {
		if (illegalchars == 0 && string.len == result.len && memcmp(string.val, result.val, string.len) == 0) {
			RETVAL_TRUE;
		}
		mbfl_string_clear(&result);
	}
}
/* }}} */

/* {{{ php_mb_populate_current_detect_order_list */
static void php_mb_populate_current_detect_order_list(void)
{
	const mbfl_encoding **entry = 0;
	size_t nentries;

	if (MBSTRG(current_detect_order_list)) {
		return;
	}

	if (MBSTRG(detect_order_list) && MBSTRG(detect_order_list_size)) {
		nentries = MBSTRG(detect_order_list_size);
		entry = (const mbfl_encoding **)safe_emalloc(nentries, sizeof(mbfl_encoding*), 0);
		memcpy(entry, MBSTRG(detect_order_list), sizeof(mbfl_encoding*) * nentries);
	} else {
		const enum mbfl_no_encoding *src = MBSTRG(default_detect_order_list);
		size_t i;
		nentries = MBSTRG(default_detect_order_list_size);
		entry = (const mbfl_encoding **)safe_emalloc(nentries, sizeof(mbfl_encoding*), 0);
		for (i = 0; i < nentries; i++) {
			entry[i] = mbfl_no2encoding(src[i]);
		}
	}
	MBSTRG(current_detect_order_list) = entry;
	MBSTRG(current_detect_order_list_size) = nentries;
}
/* }}} */

/* {{{ static int php_mb_encoding_translation() */
static int php_mb_encoding_translation(void)
{
	return MBSTRG(encoding_translation);
}
/* }}} */

/* {{{ MBSTRING_API size_t php_mb_mbchar_bytes_ex() */
MBSTRING_API size_t php_mb_mbchar_bytes_ex(const char *s, const mbfl_encoding *enc)
{
	if (enc != NULL) {
		if (enc->flag & MBFL_ENCTYPE_MBCS) {
			if (enc->mblen_table != NULL) {
				if (s != NULL) return enc->mblen_table[*(unsigned char *)s];
			}
		} else if (enc->flag & (MBFL_ENCTYPE_WCS2BE | MBFL_ENCTYPE_WCS2LE)) {
			return 2;
		} else if (enc->flag & (MBFL_ENCTYPE_WCS4BE | MBFL_ENCTYPE_WCS4LE)) {
			return 4;
		}
	}
	return 1;
}
/* }}} */

/* {{{ MBSTRING_API size_t php_mb_mbchar_bytes() */
MBSTRING_API size_t php_mb_mbchar_bytes(const char *s)
{
	return php_mb_mbchar_bytes_ex(s, MBSTRG(internal_encoding));
}
/* }}} */

/* {{{ MBSTRING_API char *php_mb_safe_strrchr_ex() */
MBSTRING_API char *php_mb_safe_strrchr_ex(const char *s, unsigned int c, size_t nbytes, const mbfl_encoding *enc)
{
	register const char *p = s;
	char *last=NULL;

	if (nbytes == (size_t)-1) {
		size_t nb = 0;

		while (*p != '\0') {
			if (nb == 0) {
				if ((unsigned char)*p == (unsigned char)c) {
					last = (char *)p;
				}
				nb = php_mb_mbchar_bytes_ex(p, enc);
				if (nb == 0) {
					return NULL; /* something is going wrong! */
				}
			}
			--nb;
			++p;
		}
	} else {
		register size_t bcnt = nbytes;
		register size_t nbytes_char;
		while (bcnt > 0) {
			if ((unsigned char)*p == (unsigned char)c) {
				last = (char *)p;
			}
			nbytes_char = php_mb_mbchar_bytes_ex(p, enc);
			if (bcnt < nbytes_char) {
				return NULL;
			}
			p += nbytes_char;
			bcnt -= nbytes_char;
		}
	}
	return last;
}
/* }}} */

/* {{{ MBSTRING_API char *php_mb_safe_strrchr() */
MBSTRING_API char *php_mb_safe_strrchr(const char *s, unsigned int c, size_t nbytes)
{
	return php_mb_safe_strrchr_ex(s, c, nbytes, MBSTRG(internal_encoding));
}
/* }}} */

/* {{{ MBSTRING_API int php_mb_stripos()
 */
MBSTRING_API int php_mb_stripos(int mode, const char *old_haystack, unsigned int old_haystack_len, const char *old_needle, unsigned int old_needle_len, long offset, const char *from_encoding)
{
	int n;
	mbfl_string haystack, needle;
	n = -1;

	mbfl_string_init(&haystack);
	mbfl_string_init(&needle);
	haystack.no_language = MBSTRG(language);
	haystack.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;
	needle.no_language = MBSTRG(language);
	needle.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;

	do {
		size_t len = 0;
		haystack.val = (unsigned char *)php_unicode_convert_case(PHP_UNICODE_CASE_UPPER, (char *)old_haystack, old_haystack_len, &len, from_encoding);
		haystack.len = len;

		if (!haystack.val) {
			break;
		}

		if (haystack.len <= 0) {
			break;
		}

		needle.val = (unsigned char *)php_unicode_convert_case(PHP_UNICODE_CASE_UPPER, (char *)old_needle, old_needle_len, &len, from_encoding);
		needle.len = len;

		if (!needle.val) {
			break;
		}

		if (needle.len <= 0) {
			break;
		}

		haystack.no_encoding = needle.no_encoding = mbfl_name2no_encoding(from_encoding);
		if (haystack.no_encoding == mbfl_no_encoding_invalid) {
			php_error_docref(NULL, E_WARNING, "Unknown encoding \"%s\"", from_encoding);
			break;
		}

 		{
 			int haystack_char_len = mbfl_strlen(&haystack);

 			if (mode) {
 				if ((offset > 0 && offset > haystack_char_len) ||
 					(offset < 0 && -offset > haystack_char_len)) {
 					php_error_docref(NULL, E_WARNING, "Offset is greater than the length of haystack string");
 					break;
 				}
 			} else {
 				if (offset < 0 || offset > haystack_char_len) {
 					php_error_docref(NULL, E_WARNING, "Offset not contained in string");
 					break;
 				}
 			}
		}

		n = mbfl_strpos(&haystack, &needle, offset, mode);
	} while(0);

	if (haystack.val) {
		efree(haystack.val);
	}

	if (needle.val) {
		efree(needle.val);
	}

	return n;
}
/* }}} */

static void php_mb_gpc_get_detect_order(const zend_encoding ***list, size_t *list_size) /* {{{ */
{
	*list = (const zend_encoding **)MBSTRG(http_input_list);
	*list_size = MBSTRG(http_input_list_size);
}
/* }}} */

static void php_mb_gpc_set_input_encoding(const zend_encoding *encoding) /* {{{ */
{
	MBSTRG(http_input_identify) = (const mbfl_encoding*)encoding;
}
/* }}} */

#endif	/* HAVE_MBSTRING */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
						if (crlf_state != -1) {
							token = (char*)ps;
							token_pos = 0;

							state = 3;
							break;
						}
						/* break is missing intentionally */

					case 3:
						if (crlf_state == -1) {
							if(token) {
								fld_val = zend_string_init(token, token_pos, 0);
							}

							if (fld_name != NULL && fld_val != NULL) {
								zval val;
								/* FIXME: some locale free implementation is
								 * really required here,,, */
								php_strtoupper(fld_name->val, fld_name->len);
								ZVAL_STR(&val, fld_val);

								zend_hash_update(ht, fld_name, &val);

								zend_string_release(fld_name);
							}

							fld_name = fld_val = NULL;
							token = (char*)ps;
							token_pos = 0;

							state = 1;
						}
	if (state == 2) {
		token = "";
		token_pos = 0;

		state = 3;
	}
	if (state == 3) {
		if(token) {
			fld_val = zend_string_init(token, token_pos, 0);
		}