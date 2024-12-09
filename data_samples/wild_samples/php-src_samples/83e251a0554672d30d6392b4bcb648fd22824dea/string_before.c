{
	int sum;
	int pos1, pos2, max;

	php_similar_str(txt1, len1, txt2, len2, &pos1, &pos2, &max);
	if ((sum = max)) {
		if (pos1 && pos2) {
			sum += php_similar_char(txt1, pos1,
									txt2, pos2);
		}
		if ((pos1 + max < len1) && (pos2 + max < len2)) {
			sum += php_similar_char(txt1 + pos1 + max, len1 - pos1 - max,
									txt2 + pos2 + max, len2 - pos2 - max);
		}
	}

	return sum;
}
{
	char *tbuf, *buf, *p, *tp, *rp, c, lc;
	int br, i=0, depth=0, in_q = 0;
	int state = 0, pos;
	char *allow_free;

	if (stateptr)
		state = *stateptr;

	buf = estrndup(rbuf, len);
	c = *buf;
	lc = '\0';
	p = buf;
	rp = rbuf;
	br = 0;
	if (allow) {
		if (IS_INTERNED(allow)) {
			allow_free = allow = zend_str_tolower_dup(allow, allow_len);
		} else {
			allow_free = NULL;
			php_strtolower(allow, allow_len);
		}
		tbuf = emalloc(PHP_TAG_BUF_SIZE + 1);
		tp = tbuf;
	} else {
		tbuf = tp = NULL;
	}

	while (i < len) {
		switch (c) {
			case '\0':
				break;
			case '<':
				if (in_q) {
					break;
				}
				if (isspace(*(p + 1)) && !allow_tag_spaces) {
					goto reg_char;
				}
				if (state == 0) {
					lc = '<';
					state = 1;
					if (allow) {
						if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
							pos = tp - tbuf;
							tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
							tp = tbuf + pos;
						}
						*(tp++) = '<';
				 	}
				} else if (state == 1) {
					depth++;
				}
				break;

			case '(':
				if (state == 2) {
					if (lc != '"' && lc != '\'') {
						lc = '(';
						br++;
					}
				} else if (allow && state == 1) {
					if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
						pos = tp - tbuf;
						tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
						tp = tbuf + pos;
					}
					*(tp++) = c;
				} else if (state == 0) {
					*(rp++) = c;
				}
				break;

			case ')':
				if (state == 2) {
					if (lc != '"' && lc != '\'') {
						lc = ')';
						br--;
					}
				} else if (allow && state == 1) {
					if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
						pos = tp - tbuf;
						tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
						tp = tbuf + pos;
					}
					*(tp++) = c;
				} else if (state == 0) {
					*(rp++) = c;
				}
				break;

			case '>':
				if (depth) {
					depth--;
					break;
				}

				if (in_q) {
					break;
				}

				switch (state) {
					case 1: /* HTML/XML */
						lc = '>';
						in_q = state = 0;
						if (allow) {
							if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
								pos = tp - tbuf;
								tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
								tp = tbuf + pos;
							}
							*(tp++) = '>';
							*tp='\0';
							if (php_tag_find(tbuf, tp-tbuf, allow)) {
								memcpy(rp, tbuf, tp-tbuf);
								rp += tp-tbuf;
							}
							tp = tbuf;
						}
						break;

					case 2: /* PHP */
						if (!br && lc != '\"' && *(p-1) == '?') {
							in_q = state = 0;
							tp = tbuf;
						}
						break;

					case 3:
						in_q = state = 0;
						tp = tbuf;
						break;

					case 4: /* JavaScript/CSS/etc... */
						if (p >= buf + 2 && *(p-1) == '-' && *(p-2) == '-') {
							in_q = state = 0;
							tp = tbuf;
						}
						break;

					default:
						*(rp++) = c;
						break;
				}
				break;

			case '"':
			case '\'':
				if (state == 4) {
					/* Inside <!-- comment --> */
					break;
				} else if (state == 2 && *(p-1) != '\\') {
					if (lc == c) {
						lc = '\0';
					} else if (lc != '\\') {
						lc = c;
					}
				} else if (state == 0) {
					*(rp++) = c;
				} else if (allow && state == 1) {
					if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
						pos = tp - tbuf;
						tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
						tp = tbuf + pos;
					}
					*(tp++) = c;
				}
				if (state && p != buf && (state == 1 || *(p-1) != '\\') && (!in_q || *p == in_q)) {
					if (in_q) {
						in_q = 0;
					} else {
						in_q = *p;
					}
				}
				break;

			case '!':
				/* JavaScript & Other HTML scripting languages */
				if (state == 1 && *(p-1) == '<') {
					state = 3;
					lc = c;
				} else {
					if (state == 0) {
						*(rp++) = c;
					} else if (allow && state == 1) {
						if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
							pos = tp - tbuf;
							tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
							tp = tbuf + pos;
						}
						*(tp++) = c;
					}
				}
				break;

			case '-':
				if (state == 3 && p >= buf + 2 && *(p-1) == '-' && *(p-2) == '!') {
					state = 4;
				} else {
					goto reg_char;
				}
				break;

			case '?':

				if (state == 1 && *(p-1) == '<') {
					br=0;
					state=2;
					break;
				}

			case 'E':
			case 'e':
				/* !DOCTYPE exception */
				if (state==3 && p > buf+6
						     && tolower(*(p-1)) == 'p'
					         && tolower(*(p-2)) == 'y'
						     && tolower(*(p-3)) == 't'
						     && tolower(*(p-4)) == 'c'
						     && tolower(*(p-5)) == 'o'
						     && tolower(*(p-6)) == 'd') {
					state = 1;
					break;
				}
				/* fall-through */

			case 'l':
			case 'L':

				/* swm: If we encounter '<?xml' then we shouldn't be in
				 * state == 2 (PHP). Switch back to HTML.
				 */

				if (state == 2 && p > buf+2 && strncasecmp(p-2, "xm", 2) == 0) {
					state = 1;
					break;
				}

				/* fall-through */
			default:
reg_char:
				if (state == 0) {
					*(rp++) = c;
				} else if (allow && state == 1) {
					if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
						pos = tp - tbuf;
						tbuf = erealloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
						tp = tbuf + pos;
					}
					*(tp++) = c;
				}
				break;
		}
		c = *(++p);
		i++;
	}
	if (rp < rbuf + len) {
		*rp = '\0';
	}
	efree(buf);
	if (allow) {
		efree(tbuf);
		if (allow_free) {
			efree(allow_free);
		}
	}
	if (stateptr)
		*stateptr = state;

	return (size_t)(rp - rbuf);
}
/* }}} */

/* {{{ proto array str_getcsv(string input[, string delimiter[, string enclosure[, string escape]]])
Parse a CSV string into an array */
PHP_FUNCTION(str_getcsv)
{
	char *str, delim = ',', enc = '"', esc = '\\';
	char *delim_str = NULL, *enc_str = NULL, *esc_str = NULL;
	int str_len = 0, delim_len = 0, enc_len = 0, esc_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sss", &str, &str_len, &delim_str, &delim_len,
		&enc_str, &enc_len, &esc_str, &esc_len) == FAILURE) {
		return;
	}

	delim = delim_len ? delim_str[0] : delim;
	enc = enc_len ? enc_str[0] : enc;
	esc = esc_len ? esc_str[0] : esc;

	php_fgetcsv(NULL, delim, enc, esc, str_len, str, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string str_repeat(string input, int mult)
   Returns the input string repeat mult times */
PHP_FUNCTION(str_repeat)
{
	char		*input_str;		/* Input string */
	int 		input_len;
	long 		mult;			/* Multiplier */
	char		*result;		/* Resulting string */
	size_t		result_len;		/* Length of the resulting string */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &input_str, &input_len, &mult) == FAILURE) {
		return;
	}

	if (mult < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Second argument has to be greater than or equal to 0");
		return;
	}

	/* Don't waste our time if it's empty */
	/* ... or if the multiplier is zero */
	if (input_len == 0 || mult == 0)
		RETURN_EMPTY_STRING();

	/* Initialize the result string */
	result_len = input_len * mult;
	result = (char *)safe_emalloc(input_len, mult, 1);

	/* Heavy optimization for situations where input string is 1 byte long */
	if (input_len == 1) {
		memset(result, *(input_str), mult);
	} else {
		char *s, *e, *ee;
		int l=0;
		memcpy(result, input_str, input_len);
		s = result;
		e = result + input_len;
		ee = result + result_len;

		while (e<ee) {
			l = (e-s) < (ee-e) ? (e-s) : (ee-e);
			memmove(e, s, l);
			e += l;
		}
	}

	result[result_len] = '\0';

	RETURN_STRINGL(result, result_len, 0);
}
/* }}} */

/* {{{ proto mixed count_chars(string input [, int mode])
   Returns info about what characters are used in input */
PHP_FUNCTION(count_chars)
{
	char *input;
	int chars[256];
	long mymode=0;
	unsigned char *buf;
	int len, inx;
	char retstr[256];
	int retlen=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &input, &len, &mymode) == FAILURE) {
		return;
	}

	if (mymode < 0 || mymode > 4) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown mode");
		RETURN_FALSE;
	}

	buf = (unsigned char *) input;
	memset((void*) chars, 0, sizeof(chars));

	while (len > 0) {
		chars[*buf]++;
		buf++;
		len--;
	}

	if (mymode < 3) {
		array_init(return_value);
	}

	for (inx = 0; inx < 256; inx++) {
		switch (mymode) {
	 		case 0:
				add_index_long(return_value, inx, chars[inx]);
				break;
	 		case 1:
				if (chars[inx] != 0) {
					add_index_long(return_value, inx, chars[inx]);
				}
				break;
  			case 2:
				if (chars[inx] == 0) {
					add_index_long(return_value, inx, chars[inx]);
				}
				break;
	  		case 3:
				if (chars[inx] != 0) {
					retstr[retlen++] = inx;
				}
				break;
  			case 4:
				if (chars[inx] == 0) {
					retstr[retlen++] = inx;
				}
				break;
		}
	}

	if (mymode >= 3 && mymode <= 4) {
		RETURN_STRINGL(retstr, retlen, 1);
	}
}
/* }}} */

/* {{{ php_strnatcmp
 */
static void php_strnatcmp(INTERNAL_FUNCTION_PARAMETERS, int fold_case)
{
	char *s1, *s2;
	int s1_len, s2_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &s1, &s1_len, &s2, &s2_len) == FAILURE) {
		return;
	}

	RETURN_LONG(strnatcmp_ex(s1, s1_len,
							 s2, s2_len,
							 fold_case));
}
/* }}} */

PHPAPI int string_natural_compare_function_ex(zval *result, zval *op1, zval *op2, zend_bool case_insensitive TSRMLS_DC) /* {{{ */
{
	zval op1_copy, op2_copy;
	int use_copy1 = 0, use_copy2 = 0;

	if (Z_TYPE_P(op1) != IS_STRING) {
		zend_make_printable_zval(op1, &op1_copy, &use_copy1);
	}
	if (Z_TYPE_P(op2) != IS_STRING) {
		zend_make_printable_zval(op2, &op2_copy, &use_copy2);
	}

	if (use_copy1) {
		op1 = &op1_copy;
	}
	if (use_copy2) {
		op2 = &op2_copy;
	}

	ZVAL_LONG(result, strnatcmp_ex(Z_STRVAL_P(op1), Z_STRLEN_P(op1), Z_STRVAL_P(op2), Z_STRLEN_P(op2), case_insensitive));

	if (use_copy1) {
		zval_dtor(op1);
	}
	if (use_copy2) {
		zval_dtor(op2);
	}
	return SUCCESS;
}
/* }}} */

PHPAPI int string_natural_case_compare_function(zval *result, zval *op1, zval *op2 TSRMLS_DC) /* {{{ */
{
	return string_natural_compare_function_ex(result, op1, op2, 1 TSRMLS_CC);
}
/* }}} */

PHPAPI int string_natural_compare_function(zval *result, zval *op1, zval *op2 TSRMLS_DC) /* {{{ */
{
	return string_natural_compare_function_ex(result, op1, op2, 0 TSRMLS_CC);
}
/* }}} */

/* {{{ proto int strnatcmp(string s1, string s2)
   Returns the result of string comparison using 'natural' algorithm */
PHP_FUNCTION(strnatcmp)
{
	php_strnatcmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto array localeconv(void)
   Returns numeric formatting information based on the current locale */
PHP_FUNCTION(localeconv)
{
	zval *grouping, *mon_grouping;
	int len, i;

	/* We don't need no stinkin' parameters... */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(grouping);
	MAKE_STD_ZVAL(mon_grouping);

	array_init(return_value);
	array_init(grouping);
	array_init(mon_grouping);

#ifdef HAVE_LOCALECONV
	{
		struct lconv currlocdata;

		localeconv_r( &currlocdata );

		/* Grab the grouping data out of the array */
		len = strlen(currlocdata.grouping);

		for (i = 0; i < len; i++) {
			add_index_long(grouping, i, currlocdata.grouping[i]);
		}

		/* Grab the monetary grouping data out of the array */
		len = strlen(currlocdata.mon_grouping);

		for (i = 0; i < len; i++) {
			add_index_long(mon_grouping, i, currlocdata.mon_grouping[i]);
		}

		add_assoc_string(return_value, "decimal_point",     currlocdata.decimal_point,     1);
		add_assoc_string(return_value, "thousands_sep",     currlocdata.thousands_sep,     1);
		add_assoc_string(return_value, "int_curr_symbol",   currlocdata.int_curr_symbol,   1);
		add_assoc_string(return_value, "currency_symbol",   currlocdata.currency_symbol,   1);
		add_assoc_string(return_value, "mon_decimal_point", currlocdata.mon_decimal_point, 1);
		add_assoc_string(return_value, "mon_thousands_sep", currlocdata.mon_thousands_sep, 1);
		add_assoc_string(return_value, "positive_sign",     currlocdata.positive_sign,     1);
		add_assoc_string(return_value, "negative_sign",     currlocdata.negative_sign,     1);
		add_assoc_long(  return_value, "int_frac_digits",   currlocdata.int_frac_digits     );
		add_assoc_long(  return_value, "frac_digits",       currlocdata.frac_digits         );
		add_assoc_long(  return_value, "p_cs_precedes",     currlocdata.p_cs_precedes       );
		add_assoc_long(  return_value, "p_sep_by_space",    currlocdata.p_sep_by_space      );
		add_assoc_long(  return_value, "n_cs_precedes",     currlocdata.n_cs_precedes       );
		add_assoc_long(  return_value, "n_sep_by_space",    currlocdata.n_sep_by_space      );
		add_assoc_long(  return_value, "p_sign_posn",       currlocdata.p_sign_posn         );
		add_assoc_long(  return_value, "n_sign_posn",       currlocdata.n_sign_posn         );
	}
#else
	/* Ok, it doesn't look like we have locale info floating around, so I guess it
	   wouldn't hurt to just go ahead and return the POSIX locale information?  */

	add_index_long(grouping, 0, -1);
	add_index_long(mon_grouping, 0, -1);

	add_assoc_string(return_value, "decimal_point",     "\x2E", 1);
	add_assoc_string(return_value, "thousands_sep",     "",     1);
	add_assoc_string(return_value, "int_curr_symbol",   "",     1);
	add_assoc_string(return_value, "currency_symbol",   "",     1);
	add_assoc_string(return_value, "mon_decimal_point", "\x2E", 1);
	add_assoc_string(return_value, "mon_thousands_sep", "",     1);
	add_assoc_string(return_value, "positive_sign",     "",     1);
	add_assoc_string(return_value, "negative_sign",     "",     1);
	add_assoc_long(  return_value, "int_frac_digits",   CHAR_MAX );
	add_assoc_long(  return_value, "frac_digits",       CHAR_MAX );
	add_assoc_long(  return_value, "p_cs_precedes",     CHAR_MAX );
	add_assoc_long(  return_value, "p_sep_by_space",    CHAR_MAX );
	add_assoc_long(  return_value, "n_cs_precedes",     CHAR_MAX );
	add_assoc_long(  return_value, "n_sep_by_space",    CHAR_MAX );
	add_assoc_long(  return_value, "p_sign_posn",       CHAR_MAX );
	add_assoc_long(  return_value, "n_sign_posn",       CHAR_MAX );
#endif

	zend_hash_update(Z_ARRVAL_P(return_value), "grouping", 9, &grouping, sizeof(zval *), NULL);
	zend_hash_update(Z_ARRVAL_P(return_value), "mon_grouping", 13, &mon_grouping, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ proto int strnatcasecmp(string s1, string s2)
   Returns the result of case-insensitive string comparison using 'natural' algorithm */
PHP_FUNCTION(strnatcasecmp)
{
	php_strnatcmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto int substr_count(string haystack, string needle [, int offset [, int length]])
   Returns the number of times a substring occurs in the string */
PHP_FUNCTION(substr_count)
{
	char *haystack, *needle;
	long offset = 0, length = 0;
	int ac = ZEND_NUM_ARGS();
	int count = 0;
	int haystack_len, needle_len;
	char *p, *endp, cmp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|ll", &haystack, &haystack_len, &needle, &needle_len, &offset, &length) == FAILURE) {
		return;
	}

	if (needle_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty substring");
		RETURN_FALSE;
	}

	p = haystack;
	endp = p + haystack_len;

	if (offset < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Offset should be greater than or equal to 0");
		RETURN_FALSE;
	}

	if (offset > haystack_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Offset value %ld exceeds string length", offset);
		RETURN_FALSE;
	}
	p += offset;

	if (ac == 4) {

		if (length <= 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length should be greater than 0");
			RETURN_FALSE;
		}
		if (length > (haystack_len - offset)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length value %ld exceeds string length", length);
			RETURN_FALSE;
		}
		endp = p + length;
	}

	if (needle_len == 1) {
		cmp = needle[0];

		while ((p = memchr(p, cmp, endp - p))) {
			count++;
			p++;
		}
	} else {
		while ((p = php_memnstr(p, needle, needle_len, endp))) {
			p += needle_len;
			count++;
		}
	}

	RETURN_LONG(count);
}
/* }}} */

/* {{{ proto string str_pad(string input, int pad_length [, string pad_string [, int pad_type]])
   Returns input string padded on the left or right to specified length with pad_string */
PHP_FUNCTION(str_pad)
{
	/* Input arguments */
	char *input;				/* Input string */
	int  input_len;
	long pad_length;			/* Length to pad to */

	/* Helper variables */
	size_t	   num_pad_chars;		/* Number of padding characters (total - input size) */
	char  *result = NULL;		/* Resulting string */
	int	   result_len = 0;		/* Length of the resulting string */
	char  *pad_str_val = " ";	/* Pointer to padding string */
	int    pad_str_len = 1;		/* Length of the padding string */
	long   pad_type_val = STR_PAD_RIGHT; /* The padding type value */
	int	   i, left_pad=0, right_pad=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|sl", &input, &input_len, &pad_length,
																  &pad_str_val, &pad_str_len, &pad_type_val) == FAILURE) {
		return;
	}

	/* If resulting string turns out to be shorter than input string,
	   we simply copy the input and return. */
	if (pad_length <= 0 || (pad_length - input_len) <= 0) {
		RETURN_STRINGL(input, input_len, 1);
	}

	if (pad_str_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Padding string cannot be empty");
		return;
	}

	if (pad_type_val < STR_PAD_LEFT || pad_type_val > STR_PAD_BOTH) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Padding type has to be STR_PAD_LEFT, STR_PAD_RIGHT, or STR_PAD_BOTH");
		return;
	}

	num_pad_chars = pad_length - input_len;
	if (num_pad_chars >= INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Padding length is too long");
		return;
	}
	result = (char *)emalloc(input_len + num_pad_chars + 1);

	/* We need to figure out the left/right padding lengths. */
	switch (pad_type_val) {
		case STR_PAD_RIGHT:
			left_pad = 0;
			right_pad = num_pad_chars;
			break;

		case STR_PAD_LEFT:
			left_pad = num_pad_chars;
			right_pad = 0;
			break;

		case STR_PAD_BOTH:
			left_pad = num_pad_chars / 2;
			right_pad = num_pad_chars - left_pad;
			break;
	}

	/* First we pad on the left. */
	for (i = 0; i < left_pad; i++)
		result[result_len++] = pad_str_val[i % pad_str_len];

	/* Then we copy the input string. */
	memcpy(result + result_len, input, input_len);
	result_len += input_len;

	/* Finally, we pad on the right. */
	for (i = 0; i < right_pad; i++)
		result[result_len++] = pad_str_val[i % pad_str_len];

	result[result_len] = '\0';

	RETURN_STRINGL(result, result_len, 0);
}
/* }}} */

/* {{{ proto mixed sscanf(string str, string format [, string ...])
   Implements an ANSI C compatible sscanf */
PHP_FUNCTION(sscanf)
{
	zval ***args = NULL;
	char *str, *format;
	int str_len, format_len, result, num_args = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss*", &str, &str_len, &format, &format_len,
		&args, &num_args) == FAILURE) {
		return;
	}

	result = php_sscanf_internal(str, format, num_args, args, 0, &return_value TSRMLS_CC);

	if (args) {
		efree(args);
	}

	if (SCAN_ERROR_WRONG_PARAM_COUNT == result) {
		WRONG_PARAM_COUNT;
	}
}
/* }}} */

static char rot13_from[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char rot13_to[] = "nopqrstuvwxyzabcdefghijklmNOPQRSTUVWXYZABCDEFGHIJKLM";

/* {{{ proto string str_rot13(string str)
   Perform the rot13 transform on a string */
PHP_FUNCTION(str_rot13)
{
	char *arg;
	int arglen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arglen) == FAILURE) {
		return;
	}

	RETVAL_STRINGL(arg, arglen, 1);

	php_strtr(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value), rot13_from, rot13_to, 52);
}
/* }}} */

static void php_string_shuffle(char *str, long len TSRMLS_DC) /* {{{ */
{
	long n_elems, rnd_idx, n_left;
	char temp;
	/* The implementation is stolen from array_data_shuffle       */
	/* Thus the characteristics of the randomization are the same */
	n_elems = len;

	if (n_elems <= 1) {
		return;
	}

	n_left = n_elems;

	while (--n_left) {
		rnd_idx = php_rand(TSRMLS_C);
		RAND_RANGE(rnd_idx, 0, n_left, PHP_RAND_MAX);
		if (rnd_idx != n_left) {
			temp = str[n_left];
			str[n_left] = str[rnd_idx];
			str[rnd_idx] = temp;
		}
	}
}
/* }}} */

/* {{{ proto void str_shuffle(string str)
   Shuffles string. One permutation of all possible is created */
PHP_FUNCTION(str_shuffle)
{
	char *arg;
	int arglen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arglen) == FAILURE) {
		return;
	}

	RETVAL_STRINGL(arg, arglen, 1);
	if (Z_STRLEN_P(return_value) > 1) {
		php_string_shuffle(Z_STRVAL_P(return_value), (long) Z_STRLEN_P(return_value) TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto mixed str_word_count(string str, [int format [, string charlist]])
   	Counts the number of words inside a string. If format of 1 is specified,
   	then the function will return an array containing all the words
   	found inside the string. If format of 2 is specified, then the function
   	will return an associated array where the position of the word is the key
   	and the word itself is the value.

   	For the purpose of this function, 'word' is defined as a locale dependent
   	string containing alphabetic characters, which also may contain, but not start
   	with "'" and "-" characters.
*/
PHP_FUNCTION(str_word_count)
{
	char *buf, *str, *char_list = NULL, *p, *e, *s, ch[256];
	int str_len, char_list_len = 0, word_count = 0;
	long type = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ls", &str, &str_len, &type, &char_list, &char_list_len) == FAILURE) {
		return;
	}

	switch(type) {
		case 1:
		case 2:
			array_init(return_value);
			if (!str_len) {
				return;
			}
			break;
		case 0:
			if (!str_len) {
				RETURN_LONG(0);
			}
			/* nothing to be done */
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid format value %ld", type);
			RETURN_FALSE;
	}

	if (char_list) {
		php_charmask((unsigned char *)char_list, char_list_len, ch TSRMLS_CC);
	}

	p = str;
	e = str + str_len;

	/* first character cannot be ' or -, unless explicitly allowed by the user */
	if ((*p == '\'' && (!char_list || !ch['\''])) || (*p == '-' && (!char_list || !ch['-']))) {
		p++;
	}
	/* last character cannot be -, unless explicitly allowed by the user */
	if (*(e - 1) == '-' && (!char_list || !ch['-'])) {
		e--;
	}

	while (p < e) {
		s = p;
		while (p < e && (isalpha((unsigned char)*p) || (char_list && ch[(unsigned char)*p]) || *p == '\'' || *p == '-')) {
			p++;
		}
		if (p > s) {
			switch (type)
			{
				case 1:
					buf = estrndup(s, (p-s));
					add_next_index_stringl(return_value, buf, (p-s), 0);
					break;
				case 2:
					buf = estrndup(s, (p-s));
					add_index_stringl(return_value, (s - str), buf, p-s, 0);
					break;
				default:
					word_count++;
					break;
			}
		}
		p++;
	}

	if (!type) {
		RETURN_LONG(word_count);
	}
}

/* }}} */

#if HAVE_STRFMON
/* {{{ proto string money_format(string format , float value)
   Convert monetary value(s) to string */
PHP_FUNCTION(money_format)
{
	int format_len = 0, str_len;
	char *format, *str, *p, *e;
	double value;
	zend_bool check = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sd", &format, &format_len, &value) == FAILURE) {
		return;
	}

	p = format;
	e = p + format_len;
	while ((p = memchr(p, '%', (e - p)))) {
		if (*(p + 1) == '%') {
			p += 2;
		} else if (!check) {
			check = 1;
			p++;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Only a single %%i or %%n token can be used");
			RETURN_FALSE;
		}
	}

	str_len = format_len + 1024;
	str = emalloc(str_len);
	if ((str_len = strfmon(str, str_len, format, value)) < 0) {
		efree(str);
		RETURN_FALSE;
	}
	str[str_len] = 0;

	RETURN_STRINGL(erealloc(str, str_len + 1), str_len, 0);
}
/* }}} */
#endif

/* {{{ proto array str_split(string str [, int split_length])
   Convert a string to an array. If split_length is specified, break the string down into chunks each split_length characters long. */
PHP_FUNCTION(str_split)
{
	char *str;
	int str_len;
	long split_length = 1;
	char *p;
	int n_reg_segments;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &str, &str_len, &split_length) == FAILURE) {
		return;
	}

	if (split_length <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The length of each segment must be greater than zero");
		RETURN_FALSE;
	}

	array_init_size(return_value, ((str_len - 1) / split_length) + 1);

	if (split_length >= str_len) {
		add_next_index_stringl(return_value, str, str_len, 1);
		return;
	}

	n_reg_segments = str_len / split_length;
	p = str;

	while (n_reg_segments-- > 0) {
		add_next_index_stringl(return_value, p, split_length, 1);
		p += split_length;
	}

	if (p != (str + str_len)) {
		add_next_index_stringl(return_value, p, (str + str_len - p), 1);
	}
}
/* }}} */

/* {{{ proto array strpbrk(string haystack, string char_list)
   Search a string for any of a set of characters */
PHP_FUNCTION(strpbrk)
{
	char *haystack, *char_list;
	int haystack_len, char_list_len;
	char *haystack_ptr, *cl_ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &haystack, &haystack_len, &char_list, &char_list_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (!char_list_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The character list cannot be empty");
		RETURN_FALSE;
	}

	for (haystack_ptr = haystack; haystack_ptr < (haystack + haystack_len); ++haystack_ptr) {
		for (cl_ptr = char_list; cl_ptr < (char_list + char_list_len); ++cl_ptr) {
			if (*cl_ptr == *haystack_ptr) {
				RETURN_STRINGL(haystack_ptr, (haystack + haystack_len - haystack_ptr), 1);
			}
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int substr_compare(string main_str, string str, int offset [, int length [, bool case_sensitivity]])
   Binary safe optionally case insensitive comparison of 2 strings from an offset, up to length characters */
PHP_FUNCTION(substr_compare)
{
	char *s1, *s2;
	int s1_len, s2_len;
	long offset, len=0;
	zend_bool cs=0;
	uint cmp_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl|lb", &s1, &s1_len, &s2, &s2_len, &offset, &len, &cs) == FAILURE) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() >= 4 && len <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The length must be greater than zero");
		RETURN_FALSE;
	}

	if (offset < 0) {
		offset = s1_len + offset;
		offset = (offset < 0) ? 0 : offset;
	}

	if (offset >= s1_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The start position cannot exceed initial string length");
		RETURN_FALSE;
	}

	cmp_len = (uint) (len ? len : MAX(s2_len, (s1_len - offset)));

	if (!cs) {
		RETURN_LONG(zend_binary_strncmp(s1 + offset, (s1_len - offset), s2, s2_len, cmp_len));
	} else {
		RETURN_LONG(zend_binary_strncasecmp_l(s1 + offset, (s1_len - offset), s2, s2_len, cmp_len));
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */