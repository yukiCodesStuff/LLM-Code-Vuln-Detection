{
	unsigned long lp = 0;
	unsigned char c, *ret, *d;
	char *hex = "0123456789ABCDEF";

	ret = safe_emalloc(1, 3 * length + 3 * (((3 * length)/PHP_QPRINT_MAXL) + 1), 0);
	d = ret;

	while (length--) {
		if (((c = *str++) == '\015') && (*str == '\012') && length > 0) {
			*d++ = '\015';
			*d++ = *str++;
			length--;
			lp = 0;
		} else {
			if (iscntrl (c) || (c == 0x7f) || (c & 0x80) || (c == '=') || ((c == ' ') && (*str == '\015'))) {
				if ((((lp+= 3) > PHP_QPRINT_MAXL) && (c <= 0x7f)) 
            || ((c > 0x7f) && (c <= 0xdf) && ((lp + 3) > PHP_QPRINT_MAXL)) 
            || ((c > 0xdf) && (c <= 0xef) && ((lp + 6) > PHP_QPRINT_MAXL)) 
            || ((c > 0xef) && (c <= 0xf4) && ((lp + 9) > PHP_QPRINT_MAXL))) {
					*d++ = '=';
					*d++ = '\015';
					*d++ = '\012';
					lp = 3;
				}
				*d++ = '=';
				*d++ = hex[c >> 4];
				*d++ = hex[c & 0xf];
			} else {
				if ((++lp) > PHP_QPRINT_MAXL) {
					*d++ = '=';
					*d++ = '\015';
					*d++ = '\012';
					lp = 1;
				}
				*d++ = c;
			}
		}
	}
	*d = '\0';
	*ret_length = d - ret;

	ret = erealloc(ret, *ret_length + 1);
	return ret;
}
/* }}} */

/*
*
* Decoding  Quoted-printable string.
*
*/
/* {{{ proto string quoted_printable_decode(string str)
   Convert a quoted-printable string to an 8 bit string */
PHP_FUNCTION(quoted_printable_decode)
{
	char *arg1, *str_in, *str_out;
	int arg1_len, i = 0, j = 0, k;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg1, &arg1_len) == FAILURE) {
		return;
	}
    
	if (arg1_len == 0) {
		/* shortcut */
		RETURN_EMPTY_STRING();
	}

	str_in = arg1;
	str_out = emalloc(arg1_len + 1);
	while (str_in[i]) {
		switch (str_in[i]) {
		case '=':
			if (str_in[i + 1] && str_in[i + 2] && 
				isxdigit((int) str_in[i + 1]) && 
				isxdigit((int) str_in[i + 2]))
			{
				str_out[j++] = (php_hex2int((int) str_in[i + 1]) << 4) 
						+ php_hex2int((int) str_in[i + 2]);
				i += 3;
			} else  /* check for soft line break according to RFC 2045*/ {
				k = 1;
				while (str_in[i + k] && ((str_in[i + k] == 32) || (str_in[i + k] == 9))) {
					/* Possibly, skip spaces/tabs at the end of line */
					k++;
				}
				if (!str_in[i + k]) {
					/* End of line reached */
					i += k;
				}
				else if ((str_in[i + k] == 13) && (str_in[i + k + 1] == 10)) {
					/* CRLF */
					i += k + 2;
				}
				else if ((str_in[i + k] == 13) || (str_in[i + k] == 10)) {
					/* CR or LF */
					i += k + 1;
				}
				else {
					str_out[j++] = str_in[i++];
				}
			}
			break;
		default:
			str_out[j++] = str_in[i++];
		}
	}
	str_out[j] = '\0';
    
	RETVAL_STRINGL(str_out, j, 0);
}
/* }}} */

/* {{{ proto string quoted_printable_encode(string str) */
PHP_FUNCTION(quoted_printable_encode)
{
	char *str, *new_str;
	int str_len;
	size_t new_str_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) != SUCCESS) {
		return;
	}

	if (!str_len) {
		RETURN_EMPTY_STRING();
	}

	new_str = (char *)php_quot_print_encode((unsigned char *)str, (size_t)str_len, &new_str_len);
	RETURN_STRINGL(new_str, new_str_len, 0);
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
	if (!str_len) {
		RETURN_EMPTY_STRING();
	}

	new_str = (char *)php_quot_print_encode((unsigned char *)str, (size_t)str_len, &new_str_len);
	RETURN_STRINGL(new_str, new_str_len, 0);
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