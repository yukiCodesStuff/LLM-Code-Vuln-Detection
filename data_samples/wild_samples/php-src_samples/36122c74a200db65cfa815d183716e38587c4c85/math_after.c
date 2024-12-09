{
	return _php_math_number_format_ex_len(d, dec, dec_point, dec_point_len,
			thousand_sep, thousand_sep_len, NULL);
}
/* }}} */

/* {{{ proto string number_format(float number [, int num_decimal_places [, string dec_separator, string thousands_separator]])
   Formats a number with grouped thousands */
PHP_FUNCTION(number_format)
{
	double num;
	long dec = 0;
	char *thousand_sep = NULL, *dec_point = NULL;
	char thousand_sep_chr = ',', dec_point_chr = '.';
	int thousand_sep_len = 0, dec_point_len = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|ls!s!", &num, &dec, &dec_point, &dec_point_len, &thousand_sep, &thousand_sep_len) == FAILURE) {
		return;
	}

	switch(ZEND_NUM_ARGS()) {
	case 1:
		RETURN_STRING(_php_math_number_format(num, 0, dec_point_chr, thousand_sep_chr), 0);
		break;
	case 2:
		RETURN_STRING(_php_math_number_format(num, dec, dec_point_chr, thousand_sep_chr), 0);
		break;
	case 4:
		if (dec_point == NULL) {
			dec_point = &dec_point_chr;
			dec_point_len = 1;
		}

		if (thousand_sep == NULL) {
			thousand_sep = &thousand_sep_chr;
			thousand_sep_len = 1;
		}

		Z_TYPE_P(return_value) = IS_STRING;
		Z_STRVAL_P(return_value) = _php_math_number_format_ex_len(num, dec,
				dec_point, dec_point_len, thousand_sep, thousand_sep_len,
				&Z_STRLEN_P(return_value));
		break;
	default:
		WRONG_PARAM_COUNT;
		break;
	}
}