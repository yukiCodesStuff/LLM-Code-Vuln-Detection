}
/* }}} */

/* {{{ proto string number_format(float number [, int num_decimal_places [, string dec_seperator, string thousands_seperator]])
   Formats a number with grouped thousands */
PHP_FUNCTION(number_format)
{
	double num;