ZEND_BEGIN_ARG_INFO_EX(arginfo_number_format, 0, 0, 1)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, num_decimal_places)
	ZEND_ARG_INFO(0, dec_seperator)
	ZEND_ARG_INFO(0, thousands_seperator)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fmod, 0)
	ZEND_ARG_INFO(0, x)