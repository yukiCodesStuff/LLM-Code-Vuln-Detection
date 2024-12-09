static void php_snmp_error(zval *object, const char *docref TSRMLS_DC, int type, const char *format, ...)
{
	va_list args;
	php_snmp_object *snmp_object;

	if (object) {
		snmp_object = (php_snmp_object *)zend_object_store_get_object(object TSRMLS_CC);
		if (type == PHP_SNMP_ERRNO_NOERROR) {