
static void dom_xpath_ext_function_php(xmlXPathParserContextPtr ctxt, int nargs, int type) /* {{{ */
{
	zval **args;
	zval *retval;
	int result, i, ret;
	int error = 0;
	zend_fcall_info fci;