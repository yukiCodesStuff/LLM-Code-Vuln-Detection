PHPAPI void php_register_variable_ex(char *var_name, zval *val, zval *track_vars_array TSRMLS_DC)
{
	char *p = NULL;
	char *ip;		/* index pointer */
	char *index;
	char *var, *var_orig;
	int var_len, index_len;
	zval *gpc_element, **gpc_element_p;