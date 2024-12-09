		if (((xsltStylesheetPtr) intern->ptr)->_private != NULL) {
			((xsltStylesheetPtr) intern->ptr)->_private = NULL;   
		}
		xsltFreeStylesheet((xsltStylesheetPtr) intern->ptr);
		intern->ptr = NULL;
	}

	php_xsl_set_object(id, sheetp TSRMLS_CC);
	RETVAL_TRUE;
}
/* }}} end xsl_xsltprocessor_import_stylesheet */

static xmlDocPtr php_xsl_apply_stylesheet(zval *id, xsl_object *intern, xsltStylesheetPtr style, zval *docp TSRMLS_DC) /* {{{ */
{
	xmlDocPtr newdocp = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	xsltTransformContextPtr ctxt;
	php_libxml_node_object *object;
	char **params = NULL;
	int clone;
	zval *doXInclude, *member;
	zend_object_handlers *std_hnd;
	FILE *f;
	int secPrefsError = 0;
	int secPrefsValue, secPrefsIni;
	xsltSecurityPrefsPtr secPrefs = NULL;

	node = php_libxml_import_node(docp TSRMLS_CC);
	
	if (node) {
		doc = node->doc;
	}
	if (doc == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Document");
		return NULL;
	}

	if (style == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "No stylesheet associated to this object");
		return NULL;
	}
	
	if (intern->profiling) {
		if (php_check_open_basedir(intern->profiling TSRMLS_CC)) {
			f = NULL;
		} else {
			f = VCWD_FOPEN(intern->profiling, "w");
		}
	} else {
		f = NULL;
	}
	
	if (intern->parameter) {
		params = php_xsl_xslt_make_params(intern->parameter, 0 TSRMLS_CC);
	}

	intern->doc = emalloc(sizeof(php_libxml_node_object));
	memset(intern->doc, 0, sizeof(php_libxml_node_object));

	if (intern->hasKeys == 1) {
		doc = xmlCopyDoc(doc, 1);
	} else {
		object = (php_libxml_node_object *)zend_object_store_get_object(docp TSRMLS_CC);
		intern->doc->document = object->document;
	}

	php_libxml_increment_doc_ref(intern->doc, doc TSRMLS_CC);

	ctxt = xsltNewTransformContext(style, doc);
	ctxt->_private = (void *) intern;

	std_hnd = zend_get_std_object_handlers();

	MAKE_STD_ZVAL(member);
	ZVAL_STRING(member, "doXInclude", 0);
	doXInclude = std_hnd->read_property(id, member, BP_VAR_IS, NULL TSRMLS_CC);
	if (Z_TYPE_P(doXInclude) != IS_NULL) {
		convert_to_long(doXInclude);
		ctxt->xinclude = Z_LVAL_P(doXInclude);
	}
	efree(member);

	secPrefsValue = intern->securityPrefs;
	
	/* This whole if block can be removed, when we remove the xsl.security_prefs php.ini option in PHP 6+ */
	secPrefsIni= INI_INT("xsl.security_prefs");
	/* if secPrefsIni has the same value as secPrefsValue, all is fine */
	if (secPrefsIni != secPrefsValue) {
		if (secPrefsIni != XSL_SECPREF_DEFAULT) {
			/* if the ini value is not set to the default, throw an E_DEPRECATED warning */
			php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "The xsl.security_prefs php.ini option is deprecated; use XsltProcessor->setSecurityPrefs() instead");
			if (intern->securityPrefsSet == 0) {
				/* if securityPrefs were not set through the setSecurityPrefs method, take the ini setting */
				secPrefsValue = secPrefsIni;
			} else {
				/* else throw a notice, that the ini setting was not used */
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "The xsl.security_prefs php.ini was not used, since the  XsltProcessor->setSecurityPrefs() method was used");
			}
		}
	}

	/* if securityPrefs is set to NONE, we don't have to do any checks, but otherwise... */
	if (secPrefsValue != XSL_SECPREF_NONE) {
		secPrefs = xsltNewSecurityPrefs(); 
		if (secPrefsValue & XSL_SECPREF_READ_FILE ) { 
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_READ_FILE, xsltSecurityForbid)) { 
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_WRITE_FILE ) { 
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_WRITE_FILE, xsltSecurityForbid)) { 
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_CREATE_DIRECTORY ) { 
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid)) { 
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_READ_NETWORK) { 
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_READ_NETWORK, xsltSecurityForbid)) { 
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_WRITE_NETWORK) { 
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_WRITE_NETWORK, xsltSecurityForbid)) { 
				secPrefsError = 1;
			}
		}
	
		if (0 != xsltSetCtxtSecurityPrefs(secPrefs, ctxt)) { 
			secPrefsError = 1;
		}
	}
	
	if (secPrefsError == 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't set libxslt security properties, not doing transformation for security reasons");
	} else {
		newdocp = xsltApplyStylesheetUser(style, doc, (const char**) params,  NULL, f, ctxt);
	}
	if (f) {
		fclose(f);
	}
	
	xsltFreeTransformContext(ctxt);
	if (secPrefs) {
		xsltFreeSecurityPrefs(secPrefs);
	}

	if (intern->node_list != NULL) {
		zend_hash_destroy(intern->node_list);
		FREE_HASHTABLE(intern->node_list);	
		intern->node_list = NULL;
	}

	php_libxml_decrement_doc_ref(intern->doc TSRMLS_CC);
	efree(intern->doc);
	intern->doc = NULL;

	if (params) {
		clone = 0;
		while(params[clone]) {
			efree(params[clone++]);
		}
		efree(params);
	}

	return newdocp;

}
/* }}} */

/* {{{ proto domdocument xsl_xsltprocessor_transform_to_doc(domnode doc);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#
Since: 
*/
PHP_FUNCTION(xsl_xsltprocessor_transform_to_doc)
{
	zval *id, *docp = NULL;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret, ret_class_len=0;
	char *ret_class = NULL;
	xsl_object *intern;

	id = getThis();
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|s!", &docp, &ret_class, &ret_class_len) == FAILURE) {
		RETURN_FALSE;
	}

	newdocp = php_xsl_apply_stylesheet(id, intern, sheetp, docp TSRMLS_CC);

	if (newdocp) {
		if (ret_class) {
			int found;
			char *curclass_name;
			zend_class_entry *curce, **ce;
			php_libxml_node_object *interndoc;

			curce = Z_OBJCE_P(docp);
			curclass_name = curce->name;
			while (curce->parent != NULL) {
				curce = curce->parent;
			}

			found = zend_lookup_class(ret_class, ret_class_len, &ce TSRMLS_CC);
			if ((found != SUCCESS) || !instanceof_function(*ce, curce TSRMLS_CC)) {
				xmlFreeDoc(newdocp);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, 
					"Expecting class compatible with %s, '%s' given", curclass_name, ret_class);
				RETURN_FALSE;
			}

			object_init_ex(return_value, *ce);
		
			interndoc = (php_libxml_node_object *)zend_objects_get_address(return_value TSRMLS_CC);
			php_libxml_increment_doc_ref(interndoc, newdocp TSRMLS_CC);
			php_libxml_increment_node_ptr(interndoc, (xmlNodePtr)newdocp, (void *)interndoc TSRMLS_CC);
		} else {
			DOM_RET_OBJ((xmlNodePtr) newdocp, &ret, NULL);
		}
	} else {
		RETURN_FALSE;
	}
	
}
/* }}} end xsl_xsltprocessor_transform_to_doc */

/* {{{ proto int xsl_xsltprocessor_transform_to_uri(domdocument doc, string uri);
*/
PHP_FUNCTION(xsl_xsltprocessor_transform_to_uri)
{
	zval *id, *docp = NULL;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret, uri_len;
	char *uri;
	xsl_object *intern;
	
	id = getThis();
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "op", &docp, &uri, &uri_len) == FAILURE) {
		RETURN_FALSE;
	}

	newdocp = php_xsl_apply_stylesheet(id, intern, sheetp, docp TSRMLS_CC);

	ret = -1;
	if (newdocp) {
		ret = xsltSaveResultToFilename(uri, newdocp, sheetp, 0);
		xmlFreeDoc(newdocp);
	}

	RETVAL_LONG(ret);
}
/* }}} end xsl_xsltprocessor_transform_to_uri */

/* {{{ proto string xsl_xsltprocessor_transform_to_xml(domdocument doc);
*/
PHP_FUNCTION(xsl_xsltprocessor_transform_to_xml)
{
	zval *id, *docp = NULL;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret;
	xmlChar *doc_txt_ptr;
	int doc_txt_len;
	xsl_object *intern;
	
	id = getThis();
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &docp) == FAILURE) {
		RETURN_FALSE;
	}

	newdocp = php_xsl_apply_stylesheet(id, intern, sheetp, docp TSRMLS_CC);

	ret = -1;
	if (newdocp) {
		ret = xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, newdocp, sheetp);
		if (doc_txt_ptr && doc_txt_len) {
			RETVAL_STRINGL(doc_txt_ptr, doc_txt_len, 1);
			xmlFree(doc_txt_ptr);
		}
		xmlFreeDoc(newdocp);
	}

	if (ret < 0) {
		RETURN_FALSE;
	}
}
/* }}} end xsl_xsltprocessor_transform_to_xml */

/* {{{ proto bool xsl_xsltprocessor_set_parameter(string namespace, mixed name [, string value]);
*/
PHP_FUNCTION(xsl_xsltprocessor_set_parameter)
{
 
	zval *id;
	zval *array_value, **entry, *new_string;
	xsl_object *intern;
	char *string_key, *name, *value, *namespace;
	ulong idx;
	int string_key_len, namespace_len, name_len, value_len;
	DOM_GET_THIS(id);

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "sa", &namespace, &namespace_len, &array_value) == SUCCESS) {
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(array_value));

		while (zend_hash_get_current_data(Z_ARRVAL_P(array_value), (void **)&entry) == SUCCESS) {
			SEPARATE_ZVAL(entry);
			convert_to_string_ex(entry);
			
			if (zend_hash_get_current_key_ex(Z_ARRVAL_P(array_value), &string_key, &string_key_len, &idx, 0, NULL) != HASH_KEY_IS_STRING) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid parameter array");
				RETURN_FALSE;
			}
			
			ALLOC_ZVAL(new_string);
			Z_ADDREF_PP(entry);
			COPY_PZVAL_TO_ZVAL(*new_string, *entry);
			
			zend_hash_update(intern->parameter, string_key, string_key_len, &new_string, sizeof(zval*), NULL);
			zend_hash_move_forward(Z_ARRVAL_P(array_value));
		}
		RETURN_TRUE;

	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "sss", &namespace, &namespace_len, &name, &name_len, &value, &value_len) == SUCCESS) {
		
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		
		MAKE_STD_ZVAL(new_string);
		ZVAL_STRING(new_string, value, 1);
		
		zend_hash_update(intern->parameter, name, name_len + 1, &new_string, sizeof(zval*), NULL);
		RETURN_TRUE;
	} else {
		WRONG_PARAM_COUNT;
	}
	
}
/* }}} end xsl_xsltprocessor_set_parameter */

/* {{{ proto string xsl_xsltprocessor_get_parameter(string namespace, string name);
*/
PHP_FUNCTION(xsl_xsltprocessor_get_parameter)
{
	zval *id;
	int name_len = 0, namespace_len = 0;
	char *name, *namespace;
	zval **value;
	xsl_object *intern;

	DOM_GET_THIS(id);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &namespace, &namespace_len, &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	if ( zend_hash_find(intern->parameter, name, name_len + 1,  (void**) &value) == SUCCESS) {
		convert_to_string_ex(value);
		RETVAL_STRING(Z_STRVAL_PP(value),1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} end xsl_xsltprocessor_get_parameter */

/* {{{ proto bool xsl_xsltprocessor_remove_parameter(string namespace, string name);
*/
PHP_FUNCTION(xsl_xsltprocessor_remove_parameter)
{
	zval *id;
	int name_len = 0, namespace_len = 0;
	char *name, *namespace;
	xsl_object *intern;

	DOM_GET_THIS(id);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &namespace, &namespace_len, &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	if ( zend_hash_del(intern->parameter, name, name_len + 1) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} end xsl_xsltprocessor_remove_parameter */

/* {{{ proto void xsl_xsltprocessor_register_php_functions([mixed $restrict]);
*/
PHP_FUNCTION(xsl_xsltprocessor_register_php_functions)
{
	zval *id;
	xsl_object *intern;
	zval *array_value, **entry, *new_string;
	int  name_len = 0;
	char *name;

	DOM_GET_THIS(id);
	
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "a",  &array_value) == SUCCESS) {
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(array_value));

		while (zend_hash_get_current_data(Z_ARRVAL_P(array_value), (void **)&entry) == SUCCESS) {
			SEPARATE_ZVAL(entry);
			convert_to_string_ex(entry);
			
			MAKE_STD_ZVAL(new_string);
			ZVAL_LONG(new_string,1);
		
			zend_hash_update(intern->registered_phpfunctions, Z_STRVAL_PP(entry), Z_STRLEN_PP(entry) + 1, &new_string, sizeof(zval*), NULL);
			zend_hash_move_forward(Z_ARRVAL_P(array_value));
		}
		intern->registerPhpFunctions = 2;

	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s",  &name, &name_len) == SUCCESS) {
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		
		MAKE_STD_ZVAL(new_string);
		ZVAL_LONG(new_string,1);
		zend_hash_update(intern->registered_phpfunctions, name, name_len + 1, &new_string, sizeof(zval*), NULL);
		intern->registerPhpFunctions = 2;
		
	} else {
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		intern->registerPhpFunctions = 1;
	}
	
}
/* }}} end xsl_xsltprocessor_register_php_functions(); */

/* {{{ proto bool xsl_xsltprocessor_set_profiling(string filename) */
PHP_FUNCTION(xsl_xsltprocessor_set_profiling)
{
	zval *id;
	xsl_object *intern;
	char *filename = NULL;
	int filename_len;
	DOM_GET_THIS(id);

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "p!", &filename, &filename_len) == SUCCESS) {
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		if (intern->profiling) {
			efree(intern->profiling);
		}
		if (filename != NULL) {
			intern->profiling = estrndup(filename,filename_len);
		} else {
			intern->profiling = NULL;
		}
		RETURN_TRUE;
	} else {
		WRONG_PARAM_COUNT;
	}
}
/* }}} end xsl_xsltprocessor_set_profiling */

/* {{{ proto long xsl_xsltprocessor_set_security_prefs(long securityPrefs) */
PHP_FUNCTION(xsl_xsltprocessor_set_security_prefs)
{
	zval *id;
	xsl_object *intern;
	long securityPrefs, oldSecurityPrefs;

	DOM_GET_THIS(id);
 	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &securityPrefs) == FAILURE) {
		return;
	}
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	oldSecurityPrefs = intern->securityPrefs; 
	intern->securityPrefs = securityPrefs;
	/* set this to 1 so that we know, it was set through this method. Can be removed, when we remove the ini setting */
	intern->securityPrefsSet = 1;
	RETURN_LONG(oldSecurityPrefs);
}
/* }}} end xsl_xsltprocessor_set_security_prefs */

/* {{{ proto long xsl_xsltprocessor_get_security_prefs() */
PHP_FUNCTION(xsl_xsltprocessor_get_security_prefs)
{
	zval *id;
	xsl_object *intern;

	DOM_GET_THIS(id);
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "") == SUCCESS) {
		intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
		RETURN_LONG(intern->securityPrefs);
	} else {
		WRONG_PARAM_COUNT;
	}
}
/* }}} end xsl_xsltprocessor_get_security_prefs */



/* {{{ proto bool xsl_xsltprocessor_has_exslt_support();
*/
PHP_FUNCTION(xsl_xsltprocessor_has_exslt_support)
{
#if HAVE_XSL_EXSLT
	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}
/* }}} end xsl_xsltprocessor_has_exslt_support(); */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */