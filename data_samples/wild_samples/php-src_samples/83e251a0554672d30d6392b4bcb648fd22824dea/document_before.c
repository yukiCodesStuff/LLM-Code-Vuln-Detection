{
	zval *id;
	xmlDoc *docp;
	dom_object *intern;
	char *source = NULL, *valid_file = NULL;
	int source_len = 0;
	xmlSchemaParserCtxtPtr  parser;
	xmlSchemaPtr            sptr;
	xmlSchemaValidCtxtPtr   vptr;
	int                     is_valid;
	char resolved_path[MAXPATHLEN + 1];

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Op", &id, dom_document_class_entry, &source, &source_len) == FAILURE) {
		return;
	}
	if (!vptr) {
		xmlSchemaFree(sptr);
		php_error(E_ERROR, "Invalid Schema Validation Context");
		RETURN_FALSE;
	}

	xmlSchemaSetValidErrors(vptr, php_libxml_error_handler, php_libxml_error_handler, vptr);
	is_valid = xmlSchemaValidateDoc(vptr, docp);
	xmlSchemaFree(sptr);
	xmlSchemaFreeValidCtxt(vptr);

	if (is_valid == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto boolean dom_document_schema_validate_file(string filename); */
PHP_FUNCTION(dom_document_schema_validate_file)
{
	_dom_document_schema_validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, DOM_LOAD_FILE);
}