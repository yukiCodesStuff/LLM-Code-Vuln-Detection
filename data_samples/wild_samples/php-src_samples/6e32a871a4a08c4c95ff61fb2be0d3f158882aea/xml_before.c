	if (parser->ltags) {
		int inx;
		for (inx = 0; inx < parser->level; inx++)
			efree(parser->ltags[ inx ]);
		efree(parser->ltags);
	}
			if ((retval = xml_call_handler(parser, parser->startElementHandler, parser->startElementPtr, 3, args))) {
				zval_ptr_dtor(&retval);
			}
		} 

		if (parser->data) {
			zval *tag, *atr;
			int atcnt = 0;

			MAKE_STD_ZVAL(tag);
			MAKE_STD_ZVAL(atr);

			array_init(tag);
			array_init(atr);

			_xml_add_to_info(parser,((char *) tag_name) + parser->toffset);

			add_assoc_string(tag,"tag",((char *) tag_name) + parser->toffset,1); /* cast to avoid gcc-warning */
			add_assoc_string(tag,"type","open",1);
			add_assoc_long(tag,"level",parser->level);

			parser->ltags[parser->level-1] = estrdup(tag_name);
			parser->lastwasopen = 1;

			attributes = (const XML_Char **) attrs;

			while (attributes && *attributes) {
				att = _xml_decode_tag(parser, attributes[0]);
				val = xml_utf8_decode(attributes[1], strlen(attributes[1]), &val_len, parser->target_encoding);
				
				add_assoc_stringl(atr,att,val,val_len,0);

				atcnt++;
				attributes += 2;

				efree(att);
			}

			if (atcnt) {
				zend_hash_add(Z_ARRVAL_P(tag),"attributes",sizeof("attributes"),&atr,sizeof(zval*),NULL);
			} else {
				zval_ptr_dtor(&atr);
			}

			zend_hash_next_index_insert(Z_ARRVAL_P(parser->data),&tag,sizeof(zval*),(void *) &parser->ctag);
		}

		efree(tag_name);
	}
			} else {
				MAKE_STD_ZVAL(tag);

				array_init(tag);
				  
				_xml_add_to_info(parser,((char *) tag_name) + parser->toffset);

				add_assoc_string(tag,"tag",((char *) tag_name) + parser->toffset,1); /* cast to avoid gcc-warning */
				add_assoc_string(tag,"type","close",1);
				add_assoc_long(tag,"level",parser->level);
				  
				zend_hash_next_index_insert(Z_ARRVAL_P(parser->data),&tag,sizeof(zval*),NULL);
			}

			parser->lastwasopen = 0;
		}

		efree(tag_name);

		if (parser->ltags) {
			efree(parser->ltags[parser->level-1]);
		}
								if (zend_hash_find(Z_ARRVAL_PP(curtag),"value",sizeof("value"),(void **) &myval) == SUCCESS) {
									int newlen = Z_STRLEN_PP(myval) + decoded_len;
									Z_STRVAL_PP(myval) = erealloc(Z_STRVAL_PP(myval),newlen+1);
									strncpy(Z_STRVAL_PP(myval) + Z_STRLEN_PP(myval), decoded_value, decoded_len + 1);
									Z_STRLEN_PP(myval) += decoded_len;
									efree(decoded_value);
									return;
								}
							}
						}
					}

					MAKE_STD_ZVAL(tag);
					
					array_init(tag);
					
					_xml_add_to_info(parser,parser->ltags[parser->level-1] + parser->toffset);

					add_assoc_string(tag,"tag",parser->ltags[parser->level-1] + parser->toffset,1);
					add_assoc_string(tag,"value",decoded_value,0);
					add_assoc_string(tag,"type","cdata",1);
					add_assoc_long(tag,"level",parser->level);

					zend_hash_next_index_insert(Z_ARRVAL_P(parser->data),&tag,sizeof(zval*),NULL);
				}
			} else {
				efree(decoded_value);
			}
		}
	}
}
/* }}} */

/* {{{ _xml_processingInstructionHandler() */
void _xml_processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data)
{
	xml_parser *parser = (xml_parser *)userData;

	if (parser && parser->processingInstructionHandler) {
		zval *retval, *args[3];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(target, 0, parser->target_encoding);
		args[2] = _xml_xmlchar_zval(data, 0, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->processingInstructionHandler, parser->processingInstructionPtr, 3, args))) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */

/* {{{ _xml_defaultHandler() */
void _xml_defaultHandler(void *userData, const XML_Char *s, int len)
{
	xml_parser *parser = (xml_parser *)userData;

	if (parser && parser->defaultHandler) {
		zval *retval, *args[2];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(s, len, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->defaultHandler, parser->defaultPtr, 2, args))) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */

/* {{{ _xml_unparsedEntityDeclHandler() */
void _xml_unparsedEntityDeclHandler(void *userData, 
										 const XML_Char *entityName, 
										 const XML_Char *base,
										 const XML_Char *systemId,
										 const XML_Char *publicId,
										 const XML_Char *notationName)
{
	xml_parser *parser = (xml_parser *)userData;

	if (parser && parser->unparsedEntityDeclHandler) {
		zval *retval, *args[6];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(entityName, 0, parser->target_encoding);
		args[2] = _xml_xmlchar_zval(base, 0, parser->target_encoding);
		args[3] = _xml_xmlchar_zval(systemId, 0, parser->target_encoding);
		args[4] = _xml_xmlchar_zval(publicId, 0, parser->target_encoding);
		args[5] = _xml_xmlchar_zval(notationName, 0, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->unparsedEntityDeclHandler, parser->unparsedEntityDeclPtr, 6, args))) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */

/* {{{ _xml_notationDeclHandler() */
void _xml_notationDeclHandler(void *userData,
							  const XML_Char *notationName,
							  const XML_Char *base,
							  const XML_Char *systemId,
							  const XML_Char *publicId)
{
	xml_parser *parser = (xml_parser *)userData;

	if (parser && parser->notationDeclHandler) {
		zval *retval, *args[5];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(notationName, 0, parser->target_encoding);
		args[2] = _xml_xmlchar_zval(base, 0, parser->target_encoding);
		args[3] = _xml_xmlchar_zval(systemId, 0, parser->target_encoding);
		args[4] = _xml_xmlchar_zval(publicId, 0, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->notationDeclHandler, parser->notationDeclPtr, 5, args))) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */

/* {{{ _xml_externalEntityRefHandler() */
int _xml_externalEntityRefHandler(XML_Parser parserPtr,
								   const XML_Char *openEntityNames,
								   const XML_Char *base,
								   const XML_Char *systemId,
								   const XML_Char *publicId)
{
	xml_parser *parser = XML_GetUserData(parserPtr);
	int ret = 0; /* abort if no handler is set (should be configurable?) */

	if (parser && parser->externalEntityRefHandler) {
		zval *retval, *args[5];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(openEntityNames, 0, parser->target_encoding);
		args[2] = _xml_xmlchar_zval(base, 0, parser->target_encoding);
		args[3] = _xml_xmlchar_zval(systemId, 0, parser->target_encoding);
		args[4] = _xml_xmlchar_zval(publicId, 0, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->externalEntityRefHandler, parser->externalEntityRefPtr, 5, args))) {
			convert_to_long(retval);
			ret = Z_LVAL_P(retval);
			efree(retval);
		} else {
			ret = 0;
		}
	}
	return ret;
}
/* }}} */

/* {{{ _xml_startNamespaceDeclHandler() */
void _xml_startNamespaceDeclHandler(void *userData,const XML_Char *prefix, const XML_Char *uri)
{
	xml_parser *parser = (xml_parser *)userData;

	if (parser && parser->startNamespaceDeclHandler) {
		zval *retval, *args[3];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(prefix, 0, parser->target_encoding);
		args[2] = _xml_xmlchar_zval(uri, 0, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->startNamespaceDeclHandler, parser->startNamespaceDeclPtr, 3, args))) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */

/* {{{ _xml_endNamespaceDeclHandler() */
void _xml_endNamespaceDeclHandler(void *userData, const XML_Char *prefix)
{
	xml_parser *parser = (xml_parser *)userData;

	if (parser && parser->endNamespaceDeclHandler) {
		zval *retval, *args[2];

		args[0] = _xml_resource_zval(parser->index);
		args[1] = _xml_xmlchar_zval(prefix, 0, parser->target_encoding);
		if ((retval = xml_call_handler(parser, parser->endNamespaceDeclHandler, parser->endNamespaceDeclPtr, 2, args))) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */

/************************* EXTENSION FUNCTIONS *************************/

static void php_xml_parser_create_impl(INTERNAL_FUNCTION_PARAMETERS, int ns_support) /* {{{ */
{
	xml_parser *parser;
	int auto_detect = 0;

	char *encoding_param = NULL;
	int encoding_param_len = 0;

	char *ns_param = NULL;
	int ns_param_len = 0;
	
	XML_Char *encoding;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (ns_support ? "|ss": "|s"), &encoding_param, &encoding_param_len, &ns_param, &ns_param_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (encoding_param != NULL) {
		/* The supported encoding types are hardcoded here because
		 * we are limited to the encodings supported by expat/xmltok.
		 */
		if (encoding_param_len == 0) {
			encoding = XML(default_encoding);
			auto_detect = 1;
		} else if (strcasecmp(encoding_param, "ISO-8859-1") == 0) {
			encoding = "ISO-8859-1";
		} else if (strcasecmp(encoding_param, "UTF-8") == 0) {
			encoding = "UTF-8";
		} else if (strcasecmp(encoding_param, "US-ASCII") == 0) {
			encoding = "US-ASCII";
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unsupported source encoding \"%s\"", encoding_param);
			RETURN_FALSE;
		}
	} else {
		encoding = XML(default_encoding);
	}

	if (ns_support && ns_param == NULL){
		ns_param = ":";
	}

	parser = ecalloc(1, sizeof(xml_parser));
	parser->parser = XML_ParserCreate_MM((auto_detect ? NULL : encoding),
                                         &php_xml_mem_hdlrs, ns_param);

	parser->target_encoding = encoding;
	parser->case_folding = 1;
	parser->object = NULL;
	parser->isparsing = 0;

	XML_SetUserData(parser->parser, parser);

	ZEND_REGISTER_RESOURCE(return_value, parser,le_xml_parser);
	parser->index = Z_LVAL_P(return_value);
}
/* }}} */

/* {{{ proto resource xml_parser_create([string encoding]) 
   Create an XML parser */
PHP_FUNCTION(xml_parser_create)
{
	php_xml_parser_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);	
}
/* }}} */

/* {{{ proto resource xml_parser_create_ns([string encoding [, string sep]]) 
   Create an XML parser */
PHP_FUNCTION(xml_parser_create_ns)
{
	php_xml_parser_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto int xml_set_object(resource parser, object &obj) 
   Set up object which should be used for callbacks */
PHP_FUNCTION(xml_set_object)
{
	xml_parser *parser;
	zval *pind, *mythis;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ro", &pind, &mythis) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	/* please leave this commented - or ask thies@thieso.net before doing it (again) */
	if (parser->object) {
		zval_ptr_dtor(&parser->object);
	}

	/* please leave this commented - or ask thies@thieso.net before doing it (again) */
/* #ifdef ZEND_ENGINE_2
	zval_add_ref(&parser->object); 
#endif */

	ALLOC_ZVAL(parser->object);
	MAKE_COPY_ZVAL(&mythis, parser->object);

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_element_handler(resource parser, string shdl, string ehdl) 
   Set up start and end element handlers */
PHP_FUNCTION(xml_set_element_handler)
{
	xml_parser *parser;
	zval *pind, **shdl, **ehdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZZ", &pind, &shdl, &ehdl) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->startElementHandler, shdl);
	xml_set_handler(&parser->endElementHandler, ehdl);
	XML_SetElementHandler(parser->parser, _xml_startElementHandler, _xml_endElementHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_character_data_handler(resource parser, string hdl) 
   Set up character data handler */
PHP_FUNCTION(xml_set_character_data_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->characterDataHandler, hdl);
	XML_SetCharacterDataHandler(parser->parser, _xml_characterDataHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_processing_instruction_handler(resource parser, string hdl) 
   Set up processing instruction (PI) handler */
PHP_FUNCTION(xml_set_processing_instruction_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->processingInstructionHandler, hdl);
	XML_SetProcessingInstructionHandler(parser->parser, _xml_processingInstructionHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_default_handler(resource parser, string hdl) 
   Set up default handler */
PHP_FUNCTION(xml_set_default_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->defaultHandler, hdl);
	XML_SetDefaultHandler(parser->parser, _xml_defaultHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_unparsed_entity_decl_handler(resource parser, string hdl) 
   Set up unparsed entity declaration handler */
PHP_FUNCTION(xml_set_unparsed_entity_decl_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->unparsedEntityDeclHandler, hdl);
	XML_SetUnparsedEntityDeclHandler(parser->parser, _xml_unparsedEntityDeclHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_notation_decl_handler(resource parser, string hdl) 
   Set up notation declaration handler */
PHP_FUNCTION(xml_set_notation_decl_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->notationDeclHandler, hdl);
	XML_SetNotationDeclHandler(parser->parser, _xml_notationDeclHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_external_entity_ref_handler(resource parser, string hdl) 
   Set up external entity reference handler */
PHP_FUNCTION(xml_set_external_entity_ref_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->externalEntityRefHandler, hdl);
	XML_SetExternalEntityRefHandler(parser->parser, (void *) _xml_externalEntityRefHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_start_namespace_decl_handler(resource parser, string hdl) 
   Set up character data handler */
PHP_FUNCTION(xml_set_start_namespace_decl_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->startNamespaceDeclHandler, hdl);
	XML_SetStartNamespaceDeclHandler(parser->parser, _xml_startNamespaceDeclHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_set_end_namespace_decl_handler(resource parser, string hdl) 
   Set up character data handler */
PHP_FUNCTION(xml_set_end_namespace_decl_handler)
{
	xml_parser *parser;
	zval *pind, **hdl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &pind, &hdl) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	xml_set_handler(&parser->endNamespaceDeclHandler, hdl);
	XML_SetEndNamespaceDeclHandler(parser->parser, _xml_endNamespaceDeclHandler);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_parse(resource parser, string data [, int isFinal]) 
   Start parsing an XML document */
PHP_FUNCTION(xml_parse)
{
	xml_parser *parser;
	zval *pind;
	char *data;
	int data_len, ret;
	long isFinal = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|l", &pind, &data, &data_len, &isFinal) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	parser->isparsing = 1;
	ret = XML_Parse(parser->parser, data, data_len, isFinal);
	parser->isparsing = 0;
	RETVAL_LONG(ret);
}

/* }}} */

/* {{{ proto int xml_parse_into_struct(resource parser, string data, array &values [, array &index ])
   Parsing a XML document */

PHP_FUNCTION(xml_parse_into_struct)
{
	xml_parser *parser;
	zval *pind, **xdata, **info = NULL;
	char *data;
	int data_len, ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsZ|Z", &pind, &data, &data_len, &xdata, &info) == FAILURE) {
		return;
	}
	
	if (info) {	
		zval_dtor(*info);
		array_init(*info);
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	zval_dtor(*xdata);
	array_init(*xdata);

	parser->data = *xdata;
	
	if (info) {
		parser->info = *info;
	}
	
	parser->level = 0;
	parser->ltags = safe_emalloc(XML_MAXLEVEL, sizeof(char *), 0);

	XML_SetDefaultHandler(parser->parser, _xml_defaultHandler);
	XML_SetElementHandler(parser->parser, _xml_startElementHandler, _xml_endElementHandler);
	XML_SetCharacterDataHandler(parser->parser, _xml_characterDataHandler);

	parser->isparsing = 1;
	ret = XML_Parse(parser->parser, data, data_len, 1);
	parser->isparsing = 0;

	RETVAL_LONG(ret);
}
/* }}} */

/* {{{ proto int xml_get_error_code(resource parser) 
   Get XML parser error code */
PHP_FUNCTION(xml_get_error_code)
{
	xml_parser *parser;
	zval *pind;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	RETVAL_LONG((long)XML_GetErrorCode(parser->parser));
}
/* }}} */

/* {{{ proto string xml_error_string(int code)
   Get XML parser error string */
PHP_FUNCTION(xml_error_string)
{
	long code;
	char *str;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &code) == FAILURE) {
		return;
	}

	str = (char *)XML_ErrorString((int)code);
	if (str) {
		RETVAL_STRING(str, 1);
	}
}
/* }}} */

/* {{{ proto int xml_get_current_line_number(resource parser) 
   Get current line number for an XML parser */
PHP_FUNCTION(xml_get_current_line_number)
{
	xml_parser *parser;
	zval *pind;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	RETVAL_LONG(XML_GetCurrentLineNumber(parser->parser));
}
/* }}} */

/* {{{ proto int xml_get_current_column_number(resource parser)
   Get current column number for an XML parser */
PHP_FUNCTION(xml_get_current_column_number)
{
	xml_parser *parser;
	zval *pind;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	RETVAL_LONG(XML_GetCurrentColumnNumber(parser->parser));
}
/* }}} */

/* {{{ proto int xml_get_current_byte_index(resource parser) 
   Get current byte index for an XML parser */
PHP_FUNCTION(xml_get_current_byte_index)
{
	xml_parser *parser;
	zval *pind;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	RETVAL_LONG(XML_GetCurrentByteIndex(parser->parser));
}
/* }}} */

/* {{{ proto int xml_parser_free(resource parser) 
   Free an XML parser */
PHP_FUNCTION(xml_parser_free)
{
	zval *pind;
	xml_parser *parser;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	if (parser->isparsing == 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parser cannot be freed while it is parsing.");
		RETURN_FALSE;
	}

	if (zend_list_delete(parser->index) == FAILURE) {
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_parser_set_option(resource parser, int option, mixed value) 
   Set options in an XML parser */
PHP_FUNCTION(xml_parser_set_option)
{
	xml_parser *parser;
	zval *pind, **val;
	long opt;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlZ", &pind, &opt, &val) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	switch (opt) {
		case PHP_XML_OPTION_CASE_FOLDING:
			convert_to_long_ex(val);
			parser->case_folding = Z_LVAL_PP(val);
			break;
		case PHP_XML_OPTION_SKIP_TAGSTART:
			convert_to_long_ex(val);
			parser->toffset = Z_LVAL_PP(val);
			break;
		case PHP_XML_OPTION_SKIP_WHITE:
			convert_to_long_ex(val);
			parser->skipwhite = Z_LVAL_PP(val);
			break;
		case PHP_XML_OPTION_TARGET_ENCODING: {
			xml_encoding *enc;
			convert_to_string_ex(val);
			enc = xml_get_encoding(Z_STRVAL_PP(val));
			if (enc == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported target encoding \"%s\"", Z_STRVAL_PP(val));
				RETURN_FALSE;
			}
			parser->target_encoding = enc->name;
			break;
		}
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown option");
			RETURN_FALSE;
			break;
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int xml_parser_get_option(resource parser, int option) 
   Get options from an XML parser */
PHP_FUNCTION(xml_parser_get_option)
{
	xml_parser *parser;
	zval *pind;
	long opt;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &pind, &opt) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(parser,xml_parser *, &pind, -1, "XML Parser", le_xml_parser);

	switch (opt) {
		case PHP_XML_OPTION_CASE_FOLDING:
			RETURN_LONG(parser->case_folding);
			break;
		case PHP_XML_OPTION_TARGET_ENCODING:
			RETURN_STRING(parser->target_encoding, 1);
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown option");
			RETURN_FALSE;
			break;
	}

	RETVAL_FALSE;	/* never reached */
}
/* }}} */

/* {{{ proto string utf8_encode(string data) 
   Encodes an ISO-8859-1 string to UTF-8 */
PHP_FUNCTION(utf8_encode)
{
	char *arg;
	XML_Char *encoded;
	int arg_len, len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	encoded = xml_utf8_encode(arg, arg_len, &len, "ISO-8859-1");
	if (encoded == NULL) {
		RETURN_FALSE;
	}
	RETVAL_STRINGL(encoded, len, 0);
}
/* }}} */

/* {{{ proto string utf8_decode(string data) 
   Converts a UTF-8 encoded string to ISO-8859-1 */
PHP_FUNCTION(utf8_decode)
{
	char *arg;
	XML_Char *decoded;
	int arg_len, len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	decoded = xml_utf8_decode(arg, arg_len, &len, "ISO-8859-1");
	if (decoded == NULL) {
		RETURN_FALSE;
	}
	RETVAL_STRINGL(decoded, len, 0);
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */