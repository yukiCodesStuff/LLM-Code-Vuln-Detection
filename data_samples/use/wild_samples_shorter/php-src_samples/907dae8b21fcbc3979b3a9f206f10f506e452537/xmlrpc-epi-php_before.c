	zval* retval = NULL;
	XMLRPC_REQUEST response;
	STRUCT_XMLRPC_REQUEST_INPUT_OPTIONS opts = {{0}};
	opts.xml_elem_opts.encoding = encoding_in ? utf8_get_encoding_id_from_string(encoding_in) : ENCODING_DEFAULT;

	/* generate XMLRPC_REQUEST from raw xml */
	response = XMLRPC_REQUEST_FromXML(xml_in, xml_in_len, &opts);

		if (XMLRPC_RequestGetRequestType(response) == xmlrpc_request_call) {
			if (method_name_out) {
				zval_dtor(method_name_out);
				Z_TYPE_P(method_name_out) = IS_STRING;
				Z_STRVAL_P(method_name_out) = estrdup(XMLRPC_RequestGetMethodName(response));
				Z_STRLEN_P(method_name_out) = strlen(Z_STRVAL_P(method_name_out));
			}
		}

		/* dust, sweep, and mop */