	zval* retval = NULL;
	XMLRPC_REQUEST response;
	STRUCT_XMLRPC_REQUEST_INPUT_OPTIONS opts = {{0}};
	const char *method_name;
	opts.xml_elem_opts.encoding = encoding_in ? utf8_get_encoding_id_from_string(Z_STRVAL_P(encoding_in)) : ENCODING_DEFAULT;

	/* generate XMLRPC_REQUEST from raw xml */
	response = XMLRPC_REQUEST_FromXML(Z_STRVAL_P(xml_in), Z_STRLEN_P(xml_in), &opts);

		if(XMLRPC_RequestGetRequestType(response) == xmlrpc_request_call) {
			if(method_name_out) {
				method_name = XMLRPC_RequestGetMethodName(response);
				if (method_name) {
					zval_dtor(method_name_out);
					Z_TYPE_P(method_name_out) = IS_STRING;
					Z_STRVAL_P(method_name_out) = estrdup(method_name);
					Z_STRLEN_P(method_name_out) = strlen(Z_STRVAL_P(method_name_out));
				} else {
					retval = NULL;
				}
			}
		}

		/* dust, sweep, and mop */