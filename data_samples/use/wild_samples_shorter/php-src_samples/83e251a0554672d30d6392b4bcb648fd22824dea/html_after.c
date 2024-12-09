	const enc_to_uni *to_uni_table = NULL;
	const entity_ht *inv_map = NULL; /* used for !double_encode */
	/* only used if flags includes ENT_HTML_IGNORE_ERRORS or ENT_HTML_SUBSTITUTE_DISALLOWED_CHARS */
	const unsigned char *replacement = NULL;
	size_t replacement_len = 0;

	if (all) { /* replace with all named entities */
		if (CHARSET_PARTIAL_SUPPORT(charset)) {
			php_error_docref0(NULL TSRMLS_CC, E_STRICT, "Only basic entities "
		 flags = ENT_COMPAT;
	int doctype;
	entity_table_opt entity_table;
	const enc_to_uni *to_uni_table = NULL;
	char *charset_hint = NULL;
	int charset_hint_len;
	enum entity_charset charset;
