{
	size_t cursor, maxlen, len;
	char *replaced;
	enum entity_charset charset = determine_charset(hint_charset TSRMLS_CC);
	int doctype = flags & ENT_HTML_DOC_TYPE_MASK;
	entity_table_opt entity_table;
	const enc_to_uni *to_uni_table = NULL;
	const entity_ht *inv_map = NULL; /* used for !double_encode */
	/* only used if flags includes ENT_HTML_IGNORE_ERRORS or ENT_HTML_SUBSTITUTE_DISALLOWED_CHARS */
	const unsigned char *replacement;
	size_t replacement_len;

	if (all) { /* replace with all named entities */
		if (CHARSET_PARTIAL_SUPPORT(charset)) {
			php_error_docref0(NULL TSRMLS_CC, E_STRICT, "Only basic entities "
				"substitution is supported for multi-byte encodings other than UTF-8; "
				"functionality is equivalent to htmlspecialchars");
		}
{
	long all = HTML_SPECIALCHARS,
		 flags = ENT_COMPAT;
	int doctype;
	entity_table_opt entity_table;
	const enc_to_uni *to_uni_table;
	char *charset_hint = NULL;
	int charset_hint_len;
	enum entity_charset charset;

	/* in this function we have to jump through some loops because we're
	 * getting the translated table from data structures that are optimized for
	 * random access, not traversal */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lls",
			&all, &flags, &charset_hint, &charset_hint_len) == FAILURE) {
		return;
	}