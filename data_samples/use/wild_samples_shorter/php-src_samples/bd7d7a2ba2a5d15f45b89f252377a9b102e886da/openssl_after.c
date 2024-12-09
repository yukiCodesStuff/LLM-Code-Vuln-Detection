
static void add_assoc_name_entry(zval * val, char * key, X509_NAME * name, int shortname TSRMLS_DC) /* {{{ */
{
	zval **data;
	zval *subitem, *subentries;
	int i, j = -1, last = -1, obj_cnt = 0;
	char *sname;
	int nid;
			sname = (char *) OBJ_nid2ln(nid);
		}

		str = X509_NAME_ENTRY_get_data(ne);
		if (ASN1_STRING_type(str) != V_ASN1_UTF8STRING) {
			to_add_len = ASN1_STRING_to_UTF8(&to_add, str);
		} else {
			to_add = ASN1_STRING_data(str);
			to_add_len = ASN1_STRING_length(str);
		}

		if (to_add_len != -1) {
			if (zend_hash_find(Z_ARRVAL_P(subitem), sname, strlen(sname)+1, (void**)&data) == SUCCESS) {
				if (Z_TYPE_PP(data) == IS_ARRAY) {
					subentries = *data;
					add_next_index_stringl(subentries, (char *)to_add, to_add_len, 1);
				} else if (Z_TYPE_PP(data) == IS_STRING) {
					MAKE_STD_ZVAL(subentries);
					array_init(subentries);
					add_next_index_stringl(subentries, Z_STRVAL_PP(data), Z_STRLEN_PP(data), 1);
					add_next_index_stringl(subentries, (char *)to_add, to_add_len, 1);
					zend_hash_update(Z_ARRVAL_P(subitem), sname, strlen(sname)+1, &subentries, sizeof(zval*), NULL);
				}
			} else {
				add_assoc_stringl(subitem, sname, (char *)to_add, to_add_len, 1);
			}
		}
	}
		bio_out = BIO_new(BIO_s_mem());
		if (nid == NID_subject_alt_name) {
			if (openssl_x509v3_subjectAltName(bio_out, extension) == 0) {
				BIO_get_mem_ptr(bio_out, &bio_buf);
				add_assoc_stringl(subitem, extname, bio_buf->data, bio_buf->length, 1);
			} else {
				zval_dtor(return_value);
				if (certresource == -1 && cert) {