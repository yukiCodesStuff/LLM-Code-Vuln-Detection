
static void add_assoc_name_entry(zval * val, char * key, X509_NAME * name, int shortname TSRMLS_DC) /* {{{ */
{
	zval *subitem, *subentries;
	int i, j = -1, last = -1, obj_cnt = 0;
	char *sname;
	int nid;
			sname = (char *) OBJ_nid2ln(nid);
		}

		MAKE_STD_ZVAL(subentries);
		array_init(subentries);

		last = -1;
		for (;;) {
			j = X509_NAME_get_index_by_OBJ(name, obj, last);
			if (j < 0) {
				if (last != -1) break;
			} else {
				obj_cnt++;
				ne  = X509_NAME_get_entry(name, j);
				str = X509_NAME_ENTRY_get_data(ne);
				if (ASN1_STRING_type(str) != V_ASN1_UTF8STRING) {
					to_add_len = ASN1_STRING_to_UTF8(&to_add, str);
					if (to_add_len != -1) {
						add_next_index_stringl(subentries, (char *)to_add, to_add_len, 1);
					}
				} else {
					to_add = ASN1_STRING_data(str);
					to_add_len = ASN1_STRING_length(str);
					add_next_index_stringl(subentries, (char *)to_add, to_add_len, 1);
				}
			}
			last = j;
		}
		i = last;
		
		if (obj_cnt > 1) {
			add_assoc_zval_ex(subitem, sname, strlen(sname) + 1, subentries);
		} else {
			zval_dtor(subentries);
			FREE_ZVAL(subentries);
			if (obj_cnt && str && to_add_len > -1) {
				add_assoc_stringl(subitem, sname, (char *)to_add, to_add_len, 1);
			}
		}
	}
		bio_out = BIO_new(BIO_s_mem());
		if (nid == NID_subject_alt_name) {
			if (openssl_x509v3_subjectAltName(bio_out, extension) == 0) {
				add_assoc_stringl(subitem, extname, bio_buf->data, bio_buf->length, 1);
			} else {
				zval_dtor(return_value);
				if (certresource == -1 && cert) {