
/* {{{ curl_free_slist
 */
static void curl_free_slist(void **slist)
{
	curl_slist_free_all((struct curl_slist *) *slist);
}
/* }}} */

/* {{{ proto array curl_version([int version])
	(*ch)->handlers->read->stream = NULL;

	zend_llist_init(&(*ch)->to_free->str,   sizeof(char *),            (llist_dtor_func_t) curl_free_string, 0);
	zend_llist_init(&(*ch)->to_free->slist, sizeof(struct curl_slist), (llist_dtor_func_t) curl_free_slist,  0);
	zend_llist_init(&(*ch)->to_free->post,  sizeof(struct HttpPost),   (llist_dtor_func_t) curl_free_post,   0);
	(*ch)->safe_upload = 0; /* for now, for BC reason we allow unsafe API */
}
/* }}} */

#if LIBCURL_VERSION_NUM >= 0x071301 /* Available since 7.19.1 */
	}
#endif

	efree(dupch->to_free);
	dupch->to_free = ch->to_free;

	/* Keep track of cloned copies to avoid invoking curl destructors for every clone */

			ph = HASH_OF(*zvalue);
			if (!ph) {
				char *name;
				switch (option) {
					case CURLOPT_HTTPHEADER:
						name = "CURLOPT_HTTPHEADER";
						break;
					return 1;
				}
			}
			zend_llist_add_element(&ch->to_free->slist, &slist);

			error = curl_easy_setopt(ch->cp, option, slist);

			break;
	/* cURL destructors should be invoked only by last curl handle */
	if (Z_REFCOUNT_P(ch->clone) <= 1) {
		zend_llist_clean(&ch->to_free->str);
		zend_llist_clean(&ch->to_free->slist);
		zend_llist_clean(&ch->to_free->post);
		efree(ch->to_free);
		FREE_ZVAL(ch->clone);
	} else {
		Z_DELREF_P(ch->clone);