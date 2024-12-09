{
	zval            *value;
	zval            *zattr;
	HashTable       *rv;
	php_sxe_object  *sxe;
	char            *name;
	xmlNodePtr       node;
	xmlAttrPtr       attr;
	int              namelen;
	int              test;
	char 		 use_iter;
	zval            *iter_data = NULL;

	use_iter = 0;

	sxe = php_sxe_fetch_object(object TSRMLS_CC);

	if (is_debug) {
		ALLOC_HASHTABLE(rv);
		zend_hash_init(rv, 0, NULL, ZVAL_PTR_DTOR, 0);
	}