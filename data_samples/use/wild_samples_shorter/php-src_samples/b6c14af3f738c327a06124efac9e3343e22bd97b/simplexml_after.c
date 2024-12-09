	int              namelen;
	int              test;
	char 		 use_iter;
	zval            *iter_data = NULL;

	use_iter = 0;

	sxe = php_sxe_fetch_object(object TSRMLS_CC);