{
	PHP_MD5_CTX md5_context;
	PHP_SHA1_CTX sha1_context;
#if defined(HAVE_HASH_EXT) && !defined(COMPILE_DL_HASH)
	void *hash_context;
#endif
	unsigned char *digest;
	int digest_len;
	int j;
	char *buf, *outid;
	struct timeval tv;
	zval **array;
	zval **token;
	char *remote_addr = NULL;

	gettimeofday(&tv, NULL);

	if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &array) == SUCCESS &&
		Z_TYPE_PP(array) == IS_ARRAY &&
		zend_hash_find(Z_ARRVAL_PP(array), "REMOTE_ADDR", sizeof("REMOTE_ADDR"), (void **) &token) == SUCCESS
	) {
		remote_addr = Z_STRVAL_PP(token);
	}