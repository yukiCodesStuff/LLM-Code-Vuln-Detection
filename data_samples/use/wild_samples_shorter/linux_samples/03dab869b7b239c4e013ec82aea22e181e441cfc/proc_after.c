	struct timespec now;
	unsigned long timo;
	key_ref_t key_ref, skey_ref;
	char xbuf[16];
	int rc;

	struct keyring_search_context ctx = {
		.index_key.type		= key->type,