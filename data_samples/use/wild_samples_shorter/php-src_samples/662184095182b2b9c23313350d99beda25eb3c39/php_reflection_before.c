	if (zend_get_property_info(ce, hash_key->key, 1) == NULL) {
		zend_property_info property_info;

		property_info.flags = ZEND_ACC_IMPLICIT_PUBLIC;
		property_info.name = hash_key->key;
		property_info.ce = ce;
		property_info.offset = -1;