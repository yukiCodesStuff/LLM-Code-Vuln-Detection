
		return (char*)&Z_OBJVAL_P(obj);
#else
		char *hash = emalloc((hash_len+1)*sizeof(char));

		zend_object_value zvalue;
		memset(&zvalue, 0, sizeof(zend_object_value));
		zvalue.handle = Z_OBJ_HANDLE_P(obj);
		zvalue.handlers = Z_OBJ_HT_P(obj);

		strncpy(hash, (char *)&zvalue, hash_len);
		hash[hash_len] = 0;

		if (hash_len_ptr) {
			*hash_len_ptr = hash_len;