	char *param_spec;
	zend_fcall_info fci1, fci2;
	zend_fcall_info_cache fci1_cache = empty_fcall_info_cache, fci2_cache = empty_fcall_info_cache;
	zend_fcall_info *fci_key, *fci_data;
	zend_fcall_info_cache *fci_key_cache, *fci_data_cache;
	PHP_ARRAY_CMP_FUNC_VARS;

	int (*intersect_key_compare_func)(const void *, const void * TSRMLS_DC);
	int (*intersect_data_compare_func)(const void *, const void * TSRMLS_DC);
	char *param_spec;
	zend_fcall_info fci1, fci2;
	zend_fcall_info_cache fci1_cache = empty_fcall_info_cache, fci2_cache = empty_fcall_info_cache;
	zend_fcall_info *fci_key, *fci_data;
	zend_fcall_info_cache *fci_key_cache, *fci_data_cache;
	PHP_ARRAY_CMP_FUNC_VARS;

	int (*diff_key_compare_func)(const void *, const void * TSRMLS_DC);
	int (*diff_data_compare_func)(const void *, const void * TSRMLS_DC);
		 **entry,
		 entry_n;
	HashPosition pos;
	double dval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &input) == FAILURE) {
		return;
	}