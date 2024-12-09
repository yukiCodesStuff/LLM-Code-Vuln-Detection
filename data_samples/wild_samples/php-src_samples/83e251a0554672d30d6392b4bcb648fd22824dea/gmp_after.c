{
	zval **base_arg, **exp_arg, **mod_arg;
	mpz_t *gmpnum_base, *gmpnum_exp, *gmpnum_mod, *gmpnum_result;
	int use_ui = 0;
	int temp_base = 0, temp_exp = 0, temp_mod;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ZZZ", &base_arg, &exp_arg, &mod_arg) == FAILURE){
		return;
	}