{
	zval ***args = NULL;
	HashTable *hash;
	int arr_argc, i, c = 0;
	Bucket ***lists, **list, ***ptrs, *p;
	int req_args;
	char *param_spec;
	zend_fcall_info fci1, fci2;
	zend_fcall_info_cache fci1_cache = empty_fcall_info_cache, fci2_cache = empty_fcall_info_cache;
	zend_fcall_info *fci_key, *fci_data;
	zend_fcall_info_cache *fci_key_cache, *fci_data_cache;
	PHP_ARRAY_CMP_FUNC_VARS;

	int (*intersect_key_compare_func)(const void *, const void * TSRMLS_DC);
	int (*intersect_data_compare_func)(const void *, const void * TSRMLS_DC);

	if (behavior == INTERSECT_NORMAL) {
		intersect_key_compare_func = php_array_key_compare;

		if (data_compare_type == INTERSECT_COMP_DATA_INTERNAL) {
			/* array_intersect() */
			req_args = 2;
			param_spec = "+";
			intersect_data_compare_func = php_array_data_compare;
		} else if (data_compare_type == INTERSECT_COMP_DATA_USER) {
			/* array_uintersect() */
			req_args = 3;
			param_spec = "+f";
			intersect_data_compare_func = php_array_user_compare;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_compare_type is %d. This should never happen. Please report as a bug", data_compare_type);
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least %d parameters are required, %d given", req_args, ZEND_NUM_ARGS());
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache) == FAILURE) {
			return;
		}
		fci_data = &fci1;
		fci_data_cache = &fci1_cache;

	} else if (behavior & INTERSECT_ASSOC) { /* triggered also when INTERSECT_KEY */
		/* INTERSECT_KEY is subset of INTERSECT_ASSOC. When having the former
		 * no comparison of the data is done (part of INTERSECT_ASSOC) */
		intersect_key_compare_func = php_array_key_compare;

		if (data_compare_type == INTERSECT_COMP_DATA_INTERNAL && key_compare_type == INTERSECT_COMP_KEY_INTERNAL) {
			/* array_intersect_assoc() or array_intersect_key() */
			req_args = 2;
			param_spec = "+";
			intersect_key_compare_func = php_array_key_compare;
			intersect_data_compare_func = php_array_data_compare;
		} else if (data_compare_type == INTERSECT_COMP_DATA_USER && key_compare_type == INTERSECT_COMP_KEY_INTERNAL) {
			/* array_uintersect_assoc() */
			req_args = 3;
			param_spec = "+f";
			intersect_key_compare_func = php_array_key_compare;
			intersect_data_compare_func = php_array_user_compare;
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
		} else if (data_compare_type == INTERSECT_COMP_DATA_INTERNAL && key_compare_type == INTERSECT_COMP_KEY_USER) {
			/* array_intersect_uassoc() or array_intersect_ukey() */
			req_args = 3;
			param_spec = "+f";
			intersect_key_compare_func = php_array_user_key_compare;
			intersect_data_compare_func = php_array_data_compare;
			fci_key = &fci1;
			fci_key_cache = &fci1_cache;
		} else if (data_compare_type == INTERSECT_COMP_DATA_USER && key_compare_type == INTERSECT_COMP_KEY_USER) {
			/* array_uintersect_uassoc() */
			req_args = 4;
			param_spec = "+ff";
			intersect_key_compare_func = php_array_user_key_compare;
			intersect_data_compare_func = php_array_user_compare;
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
			fci_key = &fci2;
			fci_key_cache = &fci2_cache;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_compare_type is %d. key_compare_type is %d. This should never happen. Please report as a bug", data_compare_type, key_compare_type);
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least %d parameters are required, %d given", req_args, ZEND_NUM_ARGS());
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache, &fci2, &fci2_cache) == FAILURE) {
			return;
		}

	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "behavior is %d. This should never happen. Please report as a bug", behavior);
		return;
	}

	PHP_ARRAY_CMP_FUNC_BACKUP();

	/* for each argument, create and sort list with pointers to the hash buckets */
	lists = (Bucket ***)safe_emalloc(arr_argc, sizeof(Bucket **), 0);
	ptrs = (Bucket ***)safe_emalloc(arr_argc, sizeof(Bucket **), 0);
	php_set_compare_func(PHP_SORT_STRING TSRMLS_CC);

	if (behavior == INTERSECT_NORMAL && data_compare_type == INTERSECT_COMP_DATA_USER) {
		BG(user_compare_fci) = *fci_data;
		BG(user_compare_fci_cache) = *fci_data_cache;
	} else if (behavior & INTERSECT_ASSOC && key_compare_type == INTERSECT_COMP_KEY_USER) {
		BG(user_compare_fci) = *fci_key;
		BG(user_compare_fci_cache) = *fci_key_cache;
	}

	for (i = 0; i < arr_argc; i++) {
		if (Z_TYPE_PP(args[i]) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is not an array", i + 1);
			arr_argc = i; /* only free up to i - 1 */
			goto out;
		}
		hash = Z_ARRVAL_PP(args[i]);
		list = (Bucket **) pemalloc((hash->nNumOfElements + 1) * sizeof(Bucket *), hash->persistent);
		if (!list) {
			PHP_ARRAY_CMP_FUNC_RESTORE();

			efree(ptrs);
			efree(lists);
			efree(args);
			RETURN_FALSE;
		}
		lists[i] = list;
		ptrs[i] = list;
		for (p = hash->pListHead; p; p = p->pListNext) {
			*list++ = p;
		}
		*list = NULL;
		if (behavior == INTERSECT_NORMAL) {
			zend_qsort((void *) lists[i], hash->nNumOfElements, sizeof(Bucket *), intersect_data_compare_func TSRMLS_CC);
		} else if (behavior & INTERSECT_ASSOC) { /* triggered also when INTERSECT_KEY */
			zend_qsort((void *) lists[i], hash->nNumOfElements, sizeof(Bucket *), intersect_key_compare_func TSRMLS_CC);
		}
	}

	/* copy the argument array */
	RETVAL_ZVAL(*args[0], 1, 0);
	if (return_value->value.ht == &EG(symbol_table)) {
		HashTable *ht;
		zval *tmp;

		ALLOC_HASHTABLE(ht);
		zend_hash_init(ht, zend_hash_num_elements(return_value->value.ht), NULL, ZVAL_PTR_DTOR, 0);
		zend_hash_copy(ht, return_value->value.ht, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
		return_value->value.ht = ht;
	}

	/* go through the lists and look for common values */
	while (*ptrs[0]) {
		if ((behavior & INTERSECT_ASSOC) /* triggered also when INTERSECT_KEY */
			&&
			key_compare_type == INTERSECT_COMP_KEY_USER) {

			BG(user_compare_fci) = *fci_key;
			BG(user_compare_fci_cache) = *fci_key_cache;
		}

		for (i = 1; i < arr_argc; i++) {
			if (behavior & INTERSECT_NORMAL) {
				while (*ptrs[i] && (0 < (c = intersect_data_compare_func(ptrs[0], ptrs[i] TSRMLS_CC)))) {
					ptrs[i]++;
				}
			} else if (behavior & INTERSECT_ASSOC) { /* triggered also when INTERSECT_KEY */
				while (*ptrs[i] && (0 < (c = intersect_key_compare_func(ptrs[0], ptrs[i] TSRMLS_CC)))) {
					ptrs[i]++;
				}
				if ((!c && *ptrs[i]) && (behavior == INTERSECT_ASSOC)) { /* only when INTERSECT_ASSOC */
					/* this means that ptrs[i] is not NULL so we can compare
					 * and "c==0" is from last operation
					 * in this branch of code we enter only when INTERSECT_ASSOC
					 * since when we have INTERSECT_KEY compare of data is not wanted. */
					if (data_compare_type == INTERSECT_COMP_DATA_USER) {
						BG(user_compare_fci) = *fci_data;
						BG(user_compare_fci_cache) = *fci_data_cache;
					}
					if (intersect_data_compare_func(ptrs[0], ptrs[i] TSRMLS_CC) != 0) {
						c = 1;
						if (key_compare_type == INTERSECT_COMP_KEY_USER) {
							BG(user_compare_fci) = *fci_key;
							BG(user_compare_fci_cache) = *fci_key_cache;
							/* When KEY_USER, the last parameter is always the callback */
						}
						/* we are going to the break */
					} else {
						/* continue looping */
					}
				}
			}
			if (!*ptrs[i]) {
				/* delete any values corresponding to remains of ptrs[0] */
				/* and exit because they do not present in at least one of */
				/* the other arguments */
				for (;;) {
					p = *ptrs[0]++;
					if (!p) {
						goto out;
					}
					if (p->nKeyLength == 0) {
						zend_hash_index_del(Z_ARRVAL_P(return_value), p->h);
					} else {
						zend_hash_quick_del(Z_ARRVAL_P(return_value), p->arKey, p->nKeyLength, p->h);
					}
				}
			}
			if (c) /* here we get if not all are equal */
				break;
			ptrs[i]++;
		}
		if (c) {
			/* Value of ptrs[0] not in all arguments, delete all entries */
			/* with value < value of ptrs[i] */
			for (;;) {
				p = *ptrs[0];
				if (p->nKeyLength == 0) {
					zend_hash_index_del(Z_ARRVAL_P(return_value), p->h);
				} else {
					zend_hash_quick_del(Z_ARRVAL_P(return_value), p->arKey, p->nKeyLength, p->h);
				}
				if (!*++ptrs[0]) {
					goto out;
				}
				if (behavior == INTERSECT_NORMAL) {
					if (0 <= intersect_data_compare_func(ptrs[0], ptrs[i] TSRMLS_CC)) {
						break;
					}
				} else if (behavior & INTERSECT_ASSOC) { /* triggered also when INTERSECT_KEY */
					/* no need of looping because indexes are unique */
					break;
				}
			}
		} else {
			/* ptrs[0] is present in all the arguments */
			/* Skip all entries with same value as ptrs[0] */
			for (;;) {
				if (!*++ptrs[0]) {
					goto out;
				}
				if (behavior == INTERSECT_NORMAL) {
					if (intersect_data_compare_func(ptrs[0] - 1, ptrs[0] TSRMLS_CC)) {
						break;
					}
				} else if (behavior & INTERSECT_ASSOC) { /* triggered also when INTERSECT_KEY */
					/* no need of looping because indexes are unique */
					break;
				}
			}
		}
	}
out:
	for (i = 0; i < arr_argc; i++) {
		hash = Z_ARRVAL_PP(args[i]);
		pefree(lists[i], hash->persistent);
	}

	PHP_ARRAY_CMP_FUNC_RESTORE();

	efree(ptrs);
	efree(lists);
	efree(args);
}
/* }}} */

/* {{{ proto array array_intersect_key(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have keys which are present in all the other arguments. Kind of equivalent to array_diff(array_keys($arr1), array_keys($arr2)[,array_keys(...)]). Equivalent of array_intersect_assoc() but does not do compare of the data. */
PHP_FUNCTION(array_intersect_key)
{
	php_array_intersect_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_COMP_DATA_NONE);
}
/* }}} */

/* {{{ proto array array_intersect_ukey(array arr1, array arr2 [, array ...], callback key_compare_func)
   Returns the entries of arr1 that have keys which are present in all the other arguments. Kind of equivalent to array_diff(array_keys($arr1), array_keys($arr2)[,array_keys(...)]). The comparison of the keys is performed by a user supplied function. Equivalent of array_intersect_uassoc() but does not do compare of the data. */
PHP_FUNCTION(array_intersect_ukey)
{
	php_array_intersect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_KEY, INTERSECT_COMP_DATA_INTERNAL, INTERSECT_COMP_KEY_USER);
}
/* }}} */

/* {{{ proto array array_intersect(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have values which are present in all the other arguments */
PHP_FUNCTION(array_intersect)
{
	php_array_intersect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_NORMAL, INTERSECT_COMP_DATA_INTERNAL, INTERSECT_COMP_KEY_INTERNAL);
}
/* }}} */

/* {{{ proto array array_uintersect(array arr1, array arr2 [, array ...], callback data_compare_func)
   Returns the entries of arr1 that have values which are present in all the other arguments. Data is compared by using an user-supplied callback. */
PHP_FUNCTION(array_uintersect)
{
	php_array_intersect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_NORMAL, INTERSECT_COMP_DATA_USER, INTERSECT_COMP_KEY_INTERNAL);
}
/* }}} */

/* {{{ proto array array_intersect_assoc(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have values which are present in all the other arguments. Keys are used to do more restrictive check */
PHP_FUNCTION(array_intersect_assoc)
{
	php_array_intersect_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_COMP_DATA_INTERNAL);
}
/* }}} */

/* {{{ proto array array_intersect_uassoc(array arr1, array arr2 [, array ...], callback key_compare_func) U
   Returns the entries of arr1 that have values which are present in all the other arguments. Keys are used to do more restrictive check and they are compared by using an user-supplied callback. */
PHP_FUNCTION(array_intersect_uassoc)
{
	php_array_intersect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_ASSOC, INTERSECT_COMP_DATA_INTERNAL, INTERSECT_COMP_KEY_USER);
}
/* }}} */

/* {{{ proto array array_uintersect_assoc(array arr1, array arr2 [, array ...], callback data_compare_func) U
   Returns the entries of arr1 that have values which are present in all the other arguments. Keys are used to do more restrictive check. Data is compared by using an user-supplied callback. */
PHP_FUNCTION(array_uintersect_assoc)
{
	php_array_intersect_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_COMP_DATA_USER);
}
/* }}} */

/* {{{ proto array array_uintersect_uassoc(array arr1, array arr2 [, array ...], callback data_compare_func, callback key_compare_func)
   Returns the entries of arr1 that have values which are present in all the other arguments. Keys are used to do more restrictive check. Both data and keys are compared by using user-supplied callbacks. */
PHP_FUNCTION(array_uintersect_uassoc)
{
	php_array_intersect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INTERSECT_ASSOC, INTERSECT_COMP_DATA_USER, INTERSECT_COMP_KEY_USER);
}
/* }}} */

static void php_array_diff_key(INTERNAL_FUNCTION_PARAMETERS, int data_compare_type) /* {{{ */
{
	Bucket *p;
	int argc, i;
	zval ***args;
	int (*diff_data_compare_func)(zval **, zval ** TSRMLS_DC) = NULL;
	zend_bool ok;
	zval **data;

	/* Get the argument count */
	argc = ZEND_NUM_ARGS();
	if (data_compare_type == DIFF_COMP_DATA_USER) {
		if (argc < 3) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least 3 parameters are required, %d given", ZEND_NUM_ARGS());
			return;
		}
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+f", &args, &argc, &BG(user_compare_fci), &BG(user_compare_fci_cache)) == FAILURE) {
			return;
		}
		diff_data_compare_func = zval_user_compare;
	} else {
		if (argc < 2) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least 2 parameters are required, %d given", ZEND_NUM_ARGS());
			return;
		}
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc) == FAILURE) {
			return;
		}
		if (data_compare_type == DIFF_COMP_DATA_INTERNAL) {
			diff_data_compare_func = zval_compare;
		}
	}

	for (i = 0; i < argc; i++) {
		if (Z_TYPE_PP(args[i]) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is not an array", i + 1);
			RETVAL_NULL();
			goto out;
		}
	}

	array_init(return_value);

	for (p = Z_ARRVAL_PP(args[0])->pListHead; p != NULL; p = p->pListNext) {
		if (p->nKeyLength == 0) {
			ok = 1;
			for (i = 1; i < argc; i++) {
				if (zend_hash_index_find(Z_ARRVAL_PP(args[i]), p->h, (void**)&data) == SUCCESS &&
					(!diff_data_compare_func ||
					diff_data_compare_func((zval**)p->pData, data TSRMLS_CC) == 0)
				) {
					ok = 0;
					break;
				}
			}
			if (ok) {
				Z_ADDREF_PP((zval**)p->pData);
				zend_hash_index_update(Z_ARRVAL_P(return_value), p->h, p->pData, sizeof(zval*), NULL);
			}
		} else {
			ok = 1;
			for (i = 1; i < argc; i++) {
				if (zend_hash_quick_find(Z_ARRVAL_PP(args[i]), p->arKey, p->nKeyLength, p->h, (void**)&data) == SUCCESS &&
					(!diff_data_compare_func ||
					diff_data_compare_func((zval**)p->pData, data TSRMLS_CC) == 0)
				) {
					ok = 0;
					break;
				}
			}
			if (ok) {
				Z_ADDREF_PP((zval**)p->pData);
				zend_hash_quick_update(Z_ARRVAL_P(return_value), p->arKey, p->nKeyLength, p->h, p->pData, sizeof(zval*), NULL);
			}
		}
	}
out:
	efree(args);
}
/* }}} */

static void php_array_diff(INTERNAL_FUNCTION_PARAMETERS, int behavior, int data_compare_type, int key_compare_type) /* {{{ */
{
	zval ***args = NULL;
	HashTable *hash;
	int arr_argc, i, c;
	Bucket ***lists, **list, ***ptrs, *p;
	int req_args;
	char *param_spec;
	zend_fcall_info fci1, fci2;
	zend_fcall_info_cache fci1_cache = empty_fcall_info_cache, fci2_cache = empty_fcall_info_cache;
	zend_fcall_info *fci_key, *fci_data;
	zend_fcall_info_cache *fci_key_cache, *fci_data_cache;
	PHP_ARRAY_CMP_FUNC_VARS;

	int (*diff_key_compare_func)(const void *, const void * TSRMLS_DC);
	int (*diff_data_compare_func)(const void *, const void * TSRMLS_DC);

	if (behavior == DIFF_NORMAL) {
		diff_key_compare_func = php_array_key_compare;

		if (data_compare_type == DIFF_COMP_DATA_INTERNAL) {
			/* array_diff */
			req_args = 2;
			param_spec = "+";
			diff_data_compare_func = php_array_data_compare;
		} else if (data_compare_type == DIFF_COMP_DATA_USER) {
			/* array_udiff */
			req_args = 3;
			param_spec = "+f";
			diff_data_compare_func = php_array_user_compare;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_compare_type is %d. This should never happen. Please report as a bug", data_compare_type);
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least %d parameters are required, %d given", req_args, ZEND_NUM_ARGS());
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache) == FAILURE) {
			return;
		}
		fci_data = &fci1;
		fci_data_cache = &fci1_cache;

	} else if (behavior & DIFF_ASSOC) { /* triggered also if DIFF_KEY */
		/* DIFF_KEY is subset of DIFF_ASSOC. When having the former
		 * no comparison of the data is done (part of DIFF_ASSOC) */

		if (data_compare_type == DIFF_COMP_DATA_INTERNAL && key_compare_type == DIFF_COMP_KEY_INTERNAL) {
			/* array_diff_assoc() or array_diff_key() */
			req_args = 2;
			param_spec = "+";
			diff_key_compare_func = php_array_key_compare;
			diff_data_compare_func = php_array_data_compare;
		} else if (data_compare_type == DIFF_COMP_DATA_USER && key_compare_type == DIFF_COMP_KEY_INTERNAL) {
			/* array_udiff_assoc() */
			req_args = 3;
			param_spec = "+f";
			diff_key_compare_func = php_array_key_compare;
			diff_data_compare_func = php_array_user_compare;
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
		} else if (data_compare_type == DIFF_COMP_DATA_INTERNAL && key_compare_type == DIFF_COMP_KEY_USER) {
			/* array_diff_uassoc() or array_diff_ukey() */
			req_args = 3;
			param_spec = "+f";
			diff_key_compare_func = php_array_user_key_compare;
			diff_data_compare_func = php_array_data_compare;
			fci_key = &fci1;
			fci_key_cache = &fci1_cache;
		} else if (data_compare_type == DIFF_COMP_DATA_USER && key_compare_type == DIFF_COMP_KEY_USER) {
			/* array_udiff_uassoc() */
			req_args = 4;
			param_spec = "+ff";
			diff_key_compare_func = php_array_user_key_compare;
			diff_data_compare_func = php_array_user_compare;
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
			fci_key = &fci2;
			fci_key_cache = &fci2_cache;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_compare_type is %d. key_compare_type is %d. This should never happen. Please report as a bug", data_compare_type, key_compare_type);
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least %d parameters are required, %d given", req_args, ZEND_NUM_ARGS());
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache, &fci2, &fci2_cache) == FAILURE) {
			return;
		}

	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "behavior is %d. This should never happen. Please report as a bug", behavior);
		return;
	}

	PHP_ARRAY_CMP_FUNC_BACKUP();

	/* for each argument, create and sort list with pointers to the hash buckets */
	lists = (Bucket ***)safe_emalloc(arr_argc, sizeof(Bucket **), 0);
	ptrs = (Bucket ***)safe_emalloc(arr_argc, sizeof(Bucket **), 0);
	php_set_compare_func(PHP_SORT_STRING TSRMLS_CC);

	if (behavior == DIFF_NORMAL && data_compare_type == DIFF_COMP_DATA_USER) {
		BG(user_compare_fci) = *fci_data;
		BG(user_compare_fci_cache) = *fci_data_cache;
	} else if (behavior & DIFF_ASSOC && key_compare_type == DIFF_COMP_KEY_USER) {
		BG(user_compare_fci) = *fci_key;
		BG(user_compare_fci_cache) = *fci_key_cache;
	}

	for (i = 0; i < arr_argc; i++) {
		if (Z_TYPE_PP(args[i]) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is not an array", i + 1);
			arr_argc = i; /* only free up to i - 1 */
			goto out;
		}
		hash = Z_ARRVAL_PP(args[i]);
		list = (Bucket **) pemalloc((hash->nNumOfElements + 1) * sizeof(Bucket *), hash->persistent);
		if (!list) {
			PHP_ARRAY_CMP_FUNC_RESTORE();

			efree(ptrs);
			efree(lists);
			efree(args);
			RETURN_FALSE;
		}
		lists[i] = list;
		ptrs[i] = list;
		for (p = hash->pListHead; p; p = p->pListNext) {
			*list++ = p;
		}
		*list = NULL;
		if (behavior == DIFF_NORMAL) {
			zend_qsort((void *) lists[i], hash->nNumOfElements, sizeof(Bucket *), diff_data_compare_func TSRMLS_CC);
		} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
			zend_qsort((void *) lists[i], hash->nNumOfElements, sizeof(Bucket *), diff_key_compare_func TSRMLS_CC);
		}
	}

	/* copy the argument array */
	RETVAL_ZVAL(*args[0], 1, 0);
	if (return_value->value.ht == &EG(symbol_table)) {
		HashTable *ht;
		zval *tmp;

		ALLOC_HASHTABLE(ht);
		zend_hash_init(ht, zend_hash_num_elements(return_value->value.ht), NULL, ZVAL_PTR_DTOR, 0);
		zend_hash_copy(ht, return_value->value.ht, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
		return_value->value.ht = ht;
	}

	/* go through the lists and look for values of ptr[0] that are not in the others */
	while (*ptrs[0]) {
		if ((behavior & DIFF_ASSOC) /* triggered also when DIFF_KEY */
			&&
			key_compare_type == DIFF_COMP_KEY_USER
		) {
			BG(user_compare_fci) = *fci_key;
			BG(user_compare_fci_cache) = *fci_key_cache;
		}
		c = 1;
		for (i = 1; i < arr_argc; i++) {
			Bucket **ptr = ptrs[i];
			if (behavior == DIFF_NORMAL) {
				while (*ptrs[i] && (0 < (c = diff_data_compare_func(ptrs[0], ptrs[i] TSRMLS_CC)))) {
					ptrs[i]++;
				}
			} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
				while (*ptr && (0 != (c = diff_key_compare_func(ptrs[0], ptr TSRMLS_CC)))) {
					ptr++;
				}
			}
			if (!c) {
				if (behavior == DIFF_NORMAL) {
					if (*ptrs[i]) {
						ptrs[i]++;
					}
					break;
				} else if (behavior == DIFF_ASSOC) {  /* only when DIFF_ASSOC */
					/* In this branch is execute only when DIFF_ASSOC. If behavior == DIFF_KEY
					 * data comparison is not needed - skipped. */
					if (*ptr) {
						if (data_compare_type == DIFF_COMP_DATA_USER) {
							BG(user_compare_fci) = *fci_data;
							BG(user_compare_fci_cache) = *fci_data_cache;
						}
						if (diff_data_compare_func(ptrs[0], ptr TSRMLS_CC) != 0) {
							/* the data is not the same */
							c = -1;
							if (key_compare_type == DIFF_COMP_KEY_USER) {
								BG(user_compare_fci) = *fci_key;
								BG(user_compare_fci_cache) = *fci_key_cache;
							}
						} else {
							break;
							/* we have found the element in other arrays thus we don't want it
							 * in the return_value -> delete from there */
						}
					}
				} else if (behavior == DIFF_KEY) { /* only when DIFF_KEY */
					/* the behavior here differs from INTERSECT_KEY in php_intersect
					 * since in the "diff" case we have to remove the entry from
					 * return_value while when doing intersection the entry must not
					 * be deleted. */
					break; /* remove the key */
				}
			}
		}
		if (!c) {
			/* ptrs[0] in one of the other arguments */
			/* delete all entries with value as ptrs[0] */
			for (;;) {
				p = *ptrs[0];
				if (p->nKeyLength == 0) {
					zend_hash_index_del(Z_ARRVAL_P(return_value), p->h);
				} else {
					zend_hash_quick_del(Z_ARRVAL_P(return_value), p->arKey, p->nKeyLength, p->h);
				}
				if (!*++ptrs[0]) {
					goto out;
				}
				if (behavior == DIFF_NORMAL) {
					if (diff_data_compare_func(ptrs[0] - 1, ptrs[0] TSRMLS_CC)) {
						break;
					}
				} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
					/* in this case no array_key_compare is needed */
					break;
				}
			}
		} else {
			/* ptrs[0] in none of the other arguments */
			/* skip all entries with value as ptrs[0] */
			for (;;) {
				if (!*++ptrs[0]) {
					goto out;
				}
				if (behavior == DIFF_NORMAL) {
					if (diff_data_compare_func(ptrs[0] - 1, ptrs[0] TSRMLS_CC)) {
						break;
					}
				} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
					/* in this case no array_key_compare is needed */
					break;
				}
			}
		}
	}
out:
	for (i = 0; i < arr_argc; i++) {
		hash = Z_ARRVAL_PP(args[i]);
		pefree(lists[i], hash->persistent);
	}

	PHP_ARRAY_CMP_FUNC_RESTORE();

	efree(ptrs);
	efree(lists);
	efree(args);
}
/* }}} */

/* {{{ proto array array_diff_key(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have keys which are not present in any of the others arguments. This function is like array_diff() but works on the keys instead of the values. The associativity is preserved. */
PHP_FUNCTION(array_diff_key)
{
	php_array_diff_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_COMP_DATA_NONE);
}
/* }}} */

/* {{{ proto array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
   Returns the entries of arr1 that have keys which are not present in any of the others arguments. User supplied function is used for comparing the keys. This function is like array_udiff() but works on the keys instead of the values. The associativity is preserved. */
PHP_FUNCTION(array_diff_ukey)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_KEY, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_USER);
}
/* }}} */

/* {{{ proto array array_diff(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have values which are not present in any of the others arguments. */
PHP_FUNCTION(array_diff)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_NORMAL, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_INTERNAL);
}
/* }}} */

/* {{{ proto array array_udiff(array arr1, array arr2 [, array ...], callback data_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments. Elements are compared by user supplied function. */
PHP_FUNCTION(array_udiff)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_NORMAL, DIFF_COMP_DATA_USER, DIFF_COMP_KEY_INTERNAL);
}
/* }}} */

/* {{{ proto array array_diff_assoc(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal */
PHP_FUNCTION(array_diff_assoc)
{
	php_array_diff_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_COMP_DATA_INTERNAL);
}
/* }}} */

/* {{{ proto array array_diff_uassoc(array arr1, array arr2 [, array ...], callback data_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Elements are compared by user supplied function. */
PHP_FUNCTION(array_diff_uassoc)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_ASSOC, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_USER);
}
/* }}} */

/* {{{ proto array array_udiff_assoc(array arr1, array arr2 [, array ...], callback key_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys are compared by user supplied function. */
PHP_FUNCTION(array_udiff_assoc)
{
	php_array_diff_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_COMP_DATA_USER);
}
/* }}} */

/* {{{ proto array array_udiff_uassoc(array arr1, array arr2 [, array ...], callback data_comp_func, callback key_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys and elements are compared by user supplied functions. */
PHP_FUNCTION(array_udiff_uassoc)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_ASSOC, DIFF_COMP_DATA_USER, DIFF_COMP_KEY_USER);
}
/* }}} */

#define MULTISORT_ORDER	0
#define MULTISORT_TYPE	1
#define MULTISORT_LAST	2

PHPAPI int php_multisort_compare(const void *a, const void *b TSRMLS_DC) /* {{{ */
{
	Bucket **ab = *(Bucket ***)a;
	Bucket **bb = *(Bucket ***)b;
	int r;
	int result = 0;
	zval temp;

	r = 0;
	do {
		php_set_compare_func(ARRAYG(multisort_flags)[MULTISORT_TYPE][r] TSRMLS_CC);

		ARRAYG(compare_func)(&temp, *((zval **)ab[r]->pData), *((zval **)bb[r]->pData) TSRMLS_CC);
		result = ARRAYG(multisort_flags)[MULTISORT_ORDER][r] * Z_LVAL(temp);
		if (result != 0) {
			return result;
		}
		r++;
	} while (ab[r] != NULL);

	return result;
}
/* }}} */

#define MULTISORT_ABORT						\
	for (k = 0; k < MULTISORT_LAST; k++)	\
		efree(ARRAYG(multisort_flags)[k]);	\
	efree(arrays);							\
	efree(args);							\
	RETURN_FALSE;

/* {{{ proto bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
   Sort multiple arrays at once similar to how ORDER BY clause works in SQL */
PHP_FUNCTION(array_multisort)
{
	zval***			args;
	zval***			arrays;
	Bucket***		indirect;
	Bucket*			p;
	HashTable*		hash;
	int				argc;
	int				array_size;
	int				num_arrays = 0;
	int				parse_state[MULTISORT_LAST];   /* 0 - flag not allowed 1 - flag allowed */
	int				sort_order = PHP_SORT_ASC;
	int				sort_type  = PHP_SORT_REGULAR;
	int				i, k;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc) == FAILURE) {
		return;
	}

	/* Allocate space for storing pointers to input arrays and sort flags. */
	arrays = (zval ***)ecalloc(argc, sizeof(zval **));
	for (i = 0; i < MULTISORT_LAST; i++) {
		parse_state[i] = 0;
		ARRAYG(multisort_flags)[i] = (int *)ecalloc(argc, sizeof(int));
	}

	/* Here we go through the input arguments and parse them. Each one can
	 * be either an array or a sort flag which follows an array. If not
	 * specified, the sort flags defaults to PHP_SORT_ASC and PHP_SORT_REGULAR
	 * accordingly. There can't be two sort flags of the same type after an
	 * array, and the very first argument has to be an array. */
	for (i = 0; i < argc; i++) {
		if (Z_TYPE_PP(args[i]) == IS_ARRAY) {
			/* We see the next array, so we update the sort flags of
			 * the previous array and reset the sort flags. */
			if (i > 0) {
				ARRAYG(multisort_flags)[MULTISORT_ORDER][num_arrays - 1] = sort_order;
				ARRAYG(multisort_flags)[MULTISORT_TYPE][num_arrays - 1] = sort_type;
				sort_order = PHP_SORT_ASC;
				sort_type = PHP_SORT_REGULAR;
			}
			arrays[num_arrays++] = args[i];

			/* Next one may be an array or a list of sort flags. */
			for (k = 0; k < MULTISORT_LAST; k++) {
				parse_state[k] = 1;
			}
		} else if (Z_TYPE_PP(args[i]) == IS_LONG) {
			switch (Z_LVAL_PP(args[i]) & ~PHP_SORT_FLAG_CASE) {
				case PHP_SORT_ASC:
				case PHP_SORT_DESC:
					/* flag allowed here */
					if (parse_state[MULTISORT_ORDER] == 1) {
						/* Save the flag and make sure then next arg is not the current flag. */
						sort_order = Z_LVAL_PP(args[i]) == PHP_SORT_DESC ? -1 : 1;
						parse_state[MULTISORT_ORDER] = 0;
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is expected to be an array or sorting flag that has not already been specified", i + 1);
						MULTISORT_ABORT;
					}
					break;

				case PHP_SORT_REGULAR:
				case PHP_SORT_NUMERIC:
				case PHP_SORT_STRING:
				case PHP_SORT_NATURAL:
#if HAVE_STRCOLL
				case PHP_SORT_LOCALE_STRING:
#endif
					/* flag allowed here */
					if (parse_state[MULTISORT_TYPE] == 1) {
						/* Save the flag and make sure then next arg is not the current flag. */
						sort_type = Z_LVAL_PP(args[i]);
						parse_state[MULTISORT_TYPE] = 0;
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is expected to be an array or sorting flag that has not already been specified", i + 1);
						MULTISORT_ABORT;
					}
					break;

				default:
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is an unknown sort flag", i + 1);
					MULTISORT_ABORT;
					break;

			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is expected to be an array or a sort flag", i + 1);
			MULTISORT_ABORT;
		}
	}
	/* Take care of the last array sort flags. */
	ARRAYG(multisort_flags)[MULTISORT_ORDER][num_arrays - 1] = sort_order;
	ARRAYG(multisort_flags)[MULTISORT_TYPE][num_arrays - 1] = sort_type;

	/* Make sure the arrays are of the same size. */
	array_size = zend_hash_num_elements(Z_ARRVAL_PP(arrays[0]));
	for (i = 0; i < num_arrays; i++) {
		if (zend_hash_num_elements(Z_ARRVAL_PP(arrays[i])) != array_size) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Array sizes are inconsistent");
			MULTISORT_ABORT;
		}
	}

	/* If all arrays are empty we don't need to do anything. */
	if (array_size < 1) {
		for (k = 0; k < MULTISORT_LAST; k++) {
			efree(ARRAYG(multisort_flags)[k]);
		}
		efree(arrays);
		efree(args);
		RETURN_TRUE;
	}

	/* Create the indirection array. This array is of size MxN, where
	 * M is the number of entries in each input array and N is the number
	 * of the input arrays + 1. The last column is NULL to indicate the end
	 * of the row. */
	indirect = (Bucket ***)safe_emalloc(array_size, sizeof(Bucket **), 0);
	for (i = 0; i < array_size; i++) {
		indirect[i] = (Bucket **)safe_emalloc((num_arrays + 1), sizeof(Bucket *), 0);
	}
	for (i = 0; i < num_arrays; i++) {
		k = 0;
		for (p = Z_ARRVAL_PP(arrays[i])->pListHead; p; p = p->pListNext, k++) {
			indirect[k][i] = p;
		}
	}
	for (k = 0; k < array_size; k++) {
		indirect[k][num_arrays] = NULL;
	}

	/* Do the actual sort magic - bada-bim, bada-boom. */
	zend_qsort(indirect, array_size, sizeof(Bucket **), php_multisort_compare TSRMLS_CC);

	/* Restructure the arrays based on sorted indirect - this is mostly taken from zend_hash_sort() function. */
	HANDLE_BLOCK_INTERRUPTIONS();
	for (i = 0; i < num_arrays; i++) {
		hash = Z_ARRVAL_PP(arrays[i]);
		hash->pListHead = indirect[0][i];;
		hash->pListTail = NULL;
		hash->pInternalPointer = hash->pListHead;

		for (k = 0; k < array_size; k++) {
			if (hash->pListTail) {
				hash->pListTail->pListNext = indirect[k][i];
			}
			indirect[k][i]->pListLast = hash->pListTail;
			indirect[k][i]->pListNext = NULL;
			hash->pListTail = indirect[k][i];
		}

		p = hash->pListHead;
		k = 0;
		while (p != NULL) {
			if (p->nKeyLength == 0)
				p->h = k++;
			p = p->pListNext;
		}
		hash->nNextFreeElement = array_size;
		zend_hash_rehash(hash);
	}
	HANDLE_UNBLOCK_INTERRUPTIONS();

	/* Clean up. */
	for (i = 0; i < array_size; i++) {
		efree(indirect[i]);
	}
	efree(indirect);
	for (k = 0; k < MULTISORT_LAST; k++) {
		efree(ARRAYG(multisort_flags)[k]);
	}
	efree(arrays);
	efree(args);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed array_rand(array input [, int num_req])
   Return key/keys for random entry/entries in the array */
PHP_FUNCTION(array_rand)
{
	zval *input;
	long randval, num_req = 1;
	int num_avail, key_type;
	char *string_key;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|l", &input, &num_req) == FAILURE) {
		return;
	}

	num_avail = zend_hash_num_elements(Z_ARRVAL_P(input));

	if (ZEND_NUM_ARGS() > 1) {
		if (num_req <= 0 || num_req > num_avail) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Second argument has to be between 1 and the number of elements in the array");
			return;
		}
	}

	/* Make the return value an array only if we need to pass back more than one result. */
	if (num_req > 1) {
		array_init_size(return_value, num_req);
	}

	/* We can't use zend_hash_index_find() because the array may have string keys or gaps. */
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
	while (num_req && (key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(input), &string_key, &string_key_len, &num_key, 0, &pos)) != HASH_KEY_NON_EXISTENT) {

		randval = php_rand(TSRMLS_C);

		if ((double) (randval / (PHP_RAND_MAX + 1.0)) < (double) num_req / (double) num_avail) {
			/* If we are returning a single result, just do it. */
			if (Z_TYPE_P(return_value) != IS_ARRAY) {
				if (key_type == HASH_KEY_IS_STRING) {
					RETURN_STRINGL(string_key, string_key_len - 1, 1);
				} else {
					RETURN_LONG(num_key);
				}
			} else {
				/* Append the result to the return value. */
				if (key_type == HASH_KEY_IS_STRING) {
					add_next_index_stringl(return_value, string_key, string_key_len - 1, 1);
				} else {
					add_next_index_long(return_value, num_key);
				}
			}
			num_req--;
		}
		num_avail--;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos);
	}
}
/* }}} */

/* {{{ proto mixed array_sum(array input)
   Returns the sum of the array entries */
PHP_FUNCTION(array_sum)
{
	zval *input,
		 **entry,
		 entry_n;
	HashPosition pos;
	double dval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &input) == FAILURE) {
		return;
	}

	ZVAL_LONG(return_value, 0);

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void **)&entry, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos)
	) {
		if (Z_TYPE_PP(entry) == IS_ARRAY || Z_TYPE_PP(entry) == IS_OBJECT) {
			continue;
		}
		entry_n = **entry;
		zval_copy_ctor(&entry_n);
		convert_scalar_to_number(&entry_n TSRMLS_CC);
		fast_add_function(return_value, return_value, &entry_n TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto mixed array_product(array input)
   Returns the product of the array entries */
PHP_FUNCTION(array_product)
{
	zval *input,
		 **entry,
		 entry_n;
	HashPosition pos;
	double dval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &input) == FAILURE) {
		return;
	}

	ZVAL_LONG(return_value, 1);
	if (!zend_hash_num_elements(Z_ARRVAL_P(input))) {
		return;
	}

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void **)&entry, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos)
	) {
		if (Z_TYPE_PP(entry) == IS_ARRAY || Z_TYPE_PP(entry) == IS_OBJECT) {
			continue;
		}
		entry_n = **entry;
		zval_copy_ctor(&entry_n);
		convert_scalar_to_number(&entry_n TSRMLS_CC);

		if (Z_TYPE(entry_n) == IS_LONG && Z_TYPE_P(return_value) == IS_LONG) {
			dval = (double)Z_LVAL_P(return_value) * (double)Z_LVAL(entry_n);
			if ( (double)LONG_MIN <= dval && dval <= (double)LONG_MAX ) {
				Z_LVAL_P(return_value) *= Z_LVAL(entry_n);
				continue;
			}
		}
		convert_to_double(return_value);
		convert_to_double(&entry_n);
		Z_DVAL_P(return_value) *= Z_DVAL(entry_n);
	}
}
/* }}} */

/* {{{ proto mixed array_reduce(array input, mixed callback [, mixed initial])
   Iteratively reduce the array to a single value via the callback. */
PHP_FUNCTION(array_reduce)
{
	zval *input;
	zval **args[2];
	zval **operand;
	zval *result = NULL;
	zval *retval;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	zval *initial = NULL;
	HashPosition pos;
	HashTable *htbl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "af|z", &input, &fci, &fci_cache, &initial) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() > 2) {
		ALLOC_ZVAL(result);
		MAKE_COPY_ZVAL(&initial, result);
	} else {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
	}

	/* (zval **)input points to an element of argument stack
	 * the base pointer of which is subject to change.
	 * thus we need to keep the pointer to the hashtable for safety */
	htbl = Z_ARRVAL_P(input);

	if (zend_hash_num_elements(htbl) == 0) {
		if (result) {
			RETVAL_ZVAL(result, 1, 1);
		}
		return;
	}

	fci.retval_ptr_ptr = &retval;
	fci.param_count = 2;
	fci.no_separation = 0;

	zend_hash_internal_pointer_reset_ex(htbl, &pos);
	while (zend_hash_get_current_data_ex(htbl, (void **)&operand, &pos) == SUCCESS) {

		if (result) {
			args[0] = &result;
			args[1] = operand;
			fci.params = args;

			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && retval) {
				zval_ptr_dtor(&result);
				result = retval;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the reduction callback");
				return;
			}
		} else {
			result = *operand;
			zval_add_ref(&result);
		}
		zend_hash_move_forward_ex(htbl, &pos);
	}
	RETVAL_ZVAL(result, 1, 1);
}
/* }}} */

/* {{{ proto array array_filter(array input [, mixed callback])
   Filters elements from the array via the callback. */
PHP_FUNCTION(array_filter)
{
	zval *array;
	zval **operand;
	zval **args[1];
	zval *retval = NULL;
	zend_bool have_callback = 0;
	char *string_key;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|f", &array, &fci, &fci_cache) == FAILURE) {
		return;
	}

	array_init(return_value);
	if (zend_hash_num_elements(Z_ARRVAL_P(array)) == 0) {
		return;
	}

	if (ZEND_NUM_ARGS() > 1) {
		have_callback = 1;
		fci.no_separation = 0;
		fci.retval_ptr_ptr = &retval;
		fci.param_count = 1;
	}

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(array), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(array), (void **)&operand, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(array), &pos)
	) {
		if (have_callback) {
			args[0] = operand;
			fci.params = args;

			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && retval) {
				if (!zend_is_true(retval)) {
					zval_ptr_dtor(&retval);
					continue;
				} else {
					zval_ptr_dtor(&retval);
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the filter callback");
				return;
			}
		} else if (!zend_is_true(*operand)) {
			continue;
		}

		zval_add_ref(operand);
		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(array), &string_key, &string_key_len, &num_key, 0, &pos)) {
			case HASH_KEY_IS_STRING:
				zend_hash_update(Z_ARRVAL_P(return_value), string_key, string_key_len, operand, sizeof(zval *), NULL);
				break;

			case HASH_KEY_IS_LONG:
				zend_hash_index_update(Z_ARRVAL_P(return_value), num_key, operand, sizeof(zval *), NULL);
				break;
		}
	}
}
/* }}} */

/* {{{ proto array array_map(mixed callback, array input1 [, array input2 ,...])
   Applies the callback to the elements in given arrays. */
PHP_FUNCTION(array_map)
{
	zval ***arrays = NULL;
	int n_arrays = 0;
	zval ***params;
	zval *result, *null;
	HashPosition *array_pos;
	zval **args;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	int i, k, maxlen = 0;
	int *array_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!+", &fci, &fci_cache, &arrays, &n_arrays) == FAILURE) {
		return;
	}

	RETVAL_NULL();

	args = (zval **)safe_emalloc(n_arrays, sizeof(zval *), 0);
	array_len = (int *)safe_emalloc(n_arrays, sizeof(int), 0);
	array_pos = (HashPosition *)safe_emalloc(n_arrays, sizeof(HashPosition), 0);

	for (i = 0; i < n_arrays; i++) {
		if (Z_TYPE_PP(arrays[i]) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d should be an array", i + 2);
			efree(arrays);
			efree(args);
			efree(array_len);
			efree(array_pos);
			return;
		}
		SEPARATE_ZVAL_IF_NOT_REF(arrays[i]);
		args[i] = *arrays[i];
		array_len[i] = zend_hash_num_elements(Z_ARRVAL_PP(arrays[i]));
		if (array_len[i] > maxlen) {
			maxlen = array_len[i];
		}
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(arrays[i]), &array_pos[i]);
	}

	efree(arrays);

	/* Short-circuit: if no callback and only one array, just return it. */
	if (!ZEND_FCI_INITIALIZED(fci) && n_arrays == 1) {
		RETVAL_ZVAL(args[0], 1, 0);
		efree(array_len);
		efree(array_pos);
		efree(args);
		return;
	}

	array_init_size(return_value, maxlen);
	params = (zval ***)safe_emalloc(n_arrays, sizeof(zval **), 0);
	MAKE_STD_ZVAL(null);
	ZVAL_NULL(null);

	/* We iterate through all the arrays at once. */
	for (k = 0; k < maxlen; k++) {
		uint str_key_len;
		ulong num_key;
		char *str_key;
		int key_type = 0;

		/* If no callback, the result will be an array, consisting of current
		 * entries from all arrays. */
		if (!ZEND_FCI_INITIALIZED(fci)) {
			MAKE_STD_ZVAL(result);
			array_init_size(result, n_arrays);
		}

		for (i = 0; i < n_arrays; i++) {
			/* If this array still has elements, add the current one to the
			 * parameter list, otherwise use null value. */
			if (k < array_len[i]) {
				zend_hash_get_current_data_ex(Z_ARRVAL_P(args[i]), (void **)&params[i], &array_pos[i]);

				/* It is safe to store only last value of key type, because
				 * this loop will run just once if there is only 1 array. */
				if (n_arrays == 1) {
					key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(args[0]), &str_key, &str_key_len, &num_key, 0, &array_pos[i]);
				}
				zend_hash_move_forward_ex(Z_ARRVAL_P(args[i]), &array_pos[i]);
			} else {
				params[i] = &null;
			}

			if (!ZEND_FCI_INITIALIZED(fci)) {
				zval_add_ref(params[i]);
				add_next_index_zval(result, *params[i]);
			}
		}

		if (ZEND_FCI_INITIALIZED(fci)) {
			fci.retval_ptr_ptr = &result;
			fci.param_count = n_arrays;
			fci.params = params;
			fci.no_separation = 0;

			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) != SUCCESS || !result) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the map callback");
				efree(array_len);
				efree(args);
				efree(array_pos);
				zval_dtor(return_value);
				zval_ptr_dtor(&null);
				efree(params);
				RETURN_NULL();
			}
		}

		if (n_arrays > 1) {
			add_next_index_zval(return_value, result);
		} else {
			if (key_type == HASH_KEY_IS_STRING) {
				add_assoc_zval_ex(return_value, str_key, str_key_len, result);
			} else {
				add_index_zval(return_value, num_key, result);
			}
		}
	}

	zval_ptr_dtor(&null);
	efree(params);
	efree(array_len);
	efree(array_pos);
	efree(args);
}
/* }}} */

/* {{{ proto bool array_key_exists(mixed key, array search)
   Checks if the given key or index exists in the array */
PHP_FUNCTION(array_key_exists)
{
	zval *key;					/* key to check for */
	HashTable *array;			/* array to check in */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zH", &key, &array) == FAILURE) {
		return;
	}

	switch (Z_TYPE_P(key)) {
		case IS_STRING:
			if (zend_symtable_exists(array, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1)) {
				RETURN_TRUE;
			}
			RETURN_FALSE;
		case IS_LONG:
			if (zend_hash_index_exists(array, Z_LVAL_P(key))) {
				RETURN_TRUE;
			}
			RETURN_FALSE;
		case IS_NULL:
			if (zend_hash_exists(array, "", 1)) {
				RETURN_TRUE;
			}
			RETURN_FALSE;

		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The first argument should be either a string or an integer");
			RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array array_chunk(array input, int size [, bool preserve_keys])
   Split array into chunks */
PHP_FUNCTION(array_chunk)
{
	int argc = ZEND_NUM_ARGS(), key_type, num_in;
	long size, current = 0;
	char *str_key;
	uint str_key_len;
	ulong num_key;
	zend_bool preserve_keys = 0;
	zval *input = NULL;
	zval *chunk = NULL;
	zval **entry;
	HashPosition pos;

	if (zend_parse_parameters(argc TSRMLS_CC, "al|b", &input, &size, &preserve_keys) == FAILURE) {
		return;
	}
	/* Do bounds checking for size parameter. */
	if (size < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Size parameter expected to be greater than 0");
		return;
	}

	num_in = zend_hash_num_elements(Z_ARRVAL_P(input));

	if (size > num_in) {
		size = num_in > 0 ? num_in : 1;
	}

	array_init_size(return_value, ((num_in - 1) / size) + 1);

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void**)&entry, &pos) == SUCCESS) {
		/* If new chunk, create and initialize it. */
		if (!chunk) {
			MAKE_STD_ZVAL(chunk);
			array_init_size(chunk, size);
		}

		/* Add entry to the chunk, preserving keys if necessary. */
		zval_add_ref(entry);

		if (preserve_keys) {
			key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(input), &str_key, &str_key_len, &num_key, 0, &pos);
			switch (key_type) {
				case HASH_KEY_IS_STRING:
					add_assoc_zval_ex(chunk, str_key, str_key_len, *entry);
					break;
				default:
					add_index_zval(chunk, num_key, *entry);
					break;
			}
		} else {
			add_next_index_zval(chunk, *entry);
		}

		/* If reached the chunk size, add it to the result array, and reset the
		 * pointer. */
		if (!(++current % size)) {
			add_next_index_zval(return_value, chunk);
			chunk = NULL;
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos);
	}

	/* Add the final chunk if there is one. */
	if (chunk) {
		add_next_index_zval(return_value, chunk);
	}
}
/* }}} */

/* {{{ proto array array_combine(array keys, array values)
   Creates an array by using the elements of the first parameter as keys and the elements of the second as the corresponding values */
PHP_FUNCTION(array_combine)
{
	zval *values, *keys;
	HashPosition pos_values, pos_keys;
	zval **entry_keys, **entry_values;
	int num_keys, num_values;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "aa", &keys, &values) == FAILURE) {
		return;
	}

	num_keys = zend_hash_num_elements(Z_ARRVAL_P(keys));
	num_values = zend_hash_num_elements(Z_ARRVAL_P(values));

	if (num_keys != num_values) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Both parameters should have an equal number of elements");
		RETURN_FALSE;
	}

	array_init_size(return_value, num_keys);

	if (!num_keys) {
		return;
	}

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(keys), &pos_keys);
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(values), &pos_values);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(keys), (void **)&entry_keys, &pos_keys) == SUCCESS &&
		zend_hash_get_current_data_ex(Z_ARRVAL_P(values), (void **)&entry_values, &pos_values) == SUCCESS
	) {
		if (Z_TYPE_PP(entry_keys) == IS_LONG) {
			zval_add_ref(entry_values);
			add_index_zval(return_value, Z_LVAL_PP(entry_keys), *entry_values);
		} else {
			zval key, *key_ptr = *entry_keys;

			if (Z_TYPE_PP(entry_keys) != IS_STRING) {
				key = **entry_keys;
				zval_copy_ctor(&key);
				convert_to_string(&key);
				key_ptr = &key;
			}

			zval_add_ref(entry_values);
			add_assoc_zval_ex(return_value, Z_STRVAL_P(key_ptr), Z_STRLEN_P(key_ptr) + 1, *entry_values);

			if (key_ptr != *entry_keys) {
				zval_dtor(&key);
			}
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(keys), &pos_keys);
		zend_hash_move_forward_ex(Z_ARRVAL_P(values), &pos_values);
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
{
	zval ***args = NULL;
	HashTable *hash;
	int arr_argc, i, c;
	Bucket ***lists, **list, ***ptrs, *p;
	int req_args;
	char *param_spec;
	zend_fcall_info fci1, fci2;
	zend_fcall_info_cache fci1_cache = empty_fcall_info_cache, fci2_cache = empty_fcall_info_cache;
	zend_fcall_info *fci_key, *fci_data;
	zend_fcall_info_cache *fci_key_cache, *fci_data_cache;
	PHP_ARRAY_CMP_FUNC_VARS;

	int (*diff_key_compare_func)(const void *, const void * TSRMLS_DC);
	int (*diff_data_compare_func)(const void *, const void * TSRMLS_DC);

	if (behavior == DIFF_NORMAL) {
		diff_key_compare_func = php_array_key_compare;

		if (data_compare_type == DIFF_COMP_DATA_INTERNAL) {
			/* array_diff */
			req_args = 2;
			param_spec = "+";
			diff_data_compare_func = php_array_data_compare;
		} else if (data_compare_type == DIFF_COMP_DATA_USER) {
			/* array_udiff */
			req_args = 3;
			param_spec = "+f";
			diff_data_compare_func = php_array_user_compare;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_compare_type is %d. This should never happen. Please report as a bug", data_compare_type);
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least %d parameters are required, %d given", req_args, ZEND_NUM_ARGS());
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache) == FAILURE) {
			return;
		}
		fci_data = &fci1;
		fci_data_cache = &fci1_cache;

	} else if (behavior & DIFF_ASSOC) { /* triggered also if DIFF_KEY */
		/* DIFF_KEY is subset of DIFF_ASSOC. When having the former
		 * no comparison of the data is done (part of DIFF_ASSOC) */

		if (data_compare_type == DIFF_COMP_DATA_INTERNAL && key_compare_type == DIFF_COMP_KEY_INTERNAL) {
			/* array_diff_assoc() or array_diff_key() */
			req_args = 2;
			param_spec = "+";
			diff_key_compare_func = php_array_key_compare;
			diff_data_compare_func = php_array_data_compare;
		} else if (data_compare_type == DIFF_COMP_DATA_USER && key_compare_type == DIFF_COMP_KEY_INTERNAL) {
			/* array_udiff_assoc() */
			req_args = 3;
			param_spec = "+f";
			diff_key_compare_func = php_array_key_compare;
			diff_data_compare_func = php_array_user_compare;
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
		} else if (data_compare_type == DIFF_COMP_DATA_INTERNAL && key_compare_type == DIFF_COMP_KEY_USER) {
			/* array_diff_uassoc() or array_diff_ukey() */
			req_args = 3;
			param_spec = "+f";
			diff_key_compare_func = php_array_user_key_compare;
			diff_data_compare_func = php_array_data_compare;
			fci_key = &fci1;
			fci_key_cache = &fci1_cache;
		} else if (data_compare_type == DIFF_COMP_DATA_USER && key_compare_type == DIFF_COMP_KEY_USER) {
			/* array_udiff_uassoc() */
			req_args = 4;
			param_spec = "+ff";
			diff_key_compare_func = php_array_user_key_compare;
			diff_data_compare_func = php_array_user_compare;
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
			fci_key = &fci2;
			fci_key_cache = &fci2_cache;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_compare_type is %d. key_compare_type is %d. This should never happen. Please report as a bug", data_compare_type, key_compare_type);
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "at least %d parameters are required, %d given", req_args, ZEND_NUM_ARGS());
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache, &fci2, &fci2_cache) == FAILURE) {
			return;
		}

	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "behavior is %d. This should never happen. Please report as a bug", behavior);
		return;
	}

	PHP_ARRAY_CMP_FUNC_BACKUP();

	/* for each argument, create and sort list with pointers to the hash buckets */
	lists = (Bucket ***)safe_emalloc(arr_argc, sizeof(Bucket **), 0);
	ptrs = (Bucket ***)safe_emalloc(arr_argc, sizeof(Bucket **), 0);
	php_set_compare_func(PHP_SORT_STRING TSRMLS_CC);

	if (behavior == DIFF_NORMAL && data_compare_type == DIFF_COMP_DATA_USER) {
		BG(user_compare_fci) = *fci_data;
		BG(user_compare_fci_cache) = *fci_data_cache;
	} else if (behavior & DIFF_ASSOC && key_compare_type == DIFF_COMP_KEY_USER) {
		BG(user_compare_fci) = *fci_key;
		BG(user_compare_fci_cache) = *fci_key_cache;
	}

	for (i = 0; i < arr_argc; i++) {
		if (Z_TYPE_PP(args[i]) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is not an array", i + 1);
			arr_argc = i; /* only free up to i - 1 */
			goto out;
		}
		hash = Z_ARRVAL_PP(args[i]);
		list = (Bucket **) pemalloc((hash->nNumOfElements + 1) * sizeof(Bucket *), hash->persistent);
		if (!list) {
			PHP_ARRAY_CMP_FUNC_RESTORE();

			efree(ptrs);
			efree(lists);
			efree(args);
			RETURN_FALSE;
		}
		lists[i] = list;
		ptrs[i] = list;
		for (p = hash->pListHead; p; p = p->pListNext) {
			*list++ = p;
		}
		*list = NULL;
		if (behavior == DIFF_NORMAL) {
			zend_qsort((void *) lists[i], hash->nNumOfElements, sizeof(Bucket *), diff_data_compare_func TSRMLS_CC);
		} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
			zend_qsort((void *) lists[i], hash->nNumOfElements, sizeof(Bucket *), diff_key_compare_func TSRMLS_CC);
		}
	}

	/* copy the argument array */
	RETVAL_ZVAL(*args[0], 1, 0);
	if (return_value->value.ht == &EG(symbol_table)) {
		HashTable *ht;
		zval *tmp;

		ALLOC_HASHTABLE(ht);
		zend_hash_init(ht, zend_hash_num_elements(return_value->value.ht), NULL, ZVAL_PTR_DTOR, 0);
		zend_hash_copy(ht, return_value->value.ht, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
		return_value->value.ht = ht;
	}

	/* go through the lists and look for values of ptr[0] that are not in the others */
	while (*ptrs[0]) {
		if ((behavior & DIFF_ASSOC) /* triggered also when DIFF_KEY */
			&&
			key_compare_type == DIFF_COMP_KEY_USER
		) {
			BG(user_compare_fci) = *fci_key;
			BG(user_compare_fci_cache) = *fci_key_cache;
		}
		c = 1;
		for (i = 1; i < arr_argc; i++) {
			Bucket **ptr = ptrs[i];
			if (behavior == DIFF_NORMAL) {
				while (*ptrs[i] && (0 < (c = diff_data_compare_func(ptrs[0], ptrs[i] TSRMLS_CC)))) {
					ptrs[i]++;
				}
			} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
				while (*ptr && (0 != (c = diff_key_compare_func(ptrs[0], ptr TSRMLS_CC)))) {
					ptr++;
				}
			}
			if (!c) {
				if (behavior == DIFF_NORMAL) {
					if (*ptrs[i]) {
						ptrs[i]++;
					}
					break;
				} else if (behavior == DIFF_ASSOC) {  /* only when DIFF_ASSOC */
					/* In this branch is execute only when DIFF_ASSOC. If behavior == DIFF_KEY
					 * data comparison is not needed - skipped. */
					if (*ptr) {
						if (data_compare_type == DIFF_COMP_DATA_USER) {
							BG(user_compare_fci) = *fci_data;
							BG(user_compare_fci_cache) = *fci_data_cache;
						}
						if (diff_data_compare_func(ptrs[0], ptr TSRMLS_CC) != 0) {
							/* the data is not the same */
							c = -1;
							if (key_compare_type == DIFF_COMP_KEY_USER) {
								BG(user_compare_fci) = *fci_key;
								BG(user_compare_fci_cache) = *fci_key_cache;
							}
						} else {
							break;
							/* we have found the element in other arrays thus we don't want it
							 * in the return_value -> delete from there */
						}
					}
				} else if (behavior == DIFF_KEY) { /* only when DIFF_KEY */
					/* the behavior here differs from INTERSECT_KEY in php_intersect
					 * since in the "diff" case we have to remove the entry from
					 * return_value while when doing intersection the entry must not
					 * be deleted. */
					break; /* remove the key */
				}
			}
		}
		if (!c) {
			/* ptrs[0] in one of the other arguments */
			/* delete all entries with value as ptrs[0] */
			for (;;) {
				p = *ptrs[0];
				if (p->nKeyLength == 0) {
					zend_hash_index_del(Z_ARRVAL_P(return_value), p->h);
				} else {
					zend_hash_quick_del(Z_ARRVAL_P(return_value), p->arKey, p->nKeyLength, p->h);
				}
				if (!*++ptrs[0]) {
					goto out;
				}
				if (behavior == DIFF_NORMAL) {
					if (diff_data_compare_func(ptrs[0] - 1, ptrs[0] TSRMLS_CC)) {
						break;
					}
				} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
					/* in this case no array_key_compare is needed */
					break;
				}
			}
		} else {
			/* ptrs[0] in none of the other arguments */
			/* skip all entries with value as ptrs[0] */
			for (;;) {
				if (!*++ptrs[0]) {
					goto out;
				}
				if (behavior == DIFF_NORMAL) {
					if (diff_data_compare_func(ptrs[0] - 1, ptrs[0] TSRMLS_CC)) {
						break;
					}
				} else if (behavior & DIFF_ASSOC) { /* triggered also when DIFF_KEY */
					/* in this case no array_key_compare is needed */
					break;
				}
			}
		}
	}
out:
	for (i = 0; i < arr_argc; i++) {
		hash = Z_ARRVAL_PP(args[i]);
		pefree(lists[i], hash->persistent);
	}

	PHP_ARRAY_CMP_FUNC_RESTORE();

	efree(ptrs);
	efree(lists);
	efree(args);
}
/* }}} */

/* {{{ proto array array_diff_key(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have keys which are not present in any of the others arguments. This function is like array_diff() but works on the keys instead of the values. The associativity is preserved. */
PHP_FUNCTION(array_diff_key)
{
	php_array_diff_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_COMP_DATA_NONE);
}
/* }}} */

/* {{{ proto array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
   Returns the entries of arr1 that have keys which are not present in any of the others arguments. User supplied function is used for comparing the keys. This function is like array_udiff() but works on the keys instead of the values. The associativity is preserved. */
PHP_FUNCTION(array_diff_ukey)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_KEY, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_USER);
}
/* }}} */

/* {{{ proto array array_diff(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have values which are not present in any of the others arguments. */
PHP_FUNCTION(array_diff)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_NORMAL, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_INTERNAL);
}
/* }}} */

/* {{{ proto array array_udiff(array arr1, array arr2 [, array ...], callback data_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments. Elements are compared by user supplied function. */
PHP_FUNCTION(array_udiff)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_NORMAL, DIFF_COMP_DATA_USER, DIFF_COMP_KEY_INTERNAL);
}
/* }}} */

/* {{{ proto array array_diff_assoc(array arr1, array arr2 [, array ...])
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal */
PHP_FUNCTION(array_diff_assoc)
{
	php_array_diff_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_COMP_DATA_INTERNAL);
}
/* }}} */

/* {{{ proto array array_diff_uassoc(array arr1, array arr2 [, array ...], callback data_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Elements are compared by user supplied function. */
PHP_FUNCTION(array_diff_uassoc)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_ASSOC, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_USER);
}
/* }}} */

/* {{{ proto array array_udiff_assoc(array arr1, array arr2 [, array ...], callback key_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys are compared by user supplied function. */
PHP_FUNCTION(array_udiff_assoc)
{
	php_array_diff_key(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_COMP_DATA_USER);
}
/* }}} */

/* {{{ proto array array_udiff_uassoc(array arr1, array arr2 [, array ...], callback data_comp_func, callback key_comp_func)
   Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys and elements are compared by user supplied functions. */
PHP_FUNCTION(array_udiff_uassoc)
{
	php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_ASSOC, DIFF_COMP_DATA_USER, DIFF_COMP_KEY_USER);
}
/* }}} */

#define MULTISORT_ORDER	0
#define MULTISORT_TYPE	1
#define MULTISORT_LAST	2

PHPAPI int php_multisort_compare(const void *a, const void *b TSRMLS_DC) /* {{{ */
{
	Bucket **ab = *(Bucket ***)a;
	Bucket **bb = *(Bucket ***)b;
	int r;
	int result = 0;
	zval temp;

	r = 0;
	do {
		php_set_compare_func(ARRAYG(multisort_flags)[MULTISORT_TYPE][r] TSRMLS_CC);

		ARRAYG(compare_func)(&temp, *((zval **)ab[r]->pData), *((zval **)bb[r]->pData) TSRMLS_CC);
		result = ARRAYG(multisort_flags)[MULTISORT_ORDER][r] * Z_LVAL(temp);
		if (result != 0) {
			return result;
		}
		r++;
	} while (ab[r] != NULL);

	return result;
}
/* }}} */

#define MULTISORT_ABORT						\
	for (k = 0; k < MULTISORT_LAST; k++)	\
		efree(ARRAYG(multisort_flags)[k]);	\
	efree(arrays);							\
	efree(args);							\
	RETURN_FALSE;

/* {{{ proto bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
   Sort multiple arrays at once similar to how ORDER BY clause works in SQL */
PHP_FUNCTION(array_multisort)
{
	zval***			args;
	zval***			arrays;
	Bucket***		indirect;
	Bucket*			p;
	HashTable*		hash;
	int				argc;
	int				array_size;
	int				num_arrays = 0;
	int				parse_state[MULTISORT_LAST];   /* 0 - flag not allowed 1 - flag allowed */
	int				sort_order = PHP_SORT_ASC;
	int				sort_type  = PHP_SORT_REGULAR;
	int				i, k;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc) == FAILURE) {
		return;
	}

	/* Allocate space for storing pointers to input arrays and sort flags. */
	arrays = (zval ***)ecalloc(argc, sizeof(zval **));
	for (i = 0; i < MULTISORT_LAST; i++) {
		parse_state[i] = 0;
		ARRAYG(multisort_flags)[i] = (int *)ecalloc(argc, sizeof(int));
	}

	/* Here we go through the input arguments and parse them. Each one can
	 * be either an array or a sort flag which follows an array. If not
	 * specified, the sort flags defaults to PHP_SORT_ASC and PHP_SORT_REGULAR
	 * accordingly. There can't be two sort flags of the same type after an
	 * array, and the very first argument has to be an array. */
	for (i = 0; i < argc; i++) {
		if (Z_TYPE_PP(args[i]) == IS_ARRAY) {
			/* We see the next array, so we update the sort flags of
			 * the previous array and reset the sort flags. */
			if (i > 0) {
				ARRAYG(multisort_flags)[MULTISORT_ORDER][num_arrays - 1] = sort_order;
				ARRAYG(multisort_flags)[MULTISORT_TYPE][num_arrays - 1] = sort_type;
				sort_order = PHP_SORT_ASC;
				sort_type = PHP_SORT_REGULAR;
			}
			arrays[num_arrays++] = args[i];

			/* Next one may be an array or a list of sort flags. */
			for (k = 0; k < MULTISORT_LAST; k++) {
				parse_state[k] = 1;
			}
		} else if (Z_TYPE_PP(args[i]) == IS_LONG) {
			switch (Z_LVAL_PP(args[i]) & ~PHP_SORT_FLAG_CASE) {
				case PHP_SORT_ASC:
				case PHP_SORT_DESC:
					/* flag allowed here */
					if (parse_state[MULTISORT_ORDER] == 1) {
						/* Save the flag and make sure then next arg is not the current flag. */
						sort_order = Z_LVAL_PP(args[i]) == PHP_SORT_DESC ? -1 : 1;
						parse_state[MULTISORT_ORDER] = 0;
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is expected to be an array or sorting flag that has not already been specified", i + 1);
						MULTISORT_ABORT;
					}
					break;

				case PHP_SORT_REGULAR:
				case PHP_SORT_NUMERIC:
				case PHP_SORT_STRING:
				case PHP_SORT_NATURAL:
#if HAVE_STRCOLL
				case PHP_SORT_LOCALE_STRING:
#endif
					/* flag allowed here */
					if (parse_state[MULTISORT_TYPE] == 1) {
						/* Save the flag and make sure then next arg is not the current flag. */
						sort_type = Z_LVAL_PP(args[i]);
						parse_state[MULTISORT_TYPE] = 0;
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is expected to be an array or sorting flag that has not already been specified", i + 1);
						MULTISORT_ABORT;
					}
					break;

				default:
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is an unknown sort flag", i + 1);
					MULTISORT_ABORT;
					break;

			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d is expected to be an array or a sort flag", i + 1);
			MULTISORT_ABORT;
		}
	}
	/* Take care of the last array sort flags. */
	ARRAYG(multisort_flags)[MULTISORT_ORDER][num_arrays - 1] = sort_order;
	ARRAYG(multisort_flags)[MULTISORT_TYPE][num_arrays - 1] = sort_type;

	/* Make sure the arrays are of the same size. */
	array_size = zend_hash_num_elements(Z_ARRVAL_PP(arrays[0]));
	for (i = 0; i < num_arrays; i++) {
		if (zend_hash_num_elements(Z_ARRVAL_PP(arrays[i])) != array_size) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Array sizes are inconsistent");
			MULTISORT_ABORT;
		}
	}

	/* If all arrays are empty we don't need to do anything. */
	if (array_size < 1) {
		for (k = 0; k < MULTISORT_LAST; k++) {
			efree(ARRAYG(multisort_flags)[k]);
		}
		efree(arrays);
		efree(args);
		RETURN_TRUE;
	}

	/* Create the indirection array. This array is of size MxN, where
	 * M is the number of entries in each input array and N is the number
	 * of the input arrays + 1. The last column is NULL to indicate the end
	 * of the row. */
	indirect = (Bucket ***)safe_emalloc(array_size, sizeof(Bucket **), 0);
	for (i = 0; i < array_size; i++) {
		indirect[i] = (Bucket **)safe_emalloc((num_arrays + 1), sizeof(Bucket *), 0);
	}
	for (i = 0; i < num_arrays; i++) {
		k = 0;
		for (p = Z_ARRVAL_PP(arrays[i])->pListHead; p; p = p->pListNext, k++) {
			indirect[k][i] = p;
		}
	}
	for (k = 0; k < array_size; k++) {
		indirect[k][num_arrays] = NULL;
	}

	/* Do the actual sort magic - bada-bim, bada-boom. */
	zend_qsort(indirect, array_size, sizeof(Bucket **), php_multisort_compare TSRMLS_CC);

	/* Restructure the arrays based on sorted indirect - this is mostly taken from zend_hash_sort() function. */
	HANDLE_BLOCK_INTERRUPTIONS();
	for (i = 0; i < num_arrays; i++) {
		hash = Z_ARRVAL_PP(arrays[i]);
		hash->pListHead = indirect[0][i];;
		hash->pListTail = NULL;
		hash->pInternalPointer = hash->pListHead;

		for (k = 0; k < array_size; k++) {
			if (hash->pListTail) {
				hash->pListTail->pListNext = indirect[k][i];
			}
			indirect[k][i]->pListLast = hash->pListTail;
			indirect[k][i]->pListNext = NULL;
			hash->pListTail = indirect[k][i];
		}

		p = hash->pListHead;
		k = 0;
		while (p != NULL) {
			if (p->nKeyLength == 0)
				p->h = k++;
			p = p->pListNext;
		}
		hash->nNextFreeElement = array_size;
		zend_hash_rehash(hash);
	}
	HANDLE_UNBLOCK_INTERRUPTIONS();

	/* Clean up. */
	for (i = 0; i < array_size; i++) {
		efree(indirect[i]);
	}
	efree(indirect);
	for (k = 0; k < MULTISORT_LAST; k++) {
		efree(ARRAYG(multisort_flags)[k]);
	}
	efree(arrays);
	efree(args);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed array_rand(array input [, int num_req])
   Return key/keys for random entry/entries in the array */
PHP_FUNCTION(array_rand)
{
	zval *input;
	long randval, num_req = 1;
	int num_avail, key_type;
	char *string_key;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|l", &input, &num_req) == FAILURE) {
		return;
	}

	num_avail = zend_hash_num_elements(Z_ARRVAL_P(input));

	if (ZEND_NUM_ARGS() > 1) {
		if (num_req <= 0 || num_req > num_avail) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Second argument has to be between 1 and the number of elements in the array");
			return;
		}
	}

	/* Make the return value an array only if we need to pass back more than one result. */
	if (num_req > 1) {
		array_init_size(return_value, num_req);
	}

	/* We can't use zend_hash_index_find() because the array may have string keys or gaps. */
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
	while (num_req && (key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(input), &string_key, &string_key_len, &num_key, 0, &pos)) != HASH_KEY_NON_EXISTENT) {

		randval = php_rand(TSRMLS_C);

		if ((double) (randval / (PHP_RAND_MAX + 1.0)) < (double) num_req / (double) num_avail) {
			/* If we are returning a single result, just do it. */
			if (Z_TYPE_P(return_value) != IS_ARRAY) {
				if (key_type == HASH_KEY_IS_STRING) {
					RETURN_STRINGL(string_key, string_key_len - 1, 1);
				} else {
					RETURN_LONG(num_key);
				}
			} else {
				/* Append the result to the return value. */
				if (key_type == HASH_KEY_IS_STRING) {
					add_next_index_stringl(return_value, string_key, string_key_len - 1, 1);
				} else {
					add_next_index_long(return_value, num_key);
				}
			}
			num_req--;
		}
		num_avail--;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos);
	}
}
/* }}} */

/* {{{ proto mixed array_sum(array input)
   Returns the sum of the array entries */
PHP_FUNCTION(array_sum)
{
	zval *input,
		 **entry,
		 entry_n;
	HashPosition pos;
	double dval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &input) == FAILURE) {
		return;
	}

	ZVAL_LONG(return_value, 0);

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void **)&entry, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos)
	) {
		if (Z_TYPE_PP(entry) == IS_ARRAY || Z_TYPE_PP(entry) == IS_OBJECT) {
			continue;
		}
		entry_n = **entry;
		zval_copy_ctor(&entry_n);
		convert_scalar_to_number(&entry_n TSRMLS_CC);
		fast_add_function(return_value, return_value, &entry_n TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto mixed array_product(array input)
   Returns the product of the array entries */
PHP_FUNCTION(array_product)
{
	zval *input,
		 **entry,
		 entry_n;
	HashPosition pos;
	double dval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &input) == FAILURE) {
		return;
	}

	ZVAL_LONG(return_value, 1);
	if (!zend_hash_num_elements(Z_ARRVAL_P(input))) {
		return;
	}

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void **)&entry, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos)
	) {
		if (Z_TYPE_PP(entry) == IS_ARRAY || Z_TYPE_PP(entry) == IS_OBJECT) {
			continue;
		}
		entry_n = **entry;
		zval_copy_ctor(&entry_n);
		convert_scalar_to_number(&entry_n TSRMLS_CC);

		if (Z_TYPE(entry_n) == IS_LONG && Z_TYPE_P(return_value) == IS_LONG) {
			dval = (double)Z_LVAL_P(return_value) * (double)Z_LVAL(entry_n);
			if ( (double)LONG_MIN <= dval && dval <= (double)LONG_MAX ) {
				Z_LVAL_P(return_value) *= Z_LVAL(entry_n);
				continue;
			}
		}
		convert_to_double(return_value);
		convert_to_double(&entry_n);
		Z_DVAL_P(return_value) *= Z_DVAL(entry_n);
	}
}
/* }}} */

/* {{{ proto mixed array_reduce(array input, mixed callback [, mixed initial])
   Iteratively reduce the array to a single value via the callback. */
PHP_FUNCTION(array_reduce)
{
	zval *input;
	zval **args[2];
	zval **operand;
	zval *result = NULL;
	zval *retval;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	zval *initial = NULL;
	HashPosition pos;
	HashTable *htbl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "af|z", &input, &fci, &fci_cache, &initial) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() > 2) {
		ALLOC_ZVAL(result);
		MAKE_COPY_ZVAL(&initial, result);
	} else {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
	}

	/* (zval **)input points to an element of argument stack
	 * the base pointer of which is subject to change.
	 * thus we need to keep the pointer to the hashtable for safety */
	htbl = Z_ARRVAL_P(input);

	if (zend_hash_num_elements(htbl) == 0) {
		if (result) {
			RETVAL_ZVAL(result, 1, 1);
		}
		return;
	}

	fci.retval_ptr_ptr = &retval;
	fci.param_count = 2;
	fci.no_separation = 0;

	zend_hash_internal_pointer_reset_ex(htbl, &pos);
	while (zend_hash_get_current_data_ex(htbl, (void **)&operand, &pos) == SUCCESS) {

		if (result) {
			args[0] = &result;
			args[1] = operand;
			fci.params = args;

			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && retval) {
				zval_ptr_dtor(&result);
				result = retval;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the reduction callback");
				return;
			}
		} else {
			result = *operand;
			zval_add_ref(&result);
		}
		zend_hash_move_forward_ex(htbl, &pos);
	}
	RETVAL_ZVAL(result, 1, 1);
}
/* }}} */

/* {{{ proto array array_filter(array input [, mixed callback])
   Filters elements from the array via the callback. */
PHP_FUNCTION(array_filter)
{
	zval *array;
	zval **operand;
	zval **args[1];
	zval *retval = NULL;
	zend_bool have_callback = 0;
	char *string_key;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|f", &array, &fci, &fci_cache) == FAILURE) {
		return;
	}

	array_init(return_value);
	if (zend_hash_num_elements(Z_ARRVAL_P(array)) == 0) {
		return;
	}

	if (ZEND_NUM_ARGS() > 1) {
		have_callback = 1;
		fci.no_separation = 0;
		fci.retval_ptr_ptr = &retval;
		fci.param_count = 1;
	}

	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(array), &pos);
		zend_hash_get_current_data_ex(Z_ARRVAL_P(array), (void **)&operand, &pos) == SUCCESS;
		zend_hash_move_forward_ex(Z_ARRVAL_P(array), &pos)
	) {
		if (have_callback) {
			args[0] = operand;
			fci.params = args;

			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && retval) {
				if (!zend_is_true(retval)) {
					zval_ptr_dtor(&retval);
					continue;
				} else {
					zval_ptr_dtor(&retval);
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the filter callback");
				return;
			}
		} else if (!zend_is_true(*operand)) {
			continue;
		}

		zval_add_ref(operand);
		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(array), &string_key, &string_key_len, &num_key, 0, &pos)) {
			case HASH_KEY_IS_STRING:
				zend_hash_update(Z_ARRVAL_P(return_value), string_key, string_key_len, operand, sizeof(zval *), NULL);
				break;

			case HASH_KEY_IS_LONG:
				zend_hash_index_update(Z_ARRVAL_P(return_value), num_key, operand, sizeof(zval *), NULL);
				break;
		}
	}
}
/* }}} */

/* {{{ proto array array_map(mixed callback, array input1 [, array input2 ,...])
   Applies the callback to the elements in given arrays. */
PHP_FUNCTION(array_map)
{
	zval ***arrays = NULL;
	int n_arrays = 0;
	zval ***params;
	zval *result, *null;
	HashPosition *array_pos;
	zval **args;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	int i, k, maxlen = 0;
	int *array_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!+", &fci, &fci_cache, &arrays, &n_arrays) == FAILURE) {
		return;
	}

	RETVAL_NULL();

	args = (zval **)safe_emalloc(n_arrays, sizeof(zval *), 0);
	array_len = (int *)safe_emalloc(n_arrays, sizeof(int), 0);
	array_pos = (HashPosition *)safe_emalloc(n_arrays, sizeof(HashPosition), 0);

	for (i = 0; i < n_arrays; i++) {
		if (Z_TYPE_PP(arrays[i]) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument #%d should be an array", i + 2);
			efree(arrays);
			efree(args);
			efree(array_len);
			efree(array_pos);
			return;
		}
		SEPARATE_ZVAL_IF_NOT_REF(arrays[i]);
		args[i] = *arrays[i];
		array_len[i] = zend_hash_num_elements(Z_ARRVAL_PP(arrays[i]));
		if (array_len[i] > maxlen) {
			maxlen = array_len[i];
		}
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(arrays[i]), &array_pos[i]);
	}

	efree(arrays);

	/* Short-circuit: if no callback and only one array, just return it. */
	if (!ZEND_FCI_INITIALIZED(fci) && n_arrays == 1) {
		RETVAL_ZVAL(args[0], 1, 0);
		efree(array_len);
		efree(array_pos);
		efree(args);
		return;
	}

	array_init_size(return_value, maxlen);
	params = (zval ***)safe_emalloc(n_arrays, sizeof(zval **), 0);
	MAKE_STD_ZVAL(null);
	ZVAL_NULL(null);

	/* We iterate through all the arrays at once. */
	for (k = 0; k < maxlen; k++) {
		uint str_key_len;
		ulong num_key;
		char *str_key;
		int key_type = 0;

		/* If no callback, the result will be an array, consisting of current
		 * entries from all arrays. */
		if (!ZEND_FCI_INITIALIZED(fci)) {
			MAKE_STD_ZVAL(result);
			array_init_size(result, n_arrays);
		}

		for (i = 0; i < n_arrays; i++) {
			/* If this array still has elements, add the current one to the
			 * parameter list, otherwise use null value. */
			if (k < array_len[i]) {
				zend_hash_get_current_data_ex(Z_ARRVAL_P(args[i]), (void **)&params[i], &array_pos[i]);

				/* It is safe to store only last value of key type, because
				 * this loop will run just once if there is only 1 array. */
				if (n_arrays == 1) {
					key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(args[0]), &str_key, &str_key_len, &num_key, 0, &array_pos[i]);
				}
				zend_hash_move_forward_ex(Z_ARRVAL_P(args[i]), &array_pos[i]);
			} else {
				params[i] = &null;
			}

			if (!ZEND_FCI_INITIALIZED(fci)) {
				zval_add_ref(params[i]);
				add_next_index_zval(result, *params[i]);
			}
		}

		if (ZEND_FCI_INITIALIZED(fci)) {
			fci.retval_ptr_ptr = &result;
			fci.param_count = n_arrays;
			fci.params = params;
			fci.no_separation = 0;

			if (zend_call_function(&fci, &fci_cache TSRMLS_CC) != SUCCESS || !result) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the map callback");
				efree(array_len);
				efree(args);
				efree(array_pos);
				zval_dtor(return_value);
				zval_ptr_dtor(&null);
				efree(params);
				RETURN_NULL();
			}
		}

		if (n_arrays > 1) {
			add_next_index_zval(return_value, result);
		} else {
			if (key_type == HASH_KEY_IS_STRING) {
				add_assoc_zval_ex(return_value, str_key, str_key_len, result);
			} else {
				add_index_zval(return_value, num_key, result);
			}
		}
	}

	zval_ptr_dtor(&null);
	efree(params);
	efree(array_len);
	efree(array_pos);
	efree(args);
}
/* }}} */

/* {{{ proto bool array_key_exists(mixed key, array search)
   Checks if the given key or index exists in the array */
PHP_FUNCTION(array_key_exists)
{
	zval *key;					/* key to check for */
	HashTable *array;			/* array to check in */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zH", &key, &array) == FAILURE) {
		return;
	}

	switch (Z_TYPE_P(key)) {
		case IS_STRING:
			if (zend_symtable_exists(array, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1)) {
				RETURN_TRUE;
			}
			RETURN_FALSE;
		case IS_LONG:
			if (zend_hash_index_exists(array, Z_LVAL_P(key))) {
				RETURN_TRUE;
			}
			RETURN_FALSE;
		case IS_NULL:
			if (zend_hash_exists(array, "", 1)) {
				RETURN_TRUE;
			}
			RETURN_FALSE;

		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The first argument should be either a string or an integer");
			RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array array_chunk(array input, int size [, bool preserve_keys])
   Split array into chunks */
PHP_FUNCTION(array_chunk)
{
	int argc = ZEND_NUM_ARGS(), key_type, num_in;
	long size, current = 0;
	char *str_key;
	uint str_key_len;
	ulong num_key;
	zend_bool preserve_keys = 0;
	zval *input = NULL;
	zval *chunk = NULL;
	zval **entry;
	HashPosition pos;

	if (zend_parse_parameters(argc TSRMLS_CC, "al|b", &input, &size, &preserve_keys) == FAILURE) {
		return;
	}
	/* Do bounds checking for size parameter. */
	if (size < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Size parameter expected to be greater than 0");
		return;
	}

	num_in = zend_hash_num_elements(Z_ARRVAL_P(input));

	if (size > num_in) {
		size = num_in > 0 ? num_in : 1;
	}

	array_init_size(return_value, ((num_in - 1) / size) + 1);

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void**)&entry, &pos) == SUCCESS) {
		/* If new chunk, create and initialize it. */
		if (!chunk) {
			MAKE_STD_ZVAL(chunk);
			array_init_size(chunk, size);
		}

		/* Add entry to the chunk, preserving keys if necessary. */
		zval_add_ref(entry);

		if (preserve_keys) {
			key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(input), &str_key, &str_key_len, &num_key, 0, &pos);
			switch (key_type) {
				case HASH_KEY_IS_STRING:
					add_assoc_zval_ex(chunk, str_key, str_key_len, *entry);
					break;
				default:
					add_index_zval(chunk, num_key, *entry);
					break;
			}
		} else {
			add_next_index_zval(chunk, *entry);
		}

		/* If reached the chunk size, add it to the result array, and reset the
		 * pointer. */
		if (!(++current % size)) {
			add_next_index_zval(return_value, chunk);
			chunk = NULL;
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos);
	}

	/* Add the final chunk if there is one. */
	if (chunk) {
		add_next_index_zval(return_value, chunk);
	}
}
/* }}} */

/* {{{ proto array array_combine(array keys, array values)
   Creates an array by using the elements of the first parameter as keys and the elements of the second as the corresponding values */
PHP_FUNCTION(array_combine)
{
	zval *values, *keys;
	HashPosition pos_values, pos_keys;
	zval **entry_keys, **entry_values;
	int num_keys, num_values;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "aa", &keys, &values) == FAILURE) {
		return;
	}

	num_keys = zend_hash_num_elements(Z_ARRVAL_P(keys));
	num_values = zend_hash_num_elements(Z_ARRVAL_P(values));

	if (num_keys != num_values) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Both parameters should have an equal number of elements");
		RETURN_FALSE;
	}

	array_init_size(return_value, num_keys);

	if (!num_keys) {
		return;
	}

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(keys), &pos_keys);
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(values), &pos_values);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(keys), (void **)&entry_keys, &pos_keys) == SUCCESS &&
		zend_hash_get_current_data_ex(Z_ARRVAL_P(values), (void **)&entry_values, &pos_values) == SUCCESS
	) {
		if (Z_TYPE_PP(entry_keys) == IS_LONG) {
			zval_add_ref(entry_values);
			add_index_zval(return_value, Z_LVAL_PP(entry_keys), *entry_values);
		} else {
			zval key, *key_ptr = *entry_keys;

			if (Z_TYPE_PP(entry_keys) != IS_STRING) {
				key = **entry_keys;
				zval_copy_ctor(&key);
				convert_to_string(&key);
				key_ptr = &key;
			}

			zval_add_ref(entry_values);
			add_assoc_zval_ex(return_value, Z_STRVAL_P(key_ptr), Z_STRLEN_P(key_ptr) + 1, *entry_values);

			if (key_ptr != *entry_keys) {
				zval_dtor(&key);
			}
		}

		zend_hash_move_forward_ex(Z_ARRVAL_P(keys), &pos_keys);
		zend_hash_move_forward_ex(Z_ARRVAL_P(values), &pos_values);
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
{
	zval *input,
		 **entry,
		 entry_n;
	HashPosition pos;
	double dval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &input) == FAILURE) {
		return;
	}