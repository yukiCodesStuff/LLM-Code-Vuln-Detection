typedef struct _user_tick_function_entry {
	zval **arguments;
	int arg_count;
	int calling;
} user_tick_function_entry;

/* some prototypes for local functions */
static void user_shutdown_function_dtor(php_shutdown_function_entry *shutdown_function_entry);
static void user_tick_function_dtor(user_tick_function_entry *tick_function_entry);

static HashTable basic_submodules;

#undef sprintf

/* {{{ arginfo */
/* {{{ main/main.c */
ZEND_BEGIN_ARG_INFO(arginfo_set_time_limit, 0)
	ZEND_ARG_INFO(0, seconds)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ main/sapi.c */
ZEND_BEGIN_ARG_INFO(arginfo_header_register_callback, 0)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ main/output.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ob_start, 0, 0, 0)
	ZEND_ARG_INFO(0, user_function)
	ZEND_ARG_INFO(0, chunk_size)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_flush, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_clean, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_end_flush, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_end_clean, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_get_flush, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_get_clean, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_get_contents, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_get_level, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_get_length, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ob_list_handlers, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ob_get_status, 0, 0, 0)
	ZEND_ARG_INFO(0, full_status)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ob_implicit_flush, 0, 0, 0)
	ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_output_reset_rewrite_vars, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_output_add_rewrite_var, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ main/streams/userspace.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_wrapper_register, 0, 0, 2)
	ZEND_ARG_INFO(0, protocol)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_wrapper_unregister, 0)
	ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_wrapper_restore, 0)
	ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ array.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_krsort, 0, 0, 1)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, sort_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ksort, 0, 0, 1)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, sort_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_count, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_natsort, 0)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_natcasesort, 0)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_asort, 0, 0, 1)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, sort_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_arsort, 0, 0, 1)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, sort_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sort, 0, 0, 1)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, sort_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rsort, 0, 0, 1)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, sort_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_usort, 0)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, cmp_function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_uasort, 0)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, cmp_function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_uksort, 0)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, cmp_function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_end, 0)
	ZEND_ARG_INFO(1, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_prev, 0)
	ZEND_ARG_INFO(1, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_next, 0)
	ZEND_ARG_INFO(1, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reset, 0)
	ZEND_ARG_INFO(1, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_current, ZEND_SEND_PREFER_REF)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_key, ZEND_SEND_PREFER_REF)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_min, 0, 0, 1)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, arg2)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_max, 0, 0, 1)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, arg2)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_walk, 0, 0, 2)
	ZEND_ARG_INFO(1, input) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, funcname)
	ZEND_ARG_INFO(0, userdata)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_walk_recursive, 0, 0, 2)
	ZEND_ARG_INFO(1, input) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, funcname)
	ZEND_ARG_INFO(0, userdata)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_in_array, 0, 0, 2)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, haystack) /* ARRAY_INFO(0, haystack, 0) */
	ZEND_ARG_INFO(0, strict)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_search, 0, 0, 2)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, haystack) /* ARRAY_INFO(0, haystack, 0) */
	ZEND_ARG_INFO(0, strict)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_extract, 0, 0, 1)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, extract_type)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_compact, 0, 0, 1)
	ZEND_ARG_INFO(0, var_names)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_fill, 0)
	ZEND_ARG_INFO(0, start_key)
	ZEND_ARG_INFO(0, num)
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_fill_keys, 0)
	ZEND_ARG_INFO(0, keys) /* ARRAY_INFO(0, keys, 0) */
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_range, 0, 0, 2)
	ZEND_ARG_INFO(0, low)
	ZEND_ARG_INFO(0, high)
	ZEND_ARG_INFO(0, step)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_shuffle, 0)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_push, 0, 0, 2)
	ZEND_ARG_INFO(1, stack) /* ARRAY_INFO(1, stack, 0) */
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_pop, 0)
	ZEND_ARG_INFO(1, stack) /* ARRAY_INFO(1, stack, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_shift, 0)
	ZEND_ARG_INFO(1, stack) /* ARRAY_INFO(1, stack, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_unshift, 0, 0, 2)
	ZEND_ARG_INFO(1, stack) /* ARRAY_INFO(1, stack, 0) */
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_splice, 0, 0, 2)
	ZEND_ARG_INFO(1, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, replacement) /* ARRAY_INFO(0, arg, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_slice, 0, 0, 2)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(1, arg, 0) */
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, preserve_keys)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_merge, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_merge_recursive, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_replace, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_replace_recursive, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_keys, 0, 0, 1)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, search_value)
	ZEND_ARG_INFO(0, strict)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_values, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_count_values, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_column, 0, 0, 2)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, column_key)
	ZEND_ARG_INFO(0, index_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_reverse, 0, 0, 1)
	ZEND_ARG_INFO(0, input) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, preserve_keys)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_pad, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, pad_size)
	ZEND_ARG_INFO(0, pad_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_flip, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_change_key_case, 0, 0, 1)
	ZEND_ARG_INFO(0, input) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, case)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_unique, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_intersect_key, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_intersect_ukey, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_key_compare_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_intersect, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_uintersect, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_data_compare_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_intersect_assoc, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_uintersect_assoc, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_data_compare_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_intersect_uassoc, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_key_compare_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_uintersect_uassoc, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_data_compare_func)
	ZEND_ARG_INFO(0, callback_key_compare_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_diff_key, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_diff_ukey, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_key_comp_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_diff, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_udiff, 0)
	ZEND_ARG_INFO(0, arr1)
	ZEND_ARG_INFO(0, arr2)
	ZEND_ARG_INFO(0, callback_data_comp_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_diff_assoc, 0, 0, 2)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, ...)  /* ARRAY_INFO(0, ..., 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_diff_uassoc, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_data_comp_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_udiff_assoc, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_key_comp_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_udiff_uassoc, 0)
	ZEND_ARG_INFO(0, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(0, arr2) /* ARRAY_INFO(0, arg2, 0) */
	ZEND_ARG_INFO(0, callback_data_comp_func)
	ZEND_ARG_INFO(0, callback_key_comp_func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_multisort, ZEND_SEND_PREFER_REF, 0, 1)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, arr1) /* ARRAY_INFO(0, arg1, 0) */
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, SORT_ASC_or_SORT_DESC)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, SORT_REGULAR_or_SORT_NUMERIC_or_SORT_STRING)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, arr2)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, SORT_ASC_or_SORT_DESC)
	ZEND_ARG_INFO(ZEND_SEND_PREFER_REF, SORT_REGULAR_or_SORT_NUMERIC_or_SORT_STRING)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_rand, 0, 0, 1)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, num_req)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_sum, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_product, 0)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_reduce, 0, 0, 2)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, initial)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_filter, 0, 0, 1)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_map, 0, 0, 2)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_key_exists, 0)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, search)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_chunk, 0, 0, 2)
	ZEND_ARG_INFO(0, arg) /* ARRAY_INFO(0, arg, 0) */
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, preserve_keys)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_combine, 0)
	ZEND_ARG_INFO(0, keys)   /* ARRAY_INFO(0, keys, 0) */
	ZEND_ARG_INFO(0, values) /* ARRAY_INFO(0, values, 0) */
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ basic_functions.c */
ZEND_BEGIN_ARG_INFO(arginfo_get_magic_quotes_gpc, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_get_magic_quotes_runtime, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_set_magic_quotes_runtime, 0, 0, 1)
	ZEND_ARG_INFO(0, new_setting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_constant, 0)
	ZEND_ARG_INFO(0, const_name)
ZEND_END_ARG_INFO()

#ifdef HAVE_INET_NTOP
ZEND_BEGIN_ARG_INFO(arginfo_inet_ntop, 0)
	ZEND_ARG_INFO(0, in_addr)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_INET_PTON
ZEND_BEGIN_ARG_INFO(arginfo_inet_pton, 0)
	ZEND_ARG_INFO(0, ip_address)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_ip2long, 0)
	ZEND_ARG_INFO(0, ip_address)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_long2ip, 0)
	ZEND_ARG_INFO(0, proper_address)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getenv, 0)
	ZEND_ARG_INFO(0, varname)
ZEND_END_ARG_INFO()

#ifdef HAVE_PUTENV
ZEND_BEGIN_ARG_INFO(arginfo_putenv, 0)
	ZEND_ARG_INFO(0, setting)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_getopt, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, opts) /* ARRAY_INFO(0, opts, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_flush, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sleep, 0)
	ZEND_ARG_INFO(0, seconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_usleep, 0)
	ZEND_ARG_INFO(0, micro_seconds)
ZEND_END_ARG_INFO()

#if HAVE_NANOSLEEP
ZEND_BEGIN_ARG_INFO(arginfo_time_nanosleep, 0)
	ZEND_ARG_INFO(0, seconds)
	ZEND_ARG_INFO(0, nanoseconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_time_sleep_until, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_get_current_user, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_get_cfg_var, 0)
	ZEND_ARG_INFO(0, option_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_error_log, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, message_type)
	ZEND_ARG_INFO(0, destination)
	ZEND_ARG_INFO(0, extra_headers)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_error_get_last, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_call_user_func, 0, 0, 1)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, parmeter)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_call_user_func_array, 0, 0, 2)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, parameters) /* ARRAY_INFO(0, parameters, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_call_user_method, 0, 0, 2)
	ZEND_ARG_INFO(0, method_name)
	ZEND_ARG_INFO(1, object)
	ZEND_ARG_INFO(0, parameter)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_call_user_method_array, 0)
	ZEND_ARG_INFO(0, method_name)
	ZEND_ARG_INFO(1, object)
	ZEND_ARG_INFO(0, params) /* ARRAY_INFO(0, params, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_forward_static_call, 0, 0, 1)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, parameter)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_forward_static_call_array, 0, 0, 2)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, parameters) /* ARRAY_INFO(0, parameters, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_register_shutdown_function, 0)
	ZEND_ARG_INFO(0, function_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_highlight_file, 0, 0, 1)
	ZEND_ARG_INFO(0, file_name)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_php_strip_whitespace, 0)
	ZEND_ARG_INFO(0, file_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_highlight_string, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ini_get, 0)
	ZEND_ARG_INFO(0, varname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ini_get_all, 0, 0, 0)
	ZEND_ARG_INFO(0, extension)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ini_set, 0)
	ZEND_ARG_INFO(0, varname)
	ZEND_ARG_INFO(0, newvalue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ini_restore, 0)
	ZEND_ARG_INFO(0, varname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_set_include_path, 0)
	ZEND_ARG_INFO(0, new_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_get_include_path, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_restore_include_path, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_print_r, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_connection_aborted, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_connection_status, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ignore_user_abort, 0, 0, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

#if HAVE_GETSERVBYNAME
ZEND_BEGIN_ARG_INFO(arginfo_getservbyname, 0)
	ZEND_ARG_INFO(0, service)
	ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()
#endif

#if HAVE_GETSERVBYPORT
ZEND_BEGIN_ARG_INFO(arginfo_getservbyport, 0)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()
#endif

#if HAVE_GETPROTOBYNAME
ZEND_BEGIN_ARG_INFO(arginfo_getprotobyname, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
#endif

#if HAVE_GETPROTOBYNUMBER
ZEND_BEGIN_ARG_INFO(arginfo_getprotobynumber, 0)
	ZEND_ARG_INFO(0, proto)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_register_tick_function, 0, 0, 1)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, arg)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_unregister_tick_function, 0)
	ZEND_ARG_INFO(0, function_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_uploaded_file, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_move_uploaded_file, 0)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, new_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_parse_ini_file, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, process_sections)
	ZEND_ARG_INFO(0, scanner_mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_parse_ini_string, 0, 0, 1)
    ZEND_ARG_INFO(0, ini_string)
    ZEND_ARG_INFO(0, process_sections)
    ZEND_ARG_INFO(0, scanner_mode)
ZEND_END_ARG_INFO()

#if ZEND_DEBUG
ZEND_BEGIN_ARG_INFO(arginfo_config_get_hash, 0)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GETLOADAVG
ZEND_BEGIN_ARG_INFO(arginfo_sys_getloadavg, 0)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ assert.c */
ZEND_BEGIN_ARG_INFO(arginfo_assert, 0)
	ZEND_ARG_INFO(0, assertion)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_assert_options, 0, 0, 1)
	ZEND_ARG_INFO(0, what)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ base64.c */
ZEND_BEGIN_ARG_INFO(arginfo_base64_encode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_base64_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, strict)
ZEND_END_ARG_INFO()

/* }}} */
/* {{{ browscap.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_browser, 0, 0, 0)
	ZEND_ARG_INFO(0, browser_name)
	ZEND_ARG_INFO(0, return_array)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ crc32.c */
ZEND_BEGIN_ARG_INFO(arginfo_crc32, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

/* }}} */
/* {{{ crypt.c */
#if HAVE_CRYPT
ZEND_BEGIN_ARG_INFO_EX(arginfo_crypt, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, salt)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ cyr_convert.c */
ZEND_BEGIN_ARG_INFO(arginfo_convert_cyr_string, 0)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(0, to)
ZEND_END_ARG_INFO()

/* }}} */
/* {{{ datetime.c */
#if HAVE_STRPTIME
ZEND_BEGIN_ARG_INFO(arginfo_strptime, 0)
	ZEND_ARG_INFO(0, timestamp)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ dir.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_opendir, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dir, 0, 0, 1)
	ZEND_ARG_INFO(0, directory)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_closedir, 0, 0, 0)
	ZEND_ARG_INFO(0, dir_handle)
ZEND_END_ARG_INFO()

#if defined(HAVE_CHROOT) && !defined(ZTS) && ENABLE_CHROOT_FUNC
ZEND_BEGIN_ARG_INFO(arginfo_chroot, 0)
	ZEND_ARG_INFO(0, directory)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_chdir, 0)
	ZEND_ARG_INFO(0, directory)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getcwd, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rewinddir, 0, 0, 0)
	ZEND_ARG_INFO(0, dir_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_readdir, 0, 0, 0)
	ZEND_ARG_INFO(0, dir_handle)
ZEND_END_ARG_INFO()

#ifdef HAVE_GLOB
ZEND_BEGIN_ARG_INFO_EX(arginfo_glob, 0, 0, 1)
	ZEND_ARG_INFO(0, pattern)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_scandir, 0, 0, 1)
	ZEND_ARG_INFO(0, dir)
	ZEND_ARG_INFO(0, sorting_order)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ arginfo ext/standard/dl.c */
ZEND_BEGIN_ARG_INFO(arginfo_dl, 0)
	ZEND_ARG_INFO(0, extension_filename)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ dns.c */
ZEND_BEGIN_ARG_INFO(arginfo_gethostbyaddr, 0)
	ZEND_ARG_INFO(0, ip_address)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_gethostbyname, 0)
	ZEND_ARG_INFO(0, hostname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_gethostbynamel, 0)
	ZEND_ARG_INFO(0, hostname)
ZEND_END_ARG_INFO()

#ifdef HAVE_GETHOSTNAME
ZEND_BEGIN_ARG_INFO(arginfo_gethostname, 0)
ZEND_END_ARG_INFO()
#endif

#if defined(PHP_WIN32) || (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE)))
ZEND_BEGIN_ARG_INFO_EX(arginfo_dns_check_record, 0, 0, 1)
	ZEND_ARG_INFO(0, host)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

# if defined(PHP_WIN32) || HAVE_FULL_DNS_FUNCS
ZEND_BEGIN_ARG_INFO_EX(arginfo_dns_get_record, 0, 0, 1)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_ARRAY_INFO(1, authns, 1)
	ZEND_ARG_ARRAY_INFO(1, addtl, 1)
	ZEND_ARG_INFO(0, raw)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dns_get_mx, 0, 0, 2)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(1, mxhosts) /* ARRAY_INFO(1, mxhosts, 1) */
	ZEND_ARG_INFO(1, weight) /* ARRAY_INFO(1, weight, 1) */
ZEND_END_ARG_INFO()
# endif

#endif /* defined(PHP_WIN32) || (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE))) */
/* }}} */

/* {{{ exec.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_exec, 0, 0, 1)
	ZEND_ARG_INFO(0, command)
	ZEND_ARG_INFO(1, output) /* ARRAY_INFO(1, output, 1) */
	ZEND_ARG_INFO(1, return_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_system, 0, 0, 1)
	ZEND_ARG_INFO(0, command)
	ZEND_ARG_INFO(1, return_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_passthru, 0, 0, 1)
	ZEND_ARG_INFO(0, command)
	ZEND_ARG_INFO(1, return_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_escapeshellcmd, 0)
	ZEND_ARG_INFO(0, command)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_escapeshellarg, 0)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_shell_exec, 0)
	ZEND_ARG_INFO(0, cmd)
ZEND_END_ARG_INFO()

#ifdef HAVE_NICE
ZEND_BEGIN_ARG_INFO(arginfo_proc_nice, 0)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ file.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_flock, 0, 0, 2)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, operation)
	ZEND_ARG_INFO(1, wouldblock)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_meta_tags, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, use_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_file_get_contents, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, context)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, maxlen)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_file_put_contents, 0, 0, 2)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_file, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tempnam, 0)
	ZEND_ARG_INFO(0, dir)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tmpfile, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fopen, 0, 0, 2)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, use_include_path)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fclose, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_popen, 0)
	ZEND_ARG_INFO(0, command)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pclose, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_feof, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fgets, 0, 0, 1)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fgetc, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fgetss, 0, 0, 1)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, allowable_tags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fscanf, 1, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(1, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fwrite, 0, 0, 2)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fflush, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rewind, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ftell, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fseek, 0, 0, 2)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, whence)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mkdir, 0, 0, 1)
	ZEND_ARG_INFO(0, pathname)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, recursive)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rmdir, 0, 0, 1)
	ZEND_ARG_INFO(0, dirname)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_readfile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_umask, 0, 0, 0)
	ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fpassthru, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rename, 0, 0, 2)
	ZEND_ARG_INFO(0, old_name)
	ZEND_ARG_INFO(0, new_name)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unlink, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ftruncate, 0)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fstat, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_copy, 0)
	ZEND_ARG_INFO(0, source_file)
	ZEND_ARG_INFO(0, destination_file)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fread, 0)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fputcsv, 0, 0, 2)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, fields) /* ARRAY_INFO(0, fields, 1) */
	ZEND_ARG_INFO(0, delimiter)
	ZEND_ARG_INFO(0, enclosure)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fgetcsv, 0, 0, 1)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, delimiter)
	ZEND_ARG_INFO(0, enclosure)
	ZEND_ARG_INFO(0, escape)
ZEND_END_ARG_INFO()

#if (!defined(__BEOS__) && !defined(NETWARE) && HAVE_REALPATH) || defined(ZTS)
ZEND_BEGIN_ARG_INFO(arginfo_realpath, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_FNMATCH
ZEND_BEGIN_ARG_INFO_EX(arginfo_fnmatch, 0, 0, 2)
	ZEND_ARG_INFO(0, pattern)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_sys_get_temp_dir, 0)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ filestat.c */
ZEND_BEGIN_ARG_INFO(arginfo_disk_total_space, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_disk_free_space, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

#ifndef NETWARE
ZEND_BEGIN_ARG_INFO(arginfo_chgrp, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_chown, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()
#endif

#if HAVE_LCHOWN
ZEND_BEGIN_ARG_INFO(arginfo_lchgrp, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_lchown, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_chmod, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

#if HAVE_UTIME
ZEND_BEGIN_ARG_INFO_EX(arginfo_touch, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, time)
	ZEND_ARG_INFO(0, atime)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_clearstatcache, 0, 0, 0)
	ZEND_ARG_INFO(0, clear_realpath_cache)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_realpath_cache_size, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_realpath_cache_get, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fileperms, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fileinode, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_filesize, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fileowner, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_filegroup, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fileatime, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_filemtime, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_filectime, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_filetype, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_writable, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_readable, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_executable, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_file, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_dir, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_link, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_file_exists, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_lstat, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stat, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ formatted_print.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_sprintf, 0, 0, 2)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vsprintf, 0)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, args) /* ARRAY_INFO(0, args, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_printf, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vprintf, 0)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, args) /* ARRAY_INFO(0, args, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fprintf, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vfprintf, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, args) /* ARRAY_INFO(0, args, 1) */
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ fsock.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_fsockopen, 0, 0, 2)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(1, errno)
	ZEND_ARG_INFO(1, errstr)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_pfsockopen, 0, 0, 2)
	ZEND_ARG_INFO(0, hostname)
	ZEND_ARG_INFO(0, port)
	ZEND_ARG_INFO(1, errno)
	ZEND_ARG_INFO(1, errstr)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ ftok.c */
#if HAVE_FTOK
ZEND_BEGIN_ARG_INFO(arginfo_ftok, 0)
	ZEND_ARG_INFO(0, pathname)
	ZEND_ARG_INFO(0, proj)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ head.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_header, 0, 0, 1)
	ZEND_ARG_INFO(0, header)
	ZEND_ARG_INFO(0, replace)
	ZEND_ARG_INFO(0, http_response_code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_header_remove, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_setcookie, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, expires)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, domain)
	ZEND_ARG_INFO(0, secure)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_setrawcookie, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, expires)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, domain)
	ZEND_ARG_INFO(0, secure)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_headers_sent, 0, 0, 0)
	ZEND_ARG_INFO(1, file)
	ZEND_ARG_INFO(1, line)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_headers_list, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_http_response_code, 0, 0, 0)
	ZEND_ARG_INFO(0, response_code)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ html.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_htmlspecialchars, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, quote_style)
	ZEND_ARG_INFO(0, charset)
	ZEND_ARG_INFO(0, double_encode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_htmlspecialchars_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, quote_style)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_html_entity_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, quote_style)
	ZEND_ARG_INFO(0, charset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_htmlentities, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, quote_style)
	ZEND_ARG_INFO(0, charset)
	ZEND_ARG_INFO(0, double_encode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_html_translation_table, 0, 0, 0)
	ZEND_ARG_INFO(0, table)
	ZEND_ARG_INFO(0, quote_style)
ZEND_END_ARG_INFO()

/* }}} */
/* {{{ http.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_http_build_query, 0, 0, 1)
	ZEND_ARG_INFO(0, formdata)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, arg_separator)
	ZEND_ARG_INFO(0, enc_type)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ image.c */
ZEND_BEGIN_ARG_INFO(arginfo_image_type_to_mime_type, 0)
	ZEND_ARG_INFO(0, imagetype)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_image_type_to_extension, 0, 0, 1)
	ZEND_ARG_INFO(0, imagetype)
	ZEND_ARG_INFO(0, include_dot)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_getimagesize, 0, 0, 1)
	ZEND_ARG_INFO(0, imagefile)
	ZEND_ARG_INFO(1, info) /* ARRAY_INFO(1, info, 1) */
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ info.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_phpinfo, 0, 0, 0)
	ZEND_ARG_INFO(0, what)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpversion, 0, 0, 0)
	ZEND_ARG_INFO(0, extension)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpcredits, 0, 0, 0)
	ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_php_sapi_name, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_php_uname, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_php_ini_scanned_files, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_php_ini_loaded_file, 0)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ iptc.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_iptcembed, 0, 0, 2)
	ZEND_ARG_INFO(0, iptcdata)
	ZEND_ARG_INFO(0, jpeg_file_name)
	ZEND_ARG_INFO(0, spool)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_iptcparse, 0)
	ZEND_ARG_INFO(0, iptcdata)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ lcg.c */
ZEND_BEGIN_ARG_INFO(arginfo_lcg_value, 0)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ levenshtein.c */
ZEND_BEGIN_ARG_INFO(arginfo_levenshtein, 0)
	ZEND_ARG_INFO(0, str1)
	ZEND_ARG_INFO(0, str2)
	ZEND_ARG_INFO(0, cost_ins)
	ZEND_ARG_INFO(0, cost_rep)
	ZEND_ARG_INFO(0, cost_del)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ link.c */
#if defined(HAVE_SYMLINK) || defined(PHP_WIN32)
ZEND_BEGIN_ARG_INFO(arginfo_readlink, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_linkinfo, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_symlink, 0)
	ZEND_ARG_INFO(0, target)
	ZEND_ARG_INFO(0, link)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_link, 0)
	ZEND_ARG_INFO(0, target)
	ZEND_ARG_INFO(0, link)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ mail.c */
ZEND_BEGIN_ARG_INFO(arginfo_ezmlm_hash, 0)
	ZEND_ARG_INFO(0, addr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mail, 0, 0, 3)
	ZEND_ARG_INFO(0, to)
	ZEND_ARG_INFO(0, subject)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, additional_headers)
	ZEND_ARG_INFO(0, additional_parameters)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ math.c */
ZEND_BEGIN_ARG_INFO(arginfo_abs, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ceil, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_floor, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_round, 0, 0, 1)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, precision)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sin, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_cos, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tan, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_asin, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_acos, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_atan, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_atan2, 0)
	ZEND_ARG_INFO(0, y)
	ZEND_ARG_INFO(0, x)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sinh, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_cosh, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tanh, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_asinh, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_acosh, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_atanh, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pi, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_finite, 0)
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_infinite, 0)
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_nan, 0)
	ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pow, 0)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, exponent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_exp, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_expm1, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_log1p, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_log, 0, 0, 1)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_log10, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sqrt, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hypot, 0)
	ZEND_ARG_INFO(0, num1)
	ZEND_ARG_INFO(0, num2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_deg2rad, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rad2deg, 0)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_bindec, 0)
	ZEND_ARG_INFO(0, binary_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hexdec, 0)
	ZEND_ARG_INFO(0, hexadecimal_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_octdec, 0)
	ZEND_ARG_INFO(0, octal_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_decbin, 0)
	ZEND_ARG_INFO(0, decimal_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_decoct, 0)
	ZEND_ARG_INFO(0, decimal_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dechex, 0)
	ZEND_ARG_INFO(0, decimal_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_base_convert, 0)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, frombase)
	ZEND_ARG_INFO(0, tobase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_number_format, 0, 0, 1)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, num_decimal_places)
	ZEND_ARG_INFO(0, dec_seperator)
	ZEND_ARG_INFO(0, thousands_seperator)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fmod, 0)
	ZEND_ARG_INFO(0, x)
	ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ md5.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_md5, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, raw_output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_md5_file, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, raw_output)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ metaphone.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_metaphone, 0, 0, 1)
	ZEND_ARG_INFO(0, text)
	ZEND_ARG_INFO(0, phones)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ microtime.c */
#ifdef HAVE_GETTIMEOFDAY
ZEND_BEGIN_ARG_INFO_EX(arginfo_microtime, 0, 0, 0)
	ZEND_ARG_INFO(0, get_as_float)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gettimeofday, 0, 0, 0)
	ZEND_ARG_INFO(0, get_as_float)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_GETRUSAGE
ZEND_BEGIN_ARG_INFO_EX(arginfo_getrusage, 0, 0, 0)
	ZEND_ARG_INFO(0, who)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ pack.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_pack, 0, 0, 2)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_unpack, 0)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, input)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ pageinfo.c */
ZEND_BEGIN_ARG_INFO(arginfo_getmyuid, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getmygid, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getmypid, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getmyinode, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getlastmod, 0)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ password.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_password_hash, 0, 0, 2)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, algo)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_password_get_info, 0, 0, 1)
	ZEND_ARG_INFO(0, hash)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_password_needs_rehash, 0, 0, 2)
	ZEND_ARG_INFO(0, hash)
	ZEND_ARG_INFO(0, algo)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_password_verify, 0, 0, 2)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, hash)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ proc_open.c */
#ifdef PHP_CAN_SUPPORT_PROC_OPEN
ZEND_BEGIN_ARG_INFO_EX(arginfo_proc_terminate, 0, 0, 1)
	ZEND_ARG_INFO(0, process)
	ZEND_ARG_INFO(0, signal)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proc_close, 0)
	ZEND_ARG_INFO(0, process)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proc_get_status, 0)
	ZEND_ARG_INFO(0, process)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_proc_open, 0, 0, 3)
	ZEND_ARG_INFO(0, command)
	ZEND_ARG_INFO(0, descriptorspec) /* ARRAY_INFO(0, descriptorspec, 1) */
	ZEND_ARG_INFO(1, pipes) /* ARRAY_INFO(1, pipes, 1) */
	ZEND_ARG_INFO(0, cwd)
	ZEND_ARG_INFO(0, env) /* ARRAY_INFO(0, env, 1) */
	ZEND_ARG_INFO(0, other_options) /* ARRAY_INFO(0, other_options, 1) */
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ quot_print.c */
ZEND_BEGIN_ARG_INFO(arginfo_quoted_printable_decode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ quot_print.c */
ZEND_BEGIN_ARG_INFO(arginfo_quoted_printable_encode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ rand.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_srand, 0, 0, 0)
	ZEND_ARG_INFO(0, seed)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mt_srand, 0, 0, 0)
	ZEND_ARG_INFO(0, seed)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rand, 0, 0, 0)
	ZEND_ARG_INFO(0, min)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mt_rand, 0, 0, 0)
	ZEND_ARG_INFO(0, min)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getrandmax, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_mt_getrandmax, 0)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ sha1.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_sha1, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, raw_output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sha1_file, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, raw_output)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ soundex.c */
ZEND_BEGIN_ARG_INFO(arginfo_soundex, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ streamsfuncs.c */
#if HAVE_SOCKETPAIR
ZEND_BEGIN_ARG_INFO(arginfo_stream_socket_pair, 0)
	ZEND_ARG_INFO(0, domain)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_socket_client, 0, 0, 1)
	ZEND_ARG_INFO(0, remoteaddress)
	ZEND_ARG_INFO(1, errcode)
	ZEND_ARG_INFO(1, errstring)
	ZEND_ARG_INFO(0, timeout)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_socket_server, 0, 0, 1)
	ZEND_ARG_INFO(0, localaddress)
	ZEND_ARG_INFO(1, errcode)
	ZEND_ARG_INFO(1, errstring)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_socket_accept, 0, 0, 1)
	ZEND_ARG_INFO(0, serverstream)
	ZEND_ARG_INFO(0, timeout)
	ZEND_ARG_INFO(1, peername)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_socket_get_name, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, want_peer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_socket_sendto, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, target_addr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_socket_recvfrom, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, amount)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(1, remote_addr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_get_contents, 0, 0, 1)
	ZEND_ARG_INFO(0, source)
	ZEND_ARG_INFO(0, maxlen)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_copy_to_stream, 0, 0, 2)
	ZEND_ARG_INFO(0, source)
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, maxlen)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_get_meta_data, 0)
	ZEND_ARG_INFO(0, fp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_get_transports, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_get_wrappers, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_resolve_include_path, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_is_local, 0)
	ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_supports_lock, 0, 0, 1)
    ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_select, 0, 0, 4)
	ZEND_ARG_INFO(1, read_streams) /* ARRAY_INFO(1, read_streams, 1) */
	ZEND_ARG_INFO(1, write_streams) /* ARRAY_INFO(1, write_streams, 1) */
	ZEND_ARG_INFO(1, except_streams) /* ARRAY_INFO(1, except_streams, 1) */
	ZEND_ARG_INFO(0, tv_sec)
	ZEND_ARG_INFO(0, tv_usec)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_context_get_options, 0)
	ZEND_ARG_INFO(0, stream_or_context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_context_set_option, 0)
	ZEND_ARG_INFO(0, stream_or_context)
	ZEND_ARG_INFO(0, wrappername)
	ZEND_ARG_INFO(0, optionname)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_context_set_params, 0)
	ZEND_ARG_INFO(0, stream_or_context)
	ZEND_ARG_INFO(0, options) /* ARRAY_INFO(0, options, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_context_get_params, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, stream_or_context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_context_get_default, 0, 0, 0)
	ZEND_ARG_INFO(0, options) /* ARRAY_INFO(0, options, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_context_set_default, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_context_create, 0, 0, 0)
	ZEND_ARG_INFO(0, options) /* ARRAY_INFO(0, options, 1) */
	ZEND_ARG_INFO(0, params) /* ARRAY_INFO(0, params, 1) */
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_filter_prepend, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, filtername)
	ZEND_ARG_INFO(0, read_write)
	ZEND_ARG_INFO(0, filterparams)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_filter_append, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, filtername)
	ZEND_ARG_INFO(0, read_write)
	ZEND_ARG_INFO(0, filterparams)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_filter_remove, 0)
	ZEND_ARG_INFO(0, stream_filter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_get_line, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, maxlen)
	ZEND_ARG_INFO(0, ending)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_set_blocking, 0)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
ZEND_BEGIN_ARG_INFO(arginfo_stream_set_timeout, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, seconds)
	ZEND_ARG_INFO(0, microseconds)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_stream_set_read_buffer, 0)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_set_write_buffer, 0)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()
		
ZEND_BEGIN_ARG_INFO(arginfo_stream_set_chunk_size, 0)
	ZEND_ARG_INFO(0, fp)
	ZEND_ARG_INFO(0, chunk_size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_socket_enable_crypto, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, enable)
	ZEND_ARG_INFO(0, cryptokind)
	ZEND_ARG_INFO(0, sessionstream)
ZEND_END_ARG_INFO()

#ifdef HAVE_SHUTDOWN
ZEND_BEGIN_ARG_INFO(arginfo_stream_socket_shutdown, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, how)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ string.c */
ZEND_BEGIN_ARG_INFO(arginfo_bin2hex, 0)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hex2bin, 0)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strspn, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, mask)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strcspn, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, mask)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

#if HAVE_NL_LANGINFO
ZEND_BEGIN_ARG_INFO(arginfo_nl_langinfo, 0)
	ZEND_ARG_INFO(0, item)
ZEND_END_ARG_INFO()
#endif

#ifdef HAVE_STRCOLL
ZEND_BEGIN_ARG_INFO(arginfo_strcoll, 0)
	ZEND_ARG_INFO(0, str1)
	ZEND_ARG_INFO(0, str2)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_trim, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, character_mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_rtrim, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, character_mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ltrim, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, character_mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wordwrap, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, break)
	ZEND_ARG_INFO(0, cut)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_explode, 0, 0, 2)
	ZEND_ARG_INFO(0, separator)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_implode, 0)
	ZEND_ARG_INFO(0, glue)
	ZEND_ARG_INFO(0, pieces)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strtok, 0)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, token)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strtoupper, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strtolower, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_basename, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, suffix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dirname, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_pathinfo, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stristr, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, part)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strstr, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, part)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strpos, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stripos, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strrpos, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strripos, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strrchr, 0)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chunk_split, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, chunklen)
	ZEND_ARG_INFO(0, ending)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_substr, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_substr_replace, 0, 0, 3)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, replace)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quotemeta, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ord, 0)
	ZEND_ARG_INFO(0, character)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_chr, 0)
	ZEND_ARG_INFO(0, codepoint)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ucfirst, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_lcfirst, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
	
ZEND_BEGIN_ARG_INFO(arginfo_ucwords, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strtr, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(0, to)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strrev, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_similar_text, 0, 0, 2)
	ZEND_ARG_INFO(0, str1)
	ZEND_ARG_INFO(0, str2)
	ZEND_ARG_INFO(1, percent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_addcslashes, 0)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, charlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_addslashes, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stripcslashes, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stripslashes, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_str_replace, 0, 0, 3)
	ZEND_ARG_INFO(0, search)
	ZEND_ARG_INFO(0, replace)
	ZEND_ARG_INFO(0, subject)
	ZEND_ARG_INFO(1, replace_count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_str_ireplace, 0, 0, 3)
	ZEND_ARG_INFO(0, search)
	ZEND_ARG_INFO(0, replace)
	ZEND_ARG_INFO(0, subject)
	ZEND_ARG_INFO(1, replace_count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hebrev, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, max_chars_per_line)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hebrevc, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, max_chars_per_line)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nl2br, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, is_xhtml)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strip_tags, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, allowable_tags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_setlocale, 0, 0, 2)
	ZEND_ARG_INFO(0, category)
	ZEND_ARG_INFO(0, locale)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_parse_str, 0, 0, 1)
	ZEND_ARG_INFO(0, encoded_string)
	ZEND_ARG_INFO(1, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_str_getcsv, 0, 0, 1)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, delimiter)
	ZEND_ARG_INFO(0, enclosure)
	ZEND_ARG_INFO(0, escape)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_str_repeat, 0)
	ZEND_ARG_INFO(0, input)
	ZEND_ARG_INFO(0, mult)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_count_chars, 0, 0, 1)
	ZEND_ARG_INFO(0, input)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strnatcmp, 0)
	ZEND_ARG_INFO(0, s1)
	ZEND_ARG_INFO(0, s2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_localeconv, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strnatcasecmp, 0)
	ZEND_ARG_INFO(0, s1)
	ZEND_ARG_INFO(0, s2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_substr_count, 0, 0, 2)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, needle)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_str_pad, 0, 0, 2)
	ZEND_ARG_INFO(0, input)
	ZEND_ARG_INFO(0, pad_length)
	ZEND_ARG_INFO(0, pad_string)
	ZEND_ARG_INFO(0, pad_type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sscanf, 1, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(1, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_str_rot13, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_str_shuffle, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_str_word_count, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, charlist)
ZEND_END_ARG_INFO()

#ifdef HAVE_STRFMON
ZEND_BEGIN_ARG_INFO(arginfo_money_format, 0)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_str_split, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, split_length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strpbrk, 0, 0, 1)
	ZEND_ARG_INFO(0, haystack)
	ZEND_ARG_INFO(0, char_list)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_substr_compare, 0, 0, 3)
	ZEND_ARG_INFO(0, main_str)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, case_sensitivity)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ syslog.c */
#ifdef HAVE_SYSLOG_H
ZEND_BEGIN_ARG_INFO(arginfo_openlog, 0)
	ZEND_ARG_INFO(0, ident)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, facility)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_closelog, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_syslog, 0)
	ZEND_ARG_INFO(0, priority)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ type.c */
ZEND_BEGIN_ARG_INFO(arginfo_gettype, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_settype, 0)
	ZEND_ARG_INFO(1, var)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_intval, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_floatval, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_strval, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_boolval, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_null, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_resource, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_bool, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_long, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_float, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_string, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_array, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_object, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_numeric, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_is_scalar, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_is_callable, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, syntax_only)
	ZEND_ARG_INFO(1, callable_name)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ uniqid.c */
#ifdef HAVE_GETTIMEOFDAY
ZEND_BEGIN_ARG_INFO_EX(arginfo_uniqid, 0, 0, 0)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, more_entropy)
ZEND_END_ARG_INFO()
#endif
/* }}} */
/* {{{ url.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_parse_url, 0, 0, 1)
	ZEND_ARG_INFO(0, url)
	ZEND_ARG_INFO(0, component)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_urlencode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_urldecode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rawurlencode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_rawurldecode, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_headers, 0, 0, 1)
	ZEND_ARG_INFO(0, url)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ user_filters.c */
ZEND_BEGIN_ARG_INFO(arginfo_stream_bucket_make_writeable, 0)
	ZEND_ARG_INFO(0, brigade)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_bucket_prepend, 0)
	ZEND_ARG_INFO(0, brigade)
	ZEND_ARG_INFO(0, bucket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_bucket_append, 0)
	ZEND_ARG_INFO(0, brigade)
	ZEND_ARG_INFO(0, bucket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_bucket_new, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_get_filters, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_stream_filter_register, 0)
	ZEND_ARG_INFO(0, filtername)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ uuencode.c */
ZEND_BEGIN_ARG_INFO(arginfo_convert_uuencode, 0)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_convert_uudecode, 0)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ var.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_var_dump, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_debug_zval_dump, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_var_export, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_serialize, 0)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_unserialize, 0)
	ZEND_ARG_INFO(0, variable_representation)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_memory_get_usage, 0, 0, 0)
	ZEND_ARG_INFO(0, real_usage)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_memory_get_peak_usage, 0, 0, 0)
	ZEND_ARG_INFO(0, real_usage)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ versioning.c */
ZEND_BEGIN_ARG_INFO_EX(arginfo_version_compare, 0, 0, 2)
	ZEND_ARG_INFO(0, ver1)
	ZEND_ARG_INFO(0, ver2)
	ZEND_ARG_INFO(0, oper)
ZEND_END_ARG_INFO()
/* }}} */
/* }}} */

const zend_function_entry basic_functions[] = { /* {{{ */
	PHP_FE(constant,														arginfo_constant)
	PHP_FE(bin2hex,															arginfo_bin2hex)
	PHP_FE(hex2bin,															arginfo_hex2bin)
	PHP_FE(sleep,															arginfo_sleep)
	PHP_FE(usleep,															arginfo_usleep)
#if HAVE_NANOSLEEP
	PHP_FE(time_nanosleep,													arginfo_time_nanosleep)
	PHP_FE(time_sleep_until,												arginfo_time_sleep_until)
#endif

#if HAVE_STRPTIME
	PHP_FE(strptime,														arginfo_strptime)
#endif

	PHP_FE(flush,															arginfo_flush)
	PHP_FE(wordwrap,														arginfo_wordwrap)
	PHP_FE(htmlspecialchars,												arginfo_htmlspecialchars)
	PHP_FE(htmlentities,													arginfo_htmlentities)
	PHP_FE(html_entity_decode,												arginfo_html_entity_decode)
	PHP_FE(htmlspecialchars_decode,											arginfo_htmlspecialchars_decode)
	PHP_FE(get_html_translation_table,										arginfo_get_html_translation_table)
	PHP_FE(sha1,															arginfo_sha1)
	PHP_FE(sha1_file,														arginfo_sha1_file)
	PHP_NAMED_FE(md5,php_if_md5,											arginfo_md5)
	PHP_NAMED_FE(md5_file,php_if_md5_file,									arginfo_md5_file)
	PHP_NAMED_FE(crc32,php_if_crc32,										arginfo_crc32)

	PHP_FE(iptcparse,														arginfo_iptcparse)
	PHP_FE(iptcembed,														arginfo_iptcembed)
	PHP_FE(getimagesize,													arginfo_getimagesize)
	PHP_FE(getimagesizefromstring,											arginfo_getimagesize)
	PHP_FE(image_type_to_mime_type,											arginfo_image_type_to_mime_type)
	PHP_FE(image_type_to_extension,											arginfo_image_type_to_extension)

	PHP_FE(phpinfo,															arginfo_phpinfo)
	PHP_FE(phpversion,														arginfo_phpversion)
	PHP_FE(phpcredits,														arginfo_phpcredits)
	PHP_FE(php_sapi_name,													arginfo_php_sapi_name)
	PHP_FE(php_uname,														arginfo_php_uname)
	PHP_FE(php_ini_scanned_files,											arginfo_php_ini_scanned_files)
	PHP_FE(php_ini_loaded_file,												arginfo_php_ini_loaded_file)

	PHP_FE(strnatcmp,														arginfo_strnatcmp)
	PHP_FE(strnatcasecmp,													arginfo_strnatcasecmp)
	PHP_FE(substr_count,													arginfo_substr_count)
	PHP_FE(strspn,															arginfo_strspn)
	PHP_FE(strcspn,															arginfo_strcspn)
	PHP_FE(strtok,															arginfo_strtok)
	PHP_FE(strtoupper,														arginfo_strtoupper)
	PHP_FE(strtolower,														arginfo_strtolower)
	PHP_FE(strpos,															arginfo_strpos)
	PHP_FE(stripos,															arginfo_stripos)
	PHP_FE(strrpos,															arginfo_strrpos)
	PHP_FE(strripos,														arginfo_strripos)
	PHP_FE(strrev,															arginfo_strrev)
	PHP_FE(hebrev,															arginfo_hebrev)
	PHP_FE(hebrevc,															arginfo_hebrevc)
	PHP_FE(nl2br,															arginfo_nl2br)
	PHP_FE(basename,														arginfo_basename)
	PHP_FE(dirname,															arginfo_dirname)
	PHP_FE(pathinfo,														arginfo_pathinfo)
	PHP_FE(stripslashes,													arginfo_stripslashes)
	PHP_FE(stripcslashes,													arginfo_stripcslashes)
	PHP_FE(strstr,															arginfo_strstr)
	PHP_FE(stristr,															arginfo_stristr)
	PHP_FE(strrchr,															arginfo_strrchr)
	PHP_FE(str_shuffle,														arginfo_str_shuffle)
	PHP_FE(str_word_count,													arginfo_str_word_count)
	PHP_FE(str_split,														arginfo_str_split)
	PHP_FE(strpbrk,															arginfo_strpbrk)
	PHP_FE(substr_compare,													arginfo_substr_compare)

#ifdef HAVE_STRCOLL
	PHP_FE(strcoll,															arginfo_strcoll)
#endif

#ifdef HAVE_STRFMON
	PHP_FE(money_format,													arginfo_money_format)
#endif

	PHP_FE(substr,															arginfo_substr)
	PHP_FE(substr_replace,													arginfo_substr_replace)
	PHP_FE(quotemeta,														arginfo_quotemeta)
	PHP_FE(ucfirst,															arginfo_ucfirst)
	PHP_FE(lcfirst,															arginfo_lcfirst)
	PHP_FE(ucwords,															arginfo_ucwords)
	PHP_FE(strtr,															arginfo_strtr)
	PHP_FE(addslashes,														arginfo_addslashes)
	PHP_FE(addcslashes,														arginfo_addcslashes)
	PHP_FE(rtrim,															arginfo_rtrim)
	PHP_FE(str_replace,														arginfo_str_replace)
	PHP_FE(str_ireplace,													arginfo_str_ireplace)
	PHP_FE(str_repeat,														arginfo_str_repeat)
	PHP_FE(count_chars,														arginfo_count_chars)
	PHP_FE(chunk_split,														arginfo_chunk_split)
	PHP_FE(trim,															arginfo_trim)
	PHP_FE(ltrim,															arginfo_ltrim)
	PHP_FE(strip_tags,														arginfo_strip_tags)
	PHP_FE(similar_text,													arginfo_similar_text)
	PHP_FE(explode,															arginfo_explode)
	PHP_FE(implode,															arginfo_implode)
	PHP_FALIAS(join,				implode,								arginfo_implode)
	PHP_FE(setlocale,														arginfo_setlocale)
	PHP_FE(localeconv,														arginfo_localeconv)

#if HAVE_NL_LANGINFO
	PHP_FE(nl_langinfo,														arginfo_nl_langinfo)
#endif

	PHP_FE(soundex,															arginfo_soundex)
	PHP_FE(levenshtein,														arginfo_levenshtein)
	PHP_FE(chr,																arginfo_chr)
	PHP_FE(ord,																arginfo_ord)
	PHP_FE(parse_str,														arginfo_parse_str)
	PHP_FE(str_getcsv,														arginfo_str_getcsv)
	PHP_FE(str_pad,															arginfo_str_pad)
	PHP_FALIAS(chop,				rtrim,									arginfo_rtrim)
	PHP_FALIAS(strchr,				strstr,									arginfo_strstr)
	PHP_NAMED_FE(sprintf,			PHP_FN(user_sprintf),					arginfo_sprintf)
	PHP_NAMED_FE(printf,			PHP_FN(user_printf),					arginfo_printf)
	PHP_FE(vprintf,															arginfo_vprintf)
	PHP_FE(vsprintf,														arginfo_vsprintf)
	PHP_FE(fprintf,															arginfo_fprintf)
	PHP_FE(vfprintf,														arginfo_vfprintf)
	PHP_FE(sscanf,															arginfo_sscanf)
	PHP_FE(fscanf,															arginfo_fscanf)
	PHP_FE(parse_url,														arginfo_parse_url)
	PHP_FE(urlencode,														arginfo_urlencode)
	PHP_FE(urldecode,														arginfo_urldecode)
	PHP_FE(rawurlencode,													arginfo_rawurlencode)
	PHP_FE(rawurldecode,													arginfo_rawurldecode)
	PHP_FE(http_build_query,												arginfo_http_build_query)

#if defined(HAVE_SYMLINK) || defined(PHP_WIN32)
	PHP_FE(readlink,														arginfo_readlink)
	PHP_FE(linkinfo,														arginfo_linkinfo)
	PHP_FE(symlink,															arginfo_symlink)
	PHP_FE(link,															arginfo_link)
#endif

	PHP_FE(unlink,															arginfo_unlink)
	PHP_FE(exec,															arginfo_exec)
	PHP_FE(system,															arginfo_system)
	PHP_FE(escapeshellcmd,													arginfo_escapeshellcmd)
	PHP_FE(escapeshellarg,													arginfo_escapeshellarg)
	PHP_FE(passthru,														arginfo_passthru)
	PHP_FE(shell_exec,														arginfo_shell_exec)
#ifdef PHP_CAN_SUPPORT_PROC_OPEN
	PHP_FE(proc_open,														arginfo_proc_open)
	PHP_FE(proc_close,														arginfo_proc_close)
	PHP_FE(proc_terminate,													arginfo_proc_terminate)
	PHP_FE(proc_get_status,													arginfo_proc_get_status)
#endif

#ifdef HAVE_NICE
	PHP_FE(proc_nice,														arginfo_proc_nice)
#endif

	PHP_FE(rand,															arginfo_rand)
	PHP_FE(srand,															arginfo_srand)
	PHP_FE(getrandmax,													arginfo_getrandmax)
	PHP_FE(mt_rand,														arginfo_mt_rand)
	PHP_FE(mt_srand,														arginfo_mt_srand)
	PHP_FE(mt_getrandmax,													arginfo_mt_getrandmax)

#if HAVE_GETSERVBYNAME
	PHP_FE(getservbyname,													arginfo_getservbyname)
#endif

#if HAVE_GETSERVBYPORT
	PHP_FE(getservbyport,													arginfo_getservbyport)
#endif

#if HAVE_GETPROTOBYNAME
	PHP_FE(getprotobyname,													arginfo_getprotobyname)
#endif

#if HAVE_GETPROTOBYNUMBER
	PHP_FE(getprotobynumber,												arginfo_getprotobynumber)
#endif

	PHP_FE(getmyuid,														arginfo_getmyuid)
	PHP_FE(getmygid,														arginfo_getmygid)
	PHP_FE(getmypid,														arginfo_getmypid)
	PHP_FE(getmyinode,														arginfo_getmyinode)
	PHP_FE(getlastmod,														arginfo_getlastmod)

	PHP_FE(base64_decode,													arginfo_base64_decode)
	PHP_FE(base64_encode,													arginfo_base64_encode)

	PHP_FE(password_hash,													arginfo_password_hash)
	PHP_FE(password_get_info,												arginfo_password_get_info)
	PHP_FE(password_needs_rehash,											arginfo_password_needs_rehash)
	PHP_FE(password_verify,													arginfo_password_verify)
	PHP_FE(convert_uuencode,												arginfo_convert_uuencode)
	PHP_FE(convert_uudecode,												arginfo_convert_uudecode)

	PHP_FE(abs,																arginfo_abs)
	PHP_FE(ceil,															arginfo_ceil)
	PHP_FE(floor,															arginfo_floor)
	PHP_FE(round,															arginfo_round)
	PHP_FE(sin,																arginfo_sin)
	PHP_FE(cos,																arginfo_cos)
	PHP_FE(tan,																arginfo_tan)
	PHP_FE(asin,															arginfo_asin)
	PHP_FE(acos,															arginfo_acos)
	PHP_FE(atan,															arginfo_atan)
	PHP_FE(atanh,															arginfo_atanh)
	PHP_FE(atan2,															arginfo_atan2)
	PHP_FE(sinh,															arginfo_sinh)
	PHP_FE(cosh,															arginfo_cosh)
	PHP_FE(tanh,															arginfo_tanh)
	PHP_FE(asinh,															arginfo_asinh)
	PHP_FE(acosh,															arginfo_acosh)
	PHP_FE(expm1,															arginfo_expm1)
	PHP_FE(log1p,															arginfo_log1p)
	PHP_FE(pi,																arginfo_pi)
	PHP_FE(is_finite,														arginfo_is_finite)
	PHP_FE(is_nan,															arginfo_is_nan)
	PHP_FE(is_infinite,														arginfo_is_infinite)
	PHP_FE(pow,																arginfo_pow)
	PHP_FE(exp,																arginfo_exp)
	PHP_FE(log,																arginfo_log)
	PHP_FE(log10,															arginfo_log10)
	PHP_FE(sqrt,															arginfo_sqrt)
	PHP_FE(hypot,															arginfo_hypot)
	PHP_FE(deg2rad,															arginfo_deg2rad)
	PHP_FE(rad2deg,															arginfo_rad2deg)
	PHP_FE(bindec,															arginfo_bindec)
	PHP_FE(hexdec,															arginfo_hexdec)
	PHP_FE(octdec,															arginfo_octdec)
	PHP_FE(decbin,															arginfo_decbin)
	PHP_FE(decoct,															arginfo_decoct)
	PHP_FE(dechex,															arginfo_dechex)
	PHP_FE(base_convert,													arginfo_base_convert)
	PHP_FE(number_format,													arginfo_number_format)
	PHP_FE(fmod,															arginfo_fmod)
#ifdef HAVE_INET_NTOP
	PHP_RAW_NAMED_FE(inet_ntop,		php_inet_ntop,								arginfo_inet_ntop)
#endif
#ifdef HAVE_INET_PTON
	PHP_RAW_NAMED_FE(inet_pton,		php_inet_pton,								arginfo_inet_pton)
#endif
	PHP_FE(ip2long,															arginfo_ip2long)
	PHP_FE(long2ip,															arginfo_long2ip)

	PHP_FE(getenv,															arginfo_getenv)
#ifdef HAVE_PUTENV
	PHP_FE(putenv,															arginfo_putenv)
#endif

	PHP_FE(getopt,															arginfo_getopt)

#ifdef HAVE_GETLOADAVG
	PHP_FE(sys_getloadavg,													arginfo_sys_getloadavg)
#endif
#ifdef HAVE_GETTIMEOFDAY
	PHP_FE(microtime,														arginfo_microtime)
	PHP_FE(gettimeofday,													arginfo_gettimeofday)
#endif

#ifdef HAVE_GETRUSAGE
	PHP_FE(getrusage,														arginfo_getrusage)
#endif

#ifdef HAVE_GETTIMEOFDAY
	PHP_FE(uniqid,															arginfo_uniqid)
#endif

	PHP_FE(quoted_printable_decode,											arginfo_quoted_printable_decode)
	PHP_FE(quoted_printable_encode,											arginfo_quoted_printable_encode)
	PHP_FE(convert_cyr_string,												arginfo_convert_cyr_string)
	PHP_FE(get_current_user,												arginfo_get_current_user)
	PHP_FE(set_time_limit,													arginfo_set_time_limit)
	PHP_FE(header_register_callback,										arginfo_header_register_callback)
	PHP_FE(get_cfg_var,														arginfo_get_cfg_var)

	PHP_DEP_FALIAS(magic_quotes_runtime,	set_magic_quotes_runtime,		arginfo_set_magic_quotes_runtime)
	PHP_DEP_FE(set_magic_quotes_runtime,									arginfo_set_magic_quotes_runtime)
	PHP_FE(get_magic_quotes_gpc,										arginfo_get_magic_quotes_gpc)
	PHP_FE(get_magic_quotes_runtime,									arginfo_get_magic_quotes_runtime)

	PHP_FE(error_log,														arginfo_error_log)
	PHP_FE(error_get_last,													arginfo_error_get_last)
	PHP_FE(call_user_func,													arginfo_call_user_func)
	PHP_FE(call_user_func_array,											arginfo_call_user_func_array)
	PHP_DEP_FE(call_user_method,											arginfo_call_user_method)
	PHP_DEP_FE(call_user_method_array,										arginfo_call_user_method_array)
	PHP_FE(forward_static_call,											arginfo_forward_static_call)
	PHP_FE(forward_static_call_array,										arginfo_forward_static_call_array)
	PHP_FE(serialize,														arginfo_serialize)
	PHP_FE(unserialize,														arginfo_unserialize)

	PHP_FE(var_dump,														arginfo_var_dump)
	PHP_FE(var_export,														arginfo_var_export)
	PHP_FE(debug_zval_dump,													arginfo_debug_zval_dump)
	PHP_FE(print_r,															arginfo_print_r)
	PHP_FE(memory_get_usage,												arginfo_memory_get_usage)
	PHP_FE(memory_get_peak_usage,											arginfo_memory_get_peak_usage)

	PHP_FE(register_shutdown_function,										arginfo_register_shutdown_function)
	PHP_FE(register_tick_function,											arginfo_register_tick_function)
	PHP_FE(unregister_tick_function,										arginfo_unregister_tick_function)

	PHP_FE(highlight_file,													arginfo_highlight_file)
	PHP_FALIAS(show_source,			highlight_file,							arginfo_highlight_file)
	PHP_FE(highlight_string,												arginfo_highlight_string)
	PHP_FE(php_strip_whitespace,											arginfo_php_strip_whitespace)

	PHP_FE(ini_get,															arginfo_ini_get)
	PHP_FE(ini_get_all,														arginfo_ini_get_all)
	PHP_FE(ini_set,															arginfo_ini_set)
	PHP_FALIAS(ini_alter,			ini_set,								arginfo_ini_set)
	PHP_FE(ini_restore,														arginfo_ini_restore)
	PHP_FE(get_include_path,												arginfo_get_include_path)
	PHP_FE(set_include_path,												arginfo_set_include_path)
	PHP_FE(restore_include_path,											arginfo_restore_include_path)

	PHP_FE(setcookie,														arginfo_setcookie)
	PHP_FE(setrawcookie,													arginfo_setrawcookie)
	PHP_FE(header,															arginfo_header)
	PHP_FE(header_remove,													arginfo_header_remove)
	PHP_FE(headers_sent,													arginfo_headers_sent)
	PHP_FE(headers_list,													arginfo_headers_list)
	PHP_FE(http_response_code,												arginfo_http_response_code)

	PHP_FE(connection_aborted,												arginfo_connection_aborted)
	PHP_FE(connection_status,												arginfo_connection_status)
	PHP_FE(ignore_user_abort,												arginfo_ignore_user_abort)
	PHP_FE(parse_ini_file,													arginfo_parse_ini_file)
	PHP_FE(parse_ini_string,												arginfo_parse_ini_string)
#if ZEND_DEBUG
	PHP_FE(config_get_hash,													arginfo_config_get_hash)
#endif
	PHP_FE(is_uploaded_file,												arginfo_is_uploaded_file)
	PHP_FE(move_uploaded_file,												arginfo_move_uploaded_file)

	/* functions from dns.c */
	PHP_FE(gethostbyaddr,													arginfo_gethostbyaddr)
	PHP_FE(gethostbyname,													arginfo_gethostbyname)
	PHP_FE(gethostbynamel,													arginfo_gethostbynamel)

#ifdef HAVE_GETHOSTNAME
	PHP_FE(gethostname,													arginfo_gethostname)
#endif

#if defined(PHP_WIN32) || (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE)))

	PHP_FE(dns_check_record,												arginfo_dns_check_record)
	PHP_FALIAS(checkdnsrr,			dns_check_record,						arginfo_dns_check_record)

# if defined(PHP_WIN32) || HAVE_FULL_DNS_FUNCS
	PHP_FE(dns_get_mx,														arginfo_dns_get_mx)
	PHP_FALIAS(getmxrr,				dns_get_mx,					arginfo_dns_get_mx)
	PHP_FE(dns_get_record,													arginfo_dns_get_record)
# endif
#endif

	/* functions from type.c */
	PHP_FE(intval,															arginfo_intval)
	PHP_FE(floatval,														arginfo_floatval)
	PHP_FALIAS(doubleval,			floatval,								arginfo_floatval)
	PHP_FE(strval,															arginfo_strval)
	PHP_FE(boolval,															arginfo_boolval)
	PHP_FE(gettype,															arginfo_gettype)
	PHP_FE(settype,															arginfo_settype)
	PHP_FE(is_null,															arginfo_is_null)
	PHP_FE(is_resource,														arginfo_is_resource)
	PHP_FE(is_bool,															arginfo_is_bool)
	PHP_FE(is_long,															arginfo_is_long)
	PHP_FE(is_float,														arginfo_is_float)
	PHP_FALIAS(is_int,				is_long,								arginfo_is_long)
	PHP_FALIAS(is_integer,			is_long,								arginfo_is_long)
	PHP_FALIAS(is_double,			is_float,								arginfo_is_float)
	PHP_FALIAS(is_real,				is_float,								arginfo_is_float)
	PHP_FE(is_numeric,														arginfo_is_numeric)
	PHP_FE(is_string,														arginfo_is_string)
	PHP_FE(is_array,														arginfo_is_array)
	PHP_FE(is_object,														arginfo_is_object)
	PHP_FE(is_scalar,														arginfo_is_scalar)
	PHP_FE(is_callable,														arginfo_is_callable)

	/* functions from file.c */
	PHP_FE(pclose,															arginfo_pclose)
	PHP_FE(popen,															arginfo_popen)
	PHP_FE(readfile,														arginfo_readfile)
	PHP_FE(rewind,															arginfo_rewind)
	PHP_FE(rmdir,															arginfo_rmdir)
	PHP_FE(umask,															arginfo_umask)
	PHP_FE(fclose,															arginfo_fclose)
	PHP_FE(feof,															arginfo_feof)
	PHP_FE(fgetc,															arginfo_fgetc)
	PHP_FE(fgets,															arginfo_fgets)
	PHP_FE(fgetss,															arginfo_fgetss)
	PHP_FE(fread,															arginfo_fread)
	PHP_NAMED_FE(fopen,				php_if_fopen,							arginfo_fopen)
	PHP_FE(fpassthru,														arginfo_fpassthru)
	PHP_NAMED_FE(ftruncate,			php_if_ftruncate,						arginfo_ftruncate)
	PHP_NAMED_FE(fstat,				php_if_fstat,							arginfo_fstat)
	PHP_FE(fseek,															arginfo_fseek)
	PHP_FE(ftell,															arginfo_ftell)
	PHP_FE(fflush,															arginfo_fflush)
	PHP_FE(fwrite,															arginfo_fwrite)
	PHP_FALIAS(fputs,				fwrite,									arginfo_fwrite)
	PHP_FE(mkdir,															arginfo_mkdir)
	PHP_FE(rename,															arginfo_rename)
	PHP_FE(copy,															arginfo_copy)
	PHP_FE(tempnam,															arginfo_tempnam)
	PHP_NAMED_FE(tmpfile,			php_if_tmpfile,							arginfo_tmpfile)
	PHP_FE(file,															arginfo_file)
	PHP_FE(file_get_contents,												arginfo_file_get_contents)
	PHP_FE(file_put_contents,												arginfo_file_put_contents)
	PHP_FE(stream_select,													arginfo_stream_select)
	PHP_FE(stream_context_create,											arginfo_stream_context_create)
	PHP_FE(stream_context_set_params,										arginfo_stream_context_set_params)
	PHP_FE(stream_context_get_params,										arginfo_stream_context_get_params)
	PHP_FE(stream_context_set_option,										arginfo_stream_context_set_option)
	PHP_FE(stream_context_get_options,										arginfo_stream_context_get_options)
	PHP_FE(stream_context_get_default,										arginfo_stream_context_get_default)
	PHP_FE(stream_context_set_default,										arginfo_stream_context_set_default)
	PHP_FE(stream_filter_prepend,											arginfo_stream_filter_prepend)
	PHP_FE(stream_filter_append,											arginfo_stream_filter_append)
	PHP_FE(stream_filter_remove,											arginfo_stream_filter_remove)
	PHP_FE(stream_socket_client,											arginfo_stream_socket_client)
	PHP_FE(stream_socket_server,											arginfo_stream_socket_server)
	PHP_FE(stream_socket_accept,											arginfo_stream_socket_accept)
	PHP_FE(stream_socket_get_name,											arginfo_stream_socket_get_name)
	PHP_FE(stream_socket_recvfrom,											arginfo_stream_socket_recvfrom)
	PHP_FE(stream_socket_sendto,											arginfo_stream_socket_sendto)
	PHP_FE(stream_socket_enable_crypto,										arginfo_stream_socket_enable_crypto)
#ifdef HAVE_SHUTDOWN
	PHP_FE(stream_socket_shutdown,											arginfo_stream_socket_shutdown)
#endif
#if HAVE_SOCKETPAIR
	PHP_FE(stream_socket_pair,												arginfo_stream_socket_pair)
#endif
	PHP_FE(stream_copy_to_stream,											arginfo_stream_copy_to_stream)
	PHP_FE(stream_get_contents,												arginfo_stream_get_contents)
	PHP_FE(stream_supports_lock,											arginfo_stream_supports_lock)
	PHP_FE(fgetcsv,															arginfo_fgetcsv)
	PHP_FE(fputcsv,															arginfo_fputcsv)
	PHP_FE(flock,															arginfo_flock)
	PHP_FE(get_meta_tags,													arginfo_get_meta_tags)
	PHP_FE(stream_set_read_buffer,											arginfo_stream_set_read_buffer)
	PHP_FE(stream_set_write_buffer,											arginfo_stream_set_write_buffer)
	PHP_FALIAS(set_file_buffer, stream_set_write_buffer,					arginfo_stream_set_write_buffer)
	PHP_FE(stream_set_chunk_size,											arginfo_stream_set_chunk_size)

	PHP_DEP_FALIAS(set_socket_blocking, stream_set_blocking,				arginfo_stream_set_blocking)
	PHP_FE(stream_set_blocking,												arginfo_stream_set_blocking)
	PHP_FALIAS(socket_set_blocking, stream_set_blocking,					arginfo_stream_set_blocking)

	PHP_FE(stream_get_meta_data,											arginfo_stream_get_meta_data)
	PHP_FE(stream_get_line,													arginfo_stream_get_line)
	PHP_FE(stream_wrapper_register,											arginfo_stream_wrapper_register)
	PHP_FALIAS(stream_register_wrapper, stream_wrapper_register,			arginfo_stream_wrapper_register)
	PHP_FE(stream_wrapper_unregister,										arginfo_stream_wrapper_unregister)
	PHP_FE(stream_wrapper_restore,											arginfo_stream_wrapper_restore)
	PHP_FE(stream_get_wrappers,												arginfo_stream_get_wrappers)
	PHP_FE(stream_get_transports,											arginfo_stream_get_transports)
	PHP_FE(stream_resolve_include_path,										arginfo_stream_resolve_include_path)
	PHP_FE(stream_is_local,												arginfo_stream_is_local)
	PHP_FE(get_headers,														arginfo_get_headers)

#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
	PHP_FE(stream_set_timeout,												arginfo_stream_set_timeout)
	PHP_FALIAS(socket_set_timeout, stream_set_timeout,						arginfo_stream_set_timeout)
#endif

	PHP_FALIAS(socket_get_status, stream_get_meta_data,						arginfo_stream_get_meta_data)

#if (!defined(__BEOS__) && !defined(NETWARE) && HAVE_REALPATH) || defined(ZTS)
	PHP_FE(realpath,														arginfo_realpath)
#endif

#ifdef HAVE_FNMATCH
	PHP_FE(fnmatch,															arginfo_fnmatch)
#endif

	/* functions from fsock.c */
	PHP_FE(fsockopen,														arginfo_fsockopen)
	PHP_FE(pfsockopen,														arginfo_pfsockopen)

	/* functions from pack.c */
	PHP_FE(pack,															arginfo_pack)
	PHP_FE(unpack,															arginfo_unpack)

	/* functions from browscap.c */
	PHP_FE(get_browser,														arginfo_get_browser)

#if HAVE_CRYPT
	/* functions from crypt.c */
	PHP_FE(crypt,															arginfo_crypt)
#endif

	/* functions from dir.c */
	PHP_FE(opendir,															arginfo_opendir)
	PHP_FE(closedir,														arginfo_closedir)
	PHP_FE(chdir,															arginfo_chdir)

#if defined(HAVE_CHROOT) && !defined(ZTS) && ENABLE_CHROOT_FUNC
	PHP_FE(chroot,															arginfo_chroot)
#endif

	PHP_FE(getcwd,															arginfo_getcwd)
	PHP_FE(rewinddir,														arginfo_rewinddir)
	PHP_NAMED_FE(readdir,			php_if_readdir,							arginfo_readdir)
	PHP_FALIAS(dir,					getdir,									arginfo_dir)
	PHP_FE(scandir,															arginfo_scandir)
#ifdef HAVE_GLOB
	PHP_FE(glob,															arginfo_glob)
#endif
	/* functions from filestat.c */
	PHP_FE(fileatime,														arginfo_fileatime)
	PHP_FE(filectime,														arginfo_filectime)
	PHP_FE(filegroup,														arginfo_filegroup)
	PHP_FE(fileinode,														arginfo_fileinode)
	PHP_FE(filemtime,														arginfo_filemtime)
	PHP_FE(fileowner,														arginfo_fileowner)
	PHP_FE(fileperms,														arginfo_fileperms)
	PHP_FE(filesize,														arginfo_filesize)
	PHP_FE(filetype,														arginfo_filetype)
	PHP_FE(file_exists,														arginfo_file_exists)
	PHP_FE(is_writable,														arginfo_is_writable)
	PHP_FALIAS(is_writeable,		is_writable,							arginfo_is_writable)
	PHP_FE(is_readable,														arginfo_is_readable)
	PHP_FE(is_executable,													arginfo_is_executable)
	PHP_FE(is_file,															arginfo_is_file)
	PHP_FE(is_dir,															arginfo_is_dir)
	PHP_FE(is_link,															arginfo_is_link)
	PHP_NAMED_FE(stat,				php_if_stat,							arginfo_stat)
	PHP_NAMED_FE(lstat,				php_if_lstat,							arginfo_lstat)
#ifndef NETWARE
	PHP_FE(chown,															arginfo_chown)
	PHP_FE(chgrp,															arginfo_chgrp)
#endif
#if HAVE_LCHOWN
	PHP_FE(lchown,															arginfo_lchown)
#endif
#if HAVE_LCHOWN
	PHP_FE(lchgrp,															arginfo_lchgrp)
#endif
	PHP_FE(chmod,															arginfo_chmod)
#if HAVE_UTIME
	PHP_FE(touch,															arginfo_touch)
#endif
	PHP_FE(clearstatcache,													arginfo_clearstatcache)
	PHP_FE(disk_total_space,												arginfo_disk_total_space)
	PHP_FE(disk_free_space,													arginfo_disk_free_space)
	PHP_FALIAS(diskfreespace,		disk_free_space,						arginfo_disk_free_space)
	PHP_FE(realpath_cache_size,												arginfo_realpath_cache_size)
	PHP_FE(realpath_cache_get,												arginfo_realpath_cache_get)

	/* functions from mail.c */
	PHP_FE(mail,															arginfo_mail)
	PHP_FE(ezmlm_hash,														arginfo_ezmlm_hash)

	/* functions from syslog.c */
#ifdef HAVE_SYSLOG_H
	PHP_FE(openlog,															arginfo_openlog)
	PHP_FE(syslog,															arginfo_syslog)
	PHP_FE(closelog,														arginfo_closelog)
#endif

	/* functions from lcg.c */
	PHP_FE(lcg_value,														arginfo_lcg_value)

	/* functions from metaphone.c */
	PHP_FE(metaphone,														arginfo_metaphone)

	/* functions from output.c */
	PHP_FE(ob_start,														arginfo_ob_start)
	PHP_FE(ob_flush,														arginfo_ob_flush)
	PHP_FE(ob_clean,														arginfo_ob_clean)
	PHP_FE(ob_end_flush,													arginfo_ob_end_flush)
	PHP_FE(ob_end_clean,													arginfo_ob_end_clean)
	PHP_FE(ob_get_flush,													arginfo_ob_get_flush)
	PHP_FE(ob_get_clean,													arginfo_ob_get_clean)
	PHP_FE(ob_get_length,													arginfo_ob_get_length)
	PHP_FE(ob_get_level,													arginfo_ob_get_level)
	PHP_FE(ob_get_status,													arginfo_ob_get_status)
	PHP_FE(ob_get_contents,													arginfo_ob_get_contents)
	PHP_FE(ob_implicit_flush,												arginfo_ob_implicit_flush)
	PHP_FE(ob_list_handlers,												arginfo_ob_list_handlers)

	/* functions from array.c */
	PHP_FE(ksort,															arginfo_ksort)
	PHP_FE(krsort,															arginfo_krsort)
	PHP_FE(natsort,															arginfo_natsort)
	PHP_FE(natcasesort,														arginfo_natcasesort)
	PHP_FE(asort,															arginfo_asort)
	PHP_FE(arsort,															arginfo_arsort)
	PHP_FE(sort,															arginfo_sort)
	PHP_FE(rsort,															arginfo_rsort)
	PHP_FE(usort,															arginfo_usort)
	PHP_FE(uasort,															arginfo_uasort)
	PHP_FE(uksort,															arginfo_uksort)
	PHP_FE(shuffle,															arginfo_shuffle)
	PHP_FE(array_walk,														arginfo_array_walk)
	PHP_FE(array_walk_recursive,											arginfo_array_walk_recursive)
	PHP_FE(count,															arginfo_count)
	PHP_FE(end,																arginfo_end)
	PHP_FE(prev,															arginfo_prev)
	PHP_FE(next,															arginfo_next)
	PHP_FE(reset,															arginfo_reset)
	PHP_FE(current,															arginfo_current)
	PHP_FE(key,																arginfo_key)
	PHP_FE(min,																arginfo_min)
	PHP_FE(max,																arginfo_max)
	PHP_FE(in_array,														arginfo_in_array)
	PHP_FE(array_search,													arginfo_array_search)
	PHP_FE(extract,															arginfo_extract)
	PHP_FE(compact,															arginfo_compact)
	PHP_FE(array_fill,														arginfo_array_fill)
	PHP_FE(array_fill_keys,													arginfo_array_fill_keys)
	PHP_FE(range,															arginfo_range)
	PHP_FE(array_multisort,													arginfo_array_multisort)
	PHP_FE(array_push,														arginfo_array_push)
	PHP_FE(array_pop,														arginfo_array_pop)
	PHP_FE(array_shift,														arginfo_array_shift)
	PHP_FE(array_unshift,													arginfo_array_unshift)
	PHP_FE(array_splice,													arginfo_array_splice)
	PHP_FE(array_slice,														arginfo_array_slice)
	PHP_FE(array_merge,														arginfo_array_merge)
	PHP_FE(array_merge_recursive,											arginfo_array_merge_recursive)
	PHP_FE(array_replace,													arginfo_array_replace)
	PHP_FE(array_replace_recursive,											arginfo_array_replace_recursive)
	PHP_FE(array_keys,														arginfo_array_keys)
	PHP_FE(array_values,													arginfo_array_values)
	PHP_FE(array_count_values,												arginfo_array_count_values)
	PHP_FE(array_column,													arginfo_array_column)
	PHP_FE(array_reverse,													arginfo_array_reverse)
	PHP_FE(array_reduce,													arginfo_array_reduce)
	PHP_FE(array_pad,														arginfo_array_pad)
	PHP_FE(array_flip,														arginfo_array_flip)
	PHP_FE(array_change_key_case,											arginfo_array_change_key_case)
	PHP_FE(array_rand,														arginfo_array_rand)
	PHP_FE(array_unique,													arginfo_array_unique)
	PHP_FE(array_intersect,													arginfo_array_intersect)
	PHP_FE(array_intersect_key,												arginfo_array_intersect_key)
	PHP_FE(array_intersect_ukey,											arginfo_array_intersect_ukey)
	PHP_FE(array_uintersect,												arginfo_array_uintersect)
	PHP_FE(array_intersect_assoc,											arginfo_array_intersect_assoc)
	PHP_FE(array_uintersect_assoc,											arginfo_array_uintersect_assoc)
	PHP_FE(array_intersect_uassoc,											arginfo_array_intersect_uassoc)
	PHP_FE(array_uintersect_uassoc,											arginfo_array_uintersect_uassoc)
	PHP_FE(array_diff,														arginfo_array_diff)
	PHP_FE(array_diff_key,													arginfo_array_diff_key)
	PHP_FE(array_diff_ukey,													arginfo_array_diff_ukey)
	PHP_FE(array_udiff,														arginfo_array_udiff)
	PHP_FE(array_diff_assoc,												arginfo_array_diff_assoc)
	PHP_FE(array_udiff_assoc,												arginfo_array_udiff_assoc)
	PHP_FE(array_diff_uassoc,												arginfo_array_diff_uassoc)
	PHP_FE(array_udiff_uassoc,												arginfo_array_udiff_uassoc)
	PHP_FE(array_sum,														arginfo_array_sum)
	PHP_FE(array_product,													arginfo_array_product)
	PHP_FE(array_filter,													arginfo_array_filter)
	PHP_FE(array_map,														arginfo_array_map)
	PHP_FE(array_chunk,														arginfo_array_chunk)
	PHP_FE(array_combine,													arginfo_array_combine)
	PHP_FE(array_key_exists,												arginfo_array_key_exists)

	/* aliases from array.c */
	PHP_FALIAS(pos,					current,								arginfo_current)
	PHP_FALIAS(sizeof,				count,									arginfo_count)
	PHP_FALIAS(key_exists,			array_key_exists,						arginfo_array_key_exists)

	/* functions from assert.c */
	PHP_FE(assert,															arginfo_assert)
	PHP_FE(assert_options,													arginfo_assert_options)

	/* functions from versioning.c */
	PHP_FE(version_compare,													arginfo_version_compare)

	/* functions from ftok.c*/
#if HAVE_FTOK
	PHP_FE(ftok,															arginfo_ftok)
#endif

	PHP_FE(str_rot13,														arginfo_str_rot13)
	PHP_FE(stream_get_filters,												arginfo_stream_get_filters)
	PHP_FE(stream_filter_register,											arginfo_stream_filter_register)
	PHP_FE(stream_bucket_make_writeable,									arginfo_stream_bucket_make_writeable)
	PHP_FE(stream_bucket_prepend,											arginfo_stream_bucket_prepend)
	PHP_FE(stream_bucket_append,											arginfo_stream_bucket_append)
	PHP_FE(stream_bucket_new,												arginfo_stream_bucket_new)

	PHP_FE(output_add_rewrite_var,											arginfo_output_add_rewrite_var)
	PHP_FE(output_reset_rewrite_vars,										arginfo_output_reset_rewrite_vars)

	PHP_FE(sys_get_temp_dir,												arginfo_sys_get_temp_dir)

	PHP_FE_END
};
/* }}} */

static const zend_module_dep standard_deps[] = { /* {{{ */
	ZEND_MOD_OPTIONAL("session")
	ZEND_MOD_END
};
/* }}} */

zend_module_entry basic_functions_module = { /* {{{ */
	STANDARD_MODULE_HEADER_EX,
	NULL,
	standard_deps,
	"standard",					/* extension name */
	basic_functions,			/* function list */
	PHP_MINIT(basic),			/* process startup */
	PHP_MSHUTDOWN(basic),		/* process shutdown */
	PHP_RINIT(basic),			/* request startup */
	PHP_RSHUTDOWN(basic),		/* request shutdown */
	PHP_MINFO(basic),			/* extension info */
	PHP_VERSION,				/* extension version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#if defined(HAVE_PUTENV)
static void php_putenv_destructor(putenv_entry *pe) /* {{{ */
{
	if (pe->previous_value) {
#if _MSC_VER >= 1300
		/* VS.Net has a bug in putenv() when setting a variable that
		 * is already set; if the SetEnvironmentVariable() API call
		 * fails, the Crt will double free() a string.
		 * We try to avoid this by setting our own value first */
		SetEnvironmentVariable(pe->key, "bugbug");
#endif
		putenv(pe->previous_value);
# if defined(PHP_WIN32)
		efree(pe->previous_value);
# endif
	} else {
# if HAVE_UNSETENV
		unsetenv(pe->key);
# elif defined(PHP_WIN32)
		SetEnvironmentVariable(pe->key, NULL);
# else
		char **env;

		for (env = environ; env != NULL && *env != NULL; env++) {
			if (!strncmp(*env, pe->key, pe->key_len) && (*env)[pe->key_len] == '=') {	/* found it */
				*env = "";
				break;
			}
		}
# endif
	}
#ifdef HAVE_TZSET
	/* don't forget to reset the various libc globals that
	 * we might have changed by an earlier call to tzset(). */
	if (!strncmp(pe->key, "TZ", pe->key_len)) {
		tzset();
	}
#endif

	efree(pe->putenv_string);
	efree(pe->key);
}