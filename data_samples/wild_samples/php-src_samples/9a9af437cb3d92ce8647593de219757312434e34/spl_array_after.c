{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	php_int_t index;
	zval *rv, *value = NULL, **tmp;

	if (check_inherited && intern->fptr_offset_has) {
		zval *offset_tmp = offset;
		SEPARATE_ARG_IF_REF(offset_tmp);
		zend_call_method_with_1_params(&object, Z_OBJCE_P(object), &intern->fptr_offset_has, "offsetExists", &rv, offset_tmp);
		zval_ptr_dtor(&offset_tmp);

		if (rv && zend_is_true(rv TSRMLS_CC)) {
			zval_ptr_dtor(&rv);
			if (check_empty == 2) {
				return 1;
			} else if (intern->fptr_offset_get) {
				value = spl_array_read_dimension_ex(1, object, offset, BP_VAR_R TSRMLS_CC);
			}
		} else {
			if (rv) {
				zval_ptr_dtor(&rv);
			}
			return 0;
		}
	}

	if (!value) {
		HashTable *ht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

		switch(Z_TYPE_P(offset)) {
			case IS_STRING: 
				if (zend_symtable_find(ht, Z_STRVAL_P(offset), Z_STRSIZE_P(offset)+1, (void **) &tmp) != FAILURE) {
					if (check_empty == 2) {
						return 1;
					}
				} else {
					return 0;
				}
				break;
			case IS_DOUBLE:
			case IS_RESOURCE:
			case IS_BOOL: 
			case IS_INT:
				if (offset->type == IS_DOUBLE) {
					index = (php_int_t)Z_DVAL_P(offset);
				} else {
					index = Z_IVAL_P(offset);
				}
				if (zend_hash_index_find(ht, index, (void **)&tmp) != FAILURE) {
					if (check_empty == 2) {
						return 1;
					}
				} else {
					return 0;
				}
				break;
			default:
				zend_error(E_WARNING, "Illegal offset type");
				return 0;
		}

		if (check_inherited && intern->fptr_offset_get) {
			value = spl_array_read_dimension_ex(1, object, offset, BP_VAR_R TSRMLS_CC);
		} else {
			value = *tmp;
		}
	}

	switch (check_empty) {
		case 0:
			return Z_TYPE_P(value) != IS_NULL;
		case 2:
			return 1;
		case 1:
			return zend_is_true(value);
	}

	return 0;
} /* }}} */

static int spl_array_has_dimension(zval *object, zval *offset, int check_empty TSRMLS_DC) /* {{{ */
{
	return spl_array_has_dimension_ex(1, object, offset, check_empty TSRMLS_CC);
} /* }}} */

/* {{{ spl_array_object_verify_pos_ex */
static inline int spl_array_object_verify_pos_ex(spl_array_object *object, HashTable *ht, const char *msg_prefix TSRMLS_DC)
{
	if (!ht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "%sArray was modified outside object and is no longer an array", msg_prefix);
		return FAILURE;
	}

	if (object->pos && (object->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(object, ht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "%sArray was modified outside object and internal position is no longer valid", msg_prefix);
		return FAILURE;
	}

	return SUCCESS;
} /* }}} */

/* {{{ spl_array_object_verify_pos */
static inline int spl_array_object_verify_pos(spl_array_object *object, HashTable *ht TSRMLS_DC)
{
	return spl_array_object_verify_pos_ex(object, ht, "" TSRMLS_CC);
} /* }}} */

/* {{{ proto bool ArrayObject::offsetExists(mixed $index)
       proto bool ArrayIterator::offsetExists(mixed $index)
   Returns whether the requested $index exists. */
SPL_METHOD(Array, offsetExists)
{
	zval *index;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &index) == FAILURE) {
		return;
	}
	RETURN_BOOL(spl_array_has_dimension_ex(0, getThis(), index, 2 TSRMLS_CC));
} /* }}} */

/* {{{ proto mixed ArrayObject::offsetGet(mixed $index)
       proto mixed ArrayIterator::offsetGet(mixed $index)
   Returns the value at the specified $index. */
SPL_METHOD(Array, offsetGet)
{
	zval *index, *value;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &index) == FAILURE) {
		return;
	}
	value = spl_array_read_dimension_ex(0, getThis(), index, BP_VAR_R TSRMLS_CC);
	RETURN_ZVAL(value, 1, 0);
} /* }}} */

/* {{{ proto void ArrayObject::offsetSet(mixed $index, mixed $newval)
       proto void ArrayIterator::offsetSet(mixed $index, mixed $newval)
   Sets the value at the specified $index to $newval. */
SPL_METHOD(Array, offsetSet)
{
	zval *index, *value;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &index, &value) == FAILURE) {
		return;
	}
	spl_array_write_dimension_ex(0, getThis(), index, value TSRMLS_CC);
} /* }}} */

void spl_array_iterator_append(zval *object, zval *append_value TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Cannot append properties to objects, use %s::offsetSet() instead", Z_OBJCE_P(object)->name);
		return;
	}

	spl_array_write_dimension(object, NULL, append_value TSRMLS_CC);
	if (!intern->pos) {
		spl_array_set_pos(intern, aht->pListTail);
	}
} /* }}} */

/* {{{ proto void ArrayObject::append(mixed $newval)
       proto void ArrayIterator::append(mixed $newval)
   Appends the value (cannot be called for objects). */
SPL_METHOD(Array, append)
{
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
		return;
	}
	spl_array_iterator_append(getThis(), value TSRMLS_CC);
} /* }}} */

/* {{{ proto void ArrayObject::offsetUnset(mixed $index)
       proto void ArrayIterator::offsetUnset(mixed $index)
   Unsets the value at the specified $index. */
SPL_METHOD(Array, offsetUnset)
{
	zval *index;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &index) == FAILURE) {
		return;
	}
	spl_array_unset_dimension_ex(0, getThis(), index TSRMLS_CC);
} /* }}} */

/* {{{ proto array ArrayObject::getArrayCopy()
      proto array ArrayIterator::getArrayCopy()
   Return a copy of the contained array */
SPL_METHOD(Array, getArrayCopy)
{
	zval *object = getThis(), *tmp;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

    array_init(return_value);
	zend_hash_copy(HASH_OF(return_value), spl_array_get_hash_table(intern, 0 TSRMLS_CC), (copy_ctor_func_t) zval_add_ref, &tmp, sizeof(zval*));
} /* }}} */

static HashTable *spl_array_get_properties(zval *object TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *result;

	if (intern->nApplyCount > 1) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Nesting level too deep - recursive dependency?");
	}

	intern->nApplyCount++;
	result = spl_array_get_hash_table(intern, 1 TSRMLS_CC);
	intern->nApplyCount--;
	return result;
} /* }}} */

static HashTable* spl_array_get_debug_info(zval *obj, int *is_temp TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(obj TSRMLS_CC);
	zval *tmp, *storage;
	php_size_t name_len;
	char *zname;
	zend_class_entry *base;

	*is_temp = 0;

	if (!intern->std.properties) {
		rebuild_object_properties(&intern->std);
	}

	if (HASH_OF(intern->array) == intern->std.properties) {
		return intern->std.properties;
	} else {
		if (intern->debug_info == NULL) {
			ALLOC_HASHTABLE(intern->debug_info);
			ZEND_INIT_SYMTABLE_EX(intern->debug_info, zend_hash_num_elements(intern->std.properties) + 1, 0);
		}

		if (intern->debug_info->nApplyCount == 0) {
			zend_hash_clean(intern->debug_info);
			zend_hash_copy(intern->debug_info, intern->std.properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

			storage = intern->array;
			zval_add_ref(&storage);

			base = (Z_OBJ_HT_P(obj) == &spl_handler_ArrayIterator) ? spl_ce_ArrayIterator : spl_ce_ArrayObject;
			zname = spl_gen_private_prop_name(base, "storage", sizeof("storage")-1, &name_len TSRMLS_CC);
			zend_symtable_update(intern->debug_info, zname, name_len+1, &storage, sizeof(zval *), NULL);
			efree(zname);
		}

		return intern->debug_info;
	}
}
/* }}} */

static zval *spl_array_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2, key TSRMLS_CC)) {
		return spl_array_read_dimension(object, member, type TSRMLS_CC);
	}
	return std_object_handlers.read_property(object, member, type, key TSRMLS_CC);
} /* }}} */

static void spl_array_write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2, key TSRMLS_CC)) {
		spl_array_write_dimension(object, member, value TSRMLS_CC);
		return;
	}
	std_object_handlers.write_property(object, member, value, key TSRMLS_CC);
} /* }}} */

static zval **spl_array_get_property_ptr_ptr(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2, key TSRMLS_CC)) {
		return spl_array_get_dimension_ptr_ptr(1, object, member, type TSRMLS_CC);
	}
	return std_object_handlers.get_property_ptr_ptr(object, member, type, key TSRMLS_CC);
} /* }}} */

static int spl_array_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2, key TSRMLS_CC)) {
		return spl_array_has_dimension(object, member, has_set_exists TSRMLS_CC);
	}
	return std_object_handlers.has_property(object, member, has_set_exists, key TSRMLS_CC);
} /* }}} */

static void spl_array_unset_property(zval *object, zval *member, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2, key TSRMLS_CC)) {
		spl_array_unset_dimension(object, member TSRMLS_CC);
		spl_array_rewind(intern TSRMLS_CC); /* because deletion might invalidate position */
		return;
	}
	std_object_handlers.unset_property(object, member, key TSRMLS_CC);
} /* }}} */

static int spl_array_compare_objects(zval *o1, zval *o2 TSRMLS_DC) /* {{{ */
{
	HashTable			*ht1,
						*ht2;
	spl_array_object	*intern1,
						*intern2;
	int					result	= 0;
	zval				temp_zv;

	intern1	= (spl_array_object*)zend_object_store_get_object(o1 TSRMLS_CC);
	intern2	= (spl_array_object*)zend_object_store_get_object(o2 TSRMLS_CC);
	ht1		= spl_array_get_hash_table(intern1, 0 TSRMLS_CC);
	ht2		= spl_array_get_hash_table(intern2, 0 TSRMLS_CC);

	zend_compare_symbol_tables(&temp_zv, ht1, ht2 TSRMLS_CC);
	result = (int)Z_IVAL(temp_zv);
	/* if we just compared std.properties, don't do it again */
	if (result == 0 &&
			!(ht1 == intern1->std.properties && ht2 == intern2->std.properties)) {
		result = std_object_handlers.compare_objects(o1, o2 TSRMLS_CC);
	}
	return result;
} /* }}} */

static int spl_array_skip_protected(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{
	char *string_key;
	php_size_t string_length;
	zend_uint_t num_key;

	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		do {
			if (zend_hash_get_current_key_ex(aht, &string_key, &string_length, &num_key, 0, &intern->pos) == HASH_KEY_IS_STRING) {
				if (!string_length || string_key[0]) {
					return SUCCESS;
				}
			} else {
				return SUCCESS;
			}
			if (zend_hash_has_more_elements_ex(aht, &intern->pos) != SUCCESS) {
				return FAILURE;
			}
			zend_hash_move_forward_ex(aht, &intern->pos);
			spl_array_update_pos(intern);
		} while (1);
	}
	return FAILURE;
} /* }}} */

static int spl_array_next_no_verify(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{
	zend_hash_move_forward_ex(aht, &intern->pos);
	spl_array_update_pos(intern);
	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		return spl_array_skip_protected(intern, aht TSRMLS_CC);
	} else {
		return zend_hash_has_more_elements_ex(aht, &intern->pos);
	}
} /* }}} */

static int spl_array_next_ex(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{
	if ((intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		return FAILURE;
	}

	return spl_array_next_no_verify(intern, aht TSRMLS_CC);
} /* }}} */

static int spl_array_next(spl_array_object *intern TSRMLS_DC) /* {{{ */
{
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	return spl_array_next_ex(intern, aht TSRMLS_CC);

} /* }}} */

/* define an overloaded iterator structure */
typedef struct {
	zend_user_iterator    intern;
	spl_array_object      *object;
} spl_array_it;

static void spl_array_it_dtor(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it *iterator = (spl_array_it *)iter;

	zend_user_it_invalidate_current(iter TSRMLS_CC);
	zval_ptr_dtor((zval**)&iterator->intern.it.data);

	efree(iterator);
}
/* }}} */

static int spl_array_it_valid(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_VALID) {
		return zend_user_it_valid(iter TSRMLS_CC);
	} else {
		if (spl_array_object_verify_pos_ex(object, aht, "ArrayIterator::valid(): " TSRMLS_CC) == FAILURE) {
			return FAILURE;
		}

		return zend_hash_has_more_elements_ex(aht, &object->pos);
	}
}
/* }}} */

static void spl_array_it_get_current_data(zend_object_iterator *iter, zval ***data TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_CURRENT) {
		zend_user_it_get_current_data(iter, data TSRMLS_CC);
	} else {
		if (zend_hash_get_current_data_ex(aht, (void**)data, &object->pos) == FAILURE) {
			*data = NULL;
		}
	}
}
/* }}} */

static void spl_array_it_get_current_key(zend_object_iterator *iter, zval *key TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_KEY) {
		zend_user_it_get_current_key(iter, key TSRMLS_CC);
	} else {
		if (spl_array_object_verify_pos_ex(object, aht, "ArrayIterator::current(): " TSRMLS_CC) == FAILURE) {
			ZVAL_NULL(key);
		} else {
			zend_hash_get_current_key_zval_ex(aht, key, &object->pos);
		}
	}
}
/* }}} */

static void spl_array_it_move_forward(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_NEXT) {
		zend_user_it_move_forward(iter TSRMLS_CC);
	} else {
		zend_user_it_invalidate_current(iter TSRMLS_CC);
		if (!aht) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::current(): Array was modified outside object and is no longer an array");
			return;
		}

		if ((object->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(object, aht TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::next(): Array was modified outside object and internal position is no longer valid");
		} else {
			spl_array_next_no_verify(object, aht TSRMLS_CC);
		}
	}
}
/* }}} */

static void spl_array_rewind_ex(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{

	zend_hash_internal_pointer_reset_ex(aht, &intern->pos);
	spl_array_update_pos(intern);
	spl_array_skip_protected(intern, aht TSRMLS_CC);

} /* }}} */

static void spl_array_rewind(spl_array_object *intern TSRMLS_DC) /* {{{ */
{
	HashTable          *aht      = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::rewind(): Array was modified outside object and is no longer an array");
		return;
	}

	spl_array_rewind_ex(intern, aht TSRMLS_CC);
}
/* }}} */

static void spl_array_it_rewind(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_REWIND) {
		zend_user_it_rewind(iter TSRMLS_CC);
	} else {
		zend_user_it_invalidate_current(iter TSRMLS_CC);
		spl_array_rewind(object TSRMLS_CC);
	}
}
/* }}} */

/* {{{ spl_array_set_array */
static void spl_array_set_array(zval *object, spl_array_object *intern, zval **array, php_int_t ar_flags, int just_array TSRMLS_DC) {

	if (Z_TYPE_PP(array) == IS_ARRAY) {
		SEPARATE_ZVAL_IF_NOT_REF(array);
	}

	if (Z_TYPE_PP(array) == IS_OBJECT && (Z_OBJ_HT_PP(array) == &spl_handler_ArrayObject || Z_OBJ_HT_PP(array) == &spl_handler_ArrayIterator)) {
		zval_ptr_dtor(&intern->array);
		if (just_array)	{
			spl_array_object *other = (spl_array_object*)zend_object_store_get_object(*array TSRMLS_CC);
			ar_flags = other->ar_flags & ~SPL_ARRAY_INT_MASK;
		}
		ar_flags |= SPL_ARRAY_USE_OTHER;
		intern->array = *array;
	} else {
		if (Z_TYPE_PP(array) != IS_OBJECT && Z_TYPE_PP(array) != IS_ARRAY) {
			zend_throw_exception(spl_ce_InvalidArgumentException, "Passed variable is not an array or object, using empty array instead", 0 TSRMLS_CC);
			return;
		}
		zval_ptr_dtor(&intern->array);
		intern->array = *array;
	}
	if (object == *array) {
		intern->ar_flags |= SPL_ARRAY_IS_SELF;
		intern->ar_flags &= ~SPL_ARRAY_USE_OTHER;
	} else {
		intern->ar_flags &= ~SPL_ARRAY_IS_SELF;
	}
	intern->ar_flags |= ar_flags;
	Z_ADDREF_P(intern->array);
	if (Z_TYPE_PP(array) == IS_OBJECT) {
		zend_object_get_properties_t handler = Z_OBJ_HANDLER_PP(array, get_properties);
		if ((handler != std_object_handlers.get_properties && handler != spl_array_get_properties)
		|| !spl_array_get_hash_table(intern, 0 TSRMLS_CC)) {
			zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0 TSRMLS_CC, "Overloaded object of type %s is not compatible with %s", Z_OBJCE_PP(array)->name, intern->std.ce->name);
		}
	}

	spl_array_rewind(intern TSRMLS_CC);
}
/* }}} */

/* iterator handler table */
zend_object_iterator_funcs spl_array_it_funcs = {
	spl_array_it_dtor,
	spl_array_it_valid,
	spl_array_it_get_current_data,
	spl_array_it_get_current_key,
	spl_array_it_move_forward,
	spl_array_it_rewind
};

zend_object_iterator *spl_array_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator;
	spl_array_object   *array_object = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (by_ref && (array_object->ar_flags & SPL_ARRAY_OVERLOADED_CURRENT)) {
		zend_error(E_ERROR, "An iterator cannot be used with foreach by reference");
	}

	iterator     = emalloc(sizeof(spl_array_it));

	Z_ADDREF_P(object);
	iterator->intern.it.data = (void*)object;
	iterator->intern.it.funcs = &spl_array_it_funcs;
	iterator->intern.ce = ce;
	iterator->intern.value = NULL;
	iterator->object = array_object;

	return (zend_object_iterator*)iterator;
}
/* }}} */

/* {{{ proto void ArrayObject::__construct(array|object ar = array() [, int flags = 0 [, string iterator_class = "ArrayIterator"]])
       proto void ArrayIterator::__construct(array|object ar = array() [, int flags = 0])
   Constructs a new array iterator from a path. */
SPL_METHOD(Array, __construct)
{
	zval *object = getThis();
	spl_array_object *intern;
	zval **array;
	php_int_t ar_flags = 0;
	zend_class_entry *ce_get_iterator = spl_ce_Iterator;
	zend_error_handling error_handling;

	if (ZEND_NUM_ARGS() == 0) {
		return; /* nothing to do */
	}

	zend_replace_error_handling(EH_THROW, spl_ce_InvalidArgumentException, &error_handling TSRMLS_CC);

	intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|iC", &array, &ar_flags, &ce_get_iterator) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (ZEND_NUM_ARGS() > 2) {
		intern->ce_get_iterator = ce_get_iterator;
	}

	ar_flags &= ~SPL_ARRAY_INT_MASK;

	spl_array_set_array(object, intern, array, ar_flags, ZEND_NUM_ARGS() == 1 TSRMLS_CC);

	zend_restore_error_handling(&error_handling TSRMLS_CC);

}
 /* }}} */

/* {{{ proto void ArrayObject::setIteratorClass(string iterator_class)
   Set the class used in getIterator. */
SPL_METHOD(Array, setIteratorClass)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	zend_class_entry * ce_get_iterator = spl_ce_Iterator;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "C", &ce_get_iterator) == FAILURE) {
		return;
	}

	intern->ce_get_iterator = ce_get_iterator;
}
/* }}} */

/* {{{ proto string ArrayObject::getIteratorClass()
   Get the class used in getIterator. */
SPL_METHOD(Array, getIteratorClass)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_STRING(intern->ce_get_iterator->name, 1);
}
/* }}} */

/* {{{ proto int ArrayObject::getFlags()
   Get flags */
SPL_METHOD(Array, getFlags)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_INT(intern->ar_flags & ~SPL_ARRAY_INT_MASK);
}
/* }}} */

/* {{{ proto void ArrayObject::setFlags(int flags)
   Set flags */
SPL_METHOD(Array, setFlags)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	php_int_t ar_flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "i", &ar_flags) == FAILURE) {
		return;
	}

	intern->ar_flags = (intern->ar_flags & SPL_ARRAY_INT_MASK) | (ar_flags & ~SPL_ARRAY_INT_MASK);
}
/* }}} */

/* {{{ proto Array|Object ArrayObject::exchangeArray(Array|Object ar = array())
   Replace the referenced array or object with a new one and return the old one (right now copy - to be changed) */
SPL_METHOD(Array, exchangeArray)
{
	zval *object = getThis(), *tmp, **array;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	array_init(return_value);
	zend_hash_copy(HASH_OF(return_value), spl_array_get_hash_table(intern, 0 TSRMLS_CC), (copy_ctor_func_t) zval_add_ref, &tmp, sizeof(zval*));

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z", &array) == FAILURE) {
		return;
	}

	spl_array_set_array(object, intern, array, 0L, 1 TSRMLS_CC);

}
/* }}} */

/* {{{ proto ArrayIterator ArrayObject::getIterator()
   Create a new iterator from a ArrayObject instance */
SPL_METHOD(Array, getIterator)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	spl_array_object *iterator;
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	return_value->type = IS_OBJECT;
	return_value->value.obj = spl_array_object_new_ex(intern->ce_get_iterator, &iterator, object, 0 TSRMLS_CC);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
}
/* }}} */

/* {{{ proto void ArrayIterator::rewind()
   Rewind array back to the start */
SPL_METHOD(Array, rewind)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	spl_array_rewind(intern TSRMLS_CC);
}
/* }}} */

/* {{{ proto void ArrayIterator::seek(int $position)
   Seek to position. */
SPL_METHOD(Array, seek)
{
	php_int_t opos, position;
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	int result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "i", &position) == FAILURE) {
		return;
	}

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	opos = position;

	if (position >= 0) { /* negative values are not supported */
		spl_array_rewind(intern TSRMLS_CC);
		result = SUCCESS;

		while (position-- > 0 && (result = spl_array_next(intern TSRMLS_CC)) == SUCCESS);

		if (result == SUCCESS && zend_hash_has_more_elements_ex(aht, &intern->pos) == SUCCESS) {
			return; /* ok */
		}
	}
	zend_throw_exception_ex(spl_ce_OutOfBoundsException, 0 TSRMLS_CC, "Seek position %pd is out of range", opos);
} /* }}} */

int static spl_array_object_count_elements_helper(spl_array_object *intern, php_int_t *count TSRMLS_DC) /* {{{ */
{
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	HashPosition pos;

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		*count = 0;
		return FAILURE;
	}

	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		/* We need to store the 'pos' since we'll modify it in the functions
		 * we're going to call and which do not support 'pos' as parameter. */
		pos = intern->pos;
		*count = 0;
		spl_array_rewind(intern TSRMLS_CC);
		while(intern->pos && spl_array_next(intern TSRMLS_CC) == SUCCESS) {
			(*count)++;
		}
		spl_array_set_pos(intern, pos);
		return SUCCESS;
	} else {
		*count = zend_hash_num_elements(aht);
		return SUCCESS;
	}
} /* }}} */

int spl_array_object_count_elements(zval *object, php_int_t *count TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (intern->fptr_count) {
		zval *rv;
		zend_call_method_with_0_params(&object, intern->std.ce, &intern->fptr_count, "count", &rv);
		if (rv) {
			zval_ptr_dtor(&intern->retval);
			MAKE_STD_ZVAL(intern->retval);
			ZVAL_ZVAL(intern->retval, rv, 1, 1);
			convert_to_int(intern->retval);
			*count = (php_int_t) Z_IVAL_P(intern->retval);
			return SUCCESS;
		}
		*count = 0;
		return FAILURE;
	}
	return spl_array_object_count_elements_helper(intern, count TSRMLS_CC);
} /* }}} */

/* {{{ proto int ArrayObject::count()
       proto int ArrayIterator::count()
   Return the number of elements in the Iterator. */
SPL_METHOD(Array, count)
{
	php_int_t count;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	spl_array_object_count_elements_helper(intern, &count TSRMLS_CC);

	RETURN_INT(count);
} /* }}} */

static void spl_array_method(INTERNAL_FUNCTION_PARAMETERS, char *fname, php_size_t fname_len, int use_arg) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	zval *tmp, *arg = NULL;
	zval *retval_ptr = NULL;

	MAKE_STD_ZVAL(tmp);
	Z_TYPE_P(tmp) = IS_ARRAY;
	Z_ARRVAL_P(tmp) = aht;

	if (!use_arg) {
		aht->nApplyCount++;
		zend_call_method(NULL, NULL, NULL, fname, fname_len, &retval_ptr, 1, tmp, NULL TSRMLS_CC);
		aht->nApplyCount--;
	} else if (use_arg == SPL_ARRAY_METHOD_MAY_USER_ARG) {
		if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|z", &arg) == FAILURE) {
			Z_TYPE_P(tmp) = IS_NULL;
			zval_ptr_dtor(&tmp);
			zend_throw_exception(spl_ce_BadMethodCallException, "Function expects one argument at most", 0 TSRMLS_CC);
			return;
		}
		aht->nApplyCount++;
		zend_call_method(NULL, NULL, NULL, fname, fname_len, &retval_ptr, arg? 2 : 1, tmp, arg TSRMLS_CC);
		aht->nApplyCount--;
	} else {
		if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg) == FAILURE) {
			Z_TYPE_P(tmp) = IS_NULL;
			zval_ptr_dtor(&tmp);
			zend_throw_exception(spl_ce_BadMethodCallException, "Function expects exactly one argument", 0 TSRMLS_CC);
			return;
		}
		aht->nApplyCount++;
		zend_call_method(NULL, NULL, NULL, fname, fname_len, &retval_ptr, 2, tmp, arg TSRMLS_CC);
		aht->nApplyCount--;
	}
	Z_TYPE_P(tmp) = IS_NULL; /* we want to destroy the zval, not the hashtable */
	zval_ptr_dtor(&tmp);
	if (retval_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	}
} /* }}} */

#define SPL_ARRAY_METHOD(cname, fname, use_arg) \
SPL_METHOD(cname, fname) \
{ \
	spl_array_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, #fname, sizeof(#fname)-1, use_arg); \
}

/* {{{ proto int ArrayObject::asort([int $sort_flags = SORT_REGULAR ])
       proto int ArrayIterator::asort([int $sort_flags = SORT_REGULAR ])
   Sort the entries by values. */
SPL_ARRAY_METHOD(Array, asort, SPL_ARRAY_METHOD_MAY_USER_ARG) /* }}} */

/* {{{ proto int ArrayObject::ksort([int $sort_flags = SORT_REGULAR ])
       proto int ArrayIterator::ksort([int $sort_flags = SORT_REGULAR ])
   Sort the entries by key. */
SPL_ARRAY_METHOD(Array, ksort, SPL_ARRAY_METHOD_MAY_USER_ARG) /* }}} */

/* {{{ proto int ArrayObject::uasort(callback cmp_function)
       proto int ArrayIterator::uasort(callback cmp_function)
   Sort the entries by values user defined function. */
SPL_ARRAY_METHOD(Array, uasort, SPL_ARRAY_METHOD_USE_ARG) /* }}} */

/* {{{ proto int ArrayObject::uksort(callback cmp_function)
       proto int ArrayIterator::uksort(callback cmp_function)
   Sort the entries by key using user defined function. */
SPL_ARRAY_METHOD(Array, uksort, SPL_ARRAY_METHOD_USE_ARG) /* }}} */

/* {{{ proto int ArrayObject::natsort()
       proto int ArrayIterator::natsort()
   Sort the entries by values using "natural order" algorithm. */
SPL_ARRAY_METHOD(Array, natsort, SPL_ARRAY_METHOD_NO_ARG) /* }}} */

/* {{{ proto int ArrayObject::natcasesort()
       proto int ArrayIterator::natcasesort()
   Sort the entries by key using case insensitive "natural order" algorithm. */
SPL_ARRAY_METHOD(Array, natcasesort, SPL_ARRAY_METHOD_NO_ARG) /* }}} */

/* {{{ proto mixed|NULL ArrayIterator::current()
   Return current array entry */
SPL_METHOD(Array, current)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	zval **entry;
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (spl_array_object_verify_pos(intern, aht TSRMLS_CC) == FAILURE) {
		return;
	}

	if (zend_hash_get_current_data_ex(aht, (void **) &entry, &intern->pos) == FAILURE) {
		return;
	}
	RETVAL_ZVAL(*entry, 1, 0);
}
/* }}} */

/* {{{ proto mixed|NULL ArrayIterator::key()
   Return current array key */
SPL_METHOD(Array, key)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	spl_array_iterator_key(getThis(), return_value TSRMLS_CC);
} /* }}} */

void spl_array_iterator_key(zval *object, zval *return_value TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (spl_array_object_verify_pos(intern, aht TSRMLS_CC) == FAILURE) {
		return;
	}

	zend_hash_get_current_key_zval_ex(aht, return_value, &intern->pos);
}
/* }}} */

/* {{{ proto void ArrayIterator::next()
   Move to next entry */
SPL_METHOD(Array, next)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (spl_array_object_verify_pos(intern, aht TSRMLS_CC) == FAILURE) {
		return;
	}

	spl_array_next_no_verify(intern, aht TSRMLS_CC);
}
/* }}} */

/* {{{ proto bool ArrayIterator::valid()
   Check whether array contains more entries */
SPL_METHOD(Array, valid)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (spl_array_object_verify_pos(intern, aht TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	} else {
		RETURN_BOOL(zend_hash_has_more_elements_ex(aht, &intern->pos) == SUCCESS);
	}
}
/* }}} */

/* {{{ proto bool RecursiveArrayIterator::hasChildren()
   Check whether current element has children (e.g. is an array) */
SPL_METHOD(Array, hasChildren)
{
	zval *object = getThis(), **entry;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (spl_array_object_verify_pos(intern, aht TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}

	if (zend_hash_get_current_data_ex(aht, (void **) &entry, &intern->pos) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_BOOL(Z_TYPE_PP(entry) == IS_ARRAY || (Z_TYPE_PP(entry) == IS_OBJECT && (intern->ar_flags & SPL_ARRAY_CHILD_ARRAYS_ONLY) == 0));
}
/* }}} */

/* {{{ proto object RecursiveArrayIterator::getChildren()
   Create a sub iterator for the current element (same class as $this) */
SPL_METHOD(Array, getChildren)
{
	zval *object = getThis(), **entry, *flags;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (spl_array_object_verify_pos(intern, aht TSRMLS_CC) == FAILURE) {
		return;
	}

	if (zend_hash_get_current_data_ex(aht, (void **) &entry, &intern->pos) == FAILURE) {
		return;
	}

	if (Z_TYPE_PP(entry) == IS_OBJECT) {
		if ((intern->ar_flags & SPL_ARRAY_CHILD_ARRAYS_ONLY) != 0) {
			return;
		}
		if (instanceof_function(Z_OBJCE_PP(entry), Z_OBJCE_P(getThis()) TSRMLS_CC)) {
			RETURN_ZVAL(*entry, 1, 0);
		}
	}

	MAKE_STD_ZVAL(flags);
	ZVAL_INT(flags, SPL_ARRAY_USE_OTHER | intern->ar_flags);
	spl_instantiate_arg_ex2(Z_OBJCE_P(getThis()), &return_value, 0, *entry, flags TSRMLS_CC);
	zval_ptr_dtor(&flags);
}
/* }}} */

/* {{{ proto string ArrayObject::serialize()
   Serialize the object */
SPL_METHOD(Array, serialize)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	zval members, *pmembers;
	php_serialize_data_t var_hash;
	smart_str buf = {0};
	zval *flags;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);

	MAKE_STD_ZVAL(flags);
	ZVAL_INT(flags, (intern->ar_flags & SPL_ARRAY_CLONE_MASK));

	/* storage */
	smart_str_appendl(&buf, "x:", 2);
	php_var_serialize(&buf, &flags, &var_hash TSRMLS_CC);
	zval_ptr_dtor(&flags);

	if (!(intern->ar_flags & SPL_ARRAY_IS_SELF)) {
		php_var_serialize(&buf, &intern->array, &var_hash TSRMLS_CC);
		smart_str_appendc(&buf, ';');
	}

	/* members */
	smart_str_appendl(&buf, "m:", 2);
	INIT_PZVAL(&members);
	if (!intern->std.properties) {
		rebuild_object_properties(&intern->std);
	}
	Z_ARRVAL(members) = intern->std.properties;
	Z_TYPE(members) = IS_ARRAY;
	pmembers = &members;
	php_var_serialize(&buf, &pmembers, &var_hash TSRMLS_CC); /* finishes the string */

	/* done */
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		RETURN_STRINGL(buf.c, buf.len, 0);
	}

	RETURN_NULL();
} /* }}} */

/* {{{ proto void ArrayObject::unserialize(string serialized)
 * unserialize the object
 */
SPL_METHOD(Array, unserialize)
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	char *buf;
	php_size_t buf_len;
	const unsigned char *p, *s;
	php_unserialize_data_t var_hash;
	zval *pmembers, *pflags = NULL;
	php_int_t flags;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &buf, &buf_len) == FAILURE) {
		return;
	}

	if (buf_len == 0) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Empty serialized string cannot be empty");
		return;
	}

	/* storage */
	s = p = (const unsigned char*)buf;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	if (*p!= 'x' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pflags);
	if (!php_var_unserialize(&pflags, &p, s + buf_len, &var_hash TSRMLS_CC) || Z_TYPE_P(pflags) != IS_INT) {
		zval_ptr_dtor(&pflags);
		goto outexcept;
	}

	--p; /* for ';' */
	flags = Z_IVAL_P(pflags);
	zval_ptr_dtor(&pflags);
	/* flags needs to be verified and we also need to verify whether the next
	 * thing we get is ';'. After that we require an 'm' or somethign else
	 * where 'm' stands for members and anything else should be an array. If
	 * neither 'a' or 'm' follows we have an error. */

	if (*p != ';') {
		goto outexcept;
	}
	++p;

	if (*p!='m') {
		if (*p!='a' && *p!='O' && *p!='C' && *p!='r') {
			goto outexcept;
		}
		intern->ar_flags &= ~SPL_ARRAY_CLONE_MASK;
		intern->ar_flags |= flags & SPL_ARRAY_CLONE_MASK;
		zval_ptr_dtor(&intern->array);
		ALLOC_INIT_ZVAL(intern->array);
		if (!php_var_unserialize(&intern->array, &p, s + buf_len, &var_hash TSRMLS_CC)) {
			goto outexcept;
		}
	}
	if (*p != ';') {
		goto outexcept;
	}
	++p;

	/* members */
	if (*p!= 'm' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pmembers);
	if (!php_var_unserialize(&pmembers, &p, s + buf_len, &var_hash TSRMLS_CC)) {
		zval_ptr_dtor(&pmembers);
		goto outexcept;
	}

	/* copy members */
	if (!intern->std.properties) {
		rebuild_object_properties(&intern->std);
	}
	zend_hash_copy(intern->std.properties, Z_ARRVAL_P(pmembers), (copy_ctor_func_t) zval_add_ref, (void *) NULL, sizeof(zval *));
	zval_ptr_dtor(&pmembers);

	/* done reading $serialized */

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return;

outexcept:
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Error at offset %pd of %pu bytes", (php_int_t)((char*)p - buf), buf_len);
	return;

} /* }}} */

/* {{{ arginfo and function tbale */
ZEND_BEGIN_ARG_INFO(arginfo_array___construct, 0)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_offsetGet, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_offsetSet, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, newval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_append, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_seek, 0)
	ZEND_ARG_INFO(0, position)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_exchangeArray, 0)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_setFlags, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_setIteratorClass, 0)
	ZEND_ARG_INFO(0, iteratorClass)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_uXsort, 0)
	ZEND_ARG_INFO(0, cmp_function)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_array_unserialize, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_array_void, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry spl_funcs_ArrayObject[] = {
	SPL_ME(Array, __construct,      arginfo_array___construct,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetExists,     arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetGet,        arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetSet,        arginfo_array_offsetSet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetUnset,      arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, append,           arginfo_array_append,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, getArrayCopy,     arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, count,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, getFlags,         arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, setFlags,         arginfo_array_setFlags,         ZEND_ACC_PUBLIC)
	SPL_ME(Array, asort,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, ksort,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, uasort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, uksort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, natsort,          arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, natcasesort,      arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, unserialize,      arginfo_array_unserialize,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, serialize,        arginfo_array_void,             ZEND_ACC_PUBLIC)
	/* ArrayObject specific */
	SPL_ME(Array, getIterator,      arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, exchangeArray,    arginfo_array_exchangeArray,    ZEND_ACC_PUBLIC)
	SPL_ME(Array, setIteratorClass, arginfo_array_setIteratorClass, ZEND_ACC_PUBLIC)
	SPL_ME(Array, getIteratorClass, arginfo_array_void,             ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static const zend_function_entry spl_funcs_ArrayIterator[] = {
	SPL_ME(Array, __construct,      arginfo_array___construct,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetExists,     arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetGet,        arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetSet,        arginfo_array_offsetSet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetUnset,      arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, append,           arginfo_array_append,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, getArrayCopy,     arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, count,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, getFlags,         arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, setFlags,         arginfo_array_setFlags,         ZEND_ACC_PUBLIC)
	SPL_ME(Array, asort,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, ksort,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, uasort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, uksort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, natsort,          arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, natcasesort,      arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, unserialize,      arginfo_array_unserialize,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, serialize,        arginfo_array_void,             ZEND_ACC_PUBLIC)
	/* ArrayIterator specific */
	SPL_ME(Array, rewind,           arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, current,          arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, key,              arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, next,             arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, valid,            arginfo_array_void,             ZEND_ACC_PUBLIC)
	SPL_ME(Array, seek,             arginfo_array_seek,             ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static const zend_function_entry spl_funcs_RecursiveArrayIterator[] = {
	SPL_ME(Array, hasChildren,   arginfo_array_void, ZEND_ACC_PUBLIC)
	SPL_ME(Array, getChildren,   arginfo_array_void, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION(spl_array) */
PHP_MINIT_FUNCTION(spl_array)
{
	REGISTER_SPL_STD_CLASS_EX(ArrayObject, spl_array_object_new, spl_funcs_ArrayObject);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, Aggregate);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, ArrayAccess);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, Serializable);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, Countable);
	memcpy(&spl_handler_ArrayObject, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	spl_handler_ArrayObject.clone_obj = spl_array_object_clone;
	spl_handler_ArrayObject.read_dimension = spl_array_read_dimension;
	spl_handler_ArrayObject.write_dimension = spl_array_write_dimension;
	spl_handler_ArrayObject.unset_dimension = spl_array_unset_dimension;
	spl_handler_ArrayObject.has_dimension = spl_array_has_dimension;
	spl_handler_ArrayObject.count_elements = spl_array_object_count_elements;

	spl_handler_ArrayObject.get_properties = spl_array_get_properties;
	spl_handler_ArrayObject.get_debug_info = spl_array_get_debug_info;
	spl_handler_ArrayObject.read_property = spl_array_read_property;
	spl_handler_ArrayObject.write_property = spl_array_write_property;
	spl_handler_ArrayObject.get_property_ptr_ptr = spl_array_get_property_ptr_ptr;
	spl_handler_ArrayObject.has_property = spl_array_has_property;
	spl_handler_ArrayObject.unset_property = spl_array_unset_property;

	spl_handler_ArrayObject.compare_objects = spl_array_compare_objects;

	REGISTER_SPL_STD_CLASS_EX(ArrayIterator, spl_array_object_new, spl_funcs_ArrayIterator);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, Iterator);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, ArrayAccess);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, SeekableIterator);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, Serializable);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, Countable);
	memcpy(&spl_handler_ArrayIterator, &spl_handler_ArrayObject, sizeof(zend_object_handlers));
	spl_ce_ArrayIterator->get_iterator = spl_array_get_iterator;

	REGISTER_SPL_SUB_CLASS_EX(RecursiveArrayIterator, ArrayIterator, spl_array_object_new, spl_funcs_RecursiveArrayIterator);
	REGISTER_SPL_IMPLEMENTS(RecursiveArrayIterator, RecursiveIterator);
	spl_ce_RecursiveArrayIterator->get_iterator = spl_array_get_iterator;

	REGISTER_SPL_CLASS_CONST_INT(ArrayObject,   "STD_PROP_LIST",    SPL_ARRAY_STD_PROP_LIST);
	REGISTER_SPL_CLASS_CONST_INT(ArrayObject,   "ARRAY_AS_PROPS",   SPL_ARRAY_ARRAY_AS_PROPS);

	REGISTER_SPL_CLASS_CONST_INT(ArrayIterator, "STD_PROP_LIST",    SPL_ARRAY_STD_PROP_LIST);
	REGISTER_SPL_CLASS_CONST_INT(ArrayIterator, "ARRAY_AS_PROPS",   SPL_ARRAY_ARRAY_AS_PROPS);

	REGISTER_SPL_CLASS_CONST_INT(RecursiveArrayIterator, "CHILD_ARRAYS_ONLY", SPL_ARRAY_CHILD_ARRAYS_ONLY);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */