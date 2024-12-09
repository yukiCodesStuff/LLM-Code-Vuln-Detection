
	*return_value = *precv->op2.zv;
	INIT_PZVAL(return_value);
	if ((Z_TYPE_P(return_value) & IS_CONSTANT_TYPE_MASK) != IS_CONSTANT
			&& (Z_TYPE_P(return_value) & IS_CONSTANT_TYPE_MASK) != IS_CONSTANT_ARRAY) {
		zval_copy_ctor(return_value);
	}
	zval_update_constant_ex(&return_value, (void*)0, param->fptr->common.scope TSRMLS_CC);
}