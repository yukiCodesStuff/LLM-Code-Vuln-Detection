	if (!precv || precv->opcode != ZEND_RECV_INIT || precv->op2_type == IS_UNUSED) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Internal error");
		return;
	}

	*return_value = *precv->op2.zv;
	INIT_PZVAL(return_value);
	if ((Z_TYPE_P(return_value) & IS_CONSTANT_TYPE_MASK) != IS_CONSTANT
			&& (Z_TYPE_P(return_value) & IS_CONSTANT_TYPE_MASK) != IS_CONSTANT_ARRAY) {
		zval_copy_ctor(return_value);
	}