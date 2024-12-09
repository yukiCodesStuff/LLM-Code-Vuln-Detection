	if (zend_get_property_info(ce, hash_key->key, 1) == NULL) {
		zend_property_info property_info;

		property_info.flags = ZEND_ACC_IMPLICIT_PUBLIC;
		property_info.name = hash_key->key;
		property_info.ce = ce;
		property_info.offset = -1;
		reflection_property_factory(ce, &property_info, &property);
		add_next_index_zval(retval, &property);
	}
	return 0;
}
/* }}} */

/* {{{ proto public ReflectionProperty[] ReflectionClass::getProperties([long $filter])
   Returns an array of this class' properties */
ZEND_METHOD(reflection_class, getProperties)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zend_long filter = 0;
	int argc = ZEND_NUM_ARGS();

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (argc) {
		if (zend_parse_parameters(argc, "|l", &filter) == FAILURE) {
			return;
		}