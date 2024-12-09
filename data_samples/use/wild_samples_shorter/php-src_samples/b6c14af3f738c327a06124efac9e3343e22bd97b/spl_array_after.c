/* {{{ spl_array_object_new_ex */
static zend_object_value spl_array_object_new_ex(zend_class_entry *class_type, spl_array_object **obj, zval *orig, int clone_orig TSRMLS_DC)
{
	zend_object_value retval = {0};
	spl_array_object *intern;
	zval *tmp;
	zend_class_entry * parent = class_type;
	int inherited = 0;