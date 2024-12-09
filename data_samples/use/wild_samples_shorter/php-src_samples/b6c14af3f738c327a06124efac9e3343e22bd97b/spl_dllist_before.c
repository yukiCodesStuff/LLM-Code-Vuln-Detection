
static zend_object_value spl_dllist_object_new_ex(zend_class_entry *class_type, spl_dllist_object **obj, zval *orig, int clone_orig TSRMLS_DC) /* {{{ */
{
	zend_object_value  retval;
	spl_dllist_object *intern;
	zend_class_entry  *parent = class_type;
	int                inherited = 0;
