/* {{{ resourcebundle_array_fetch */
static void resourcebundle_array_fetch(zval *object, zval *offset, zval *return_value, int fallback TSRMLS_DC) 
{
	int32_t     meindex = 0;
	char *      mekey = NULL;
	long        mekeylen;
    zend_bool    is_numeric = 0;
	char         *pbuf;
	ResourceBundle_object *rb;