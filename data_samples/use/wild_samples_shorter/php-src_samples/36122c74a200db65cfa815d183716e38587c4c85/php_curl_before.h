struct _php_curl_free {
	zend_llist str;
	zend_llist post;
	zend_llist slist;
};

typedef struct {
	struct _php_curl_error   err;