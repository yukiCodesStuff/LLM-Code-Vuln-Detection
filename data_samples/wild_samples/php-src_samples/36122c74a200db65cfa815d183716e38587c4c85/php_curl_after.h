struct _php_curl_free {
	zend_llist str;
	zend_llist post;
	HashTable *slist;
};

typedef struct {
	struct _php_curl_error   err;
	struct _php_curl_free    *to_free;
	struct _php_curl_send_headers header;
	void ***thread_ctx;
	CURL                    *cp;
	php_curl_handlers       *handlers;
	long                     id;
	zend_bool                in_callback;
	zval                     *clone;
	zend_bool                safe_upload;
} php_curl;

#define CURLOPT_SAFE_UPLOAD -1

typedef struct {
	int    still_running;
	CURLM *multi;
	zend_llist easyh;
} php_curlm;

typedef struct {
	CURLSH                   *share;
} php_curlsh;

void _php_curl_cleanup_handle(php_curl *);
void _php_curl_multi_cleanup_list(void *data);
int  _php_curl_verify_handlers(php_curl *ch, int reporterror TSRMLS_DC);

void curlfile_register_class(TSRMLS_D);
PHP_CURL_API extern zend_class_entry *curl_CURLFile_class;

#else
#define curl_module_ptr NULL
#endif /* HAVE_CURL */
#define phpext_curl_ptr curl_module_ptr
#endif  /* _PHP_CURL_H */