                           char **response, 
                           int   *response_len TSRMLS_DC);

int proxy_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC);
int basic_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC);
void http_context_headers(php_stream_context* context,
                          zend_bool has_authorization,
                          zend_bool has_proxy_authorization,
                          zend_bool has_cookies,
                          smart_str* soap_headers TSRMLS_DC);
#endif