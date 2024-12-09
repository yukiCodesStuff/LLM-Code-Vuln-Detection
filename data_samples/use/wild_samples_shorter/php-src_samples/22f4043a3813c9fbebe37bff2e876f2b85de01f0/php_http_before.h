                           char **response, 
                           int   *response_len TSRMLS_DC);

void proxy_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC);
void basic_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC);
#endif