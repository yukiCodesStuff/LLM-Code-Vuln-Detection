/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Brad Lafountain <rodif_bl@yahoo.com>                        |
  |          Shane Caraveo <shane@caraveo.com>                           |
  |          Dmitry Stogov <dmitry@zend.com>                             |
  +----------------------------------------------------------------------+
*/
/* $Id$ */

#include "php_soap.h"
#include "ext/standard/base64.h"
#include "ext/standard/md5.h"
#include "ext/standard/php_rand.h"

static char *get_http_header_value(char *headers, char *type);
static int get_http_body(php_stream *socketd, int close, char *headers,  char **response, int *out_size TSRMLS_DC);
static int get_http_headers(php_stream *socketd,char **response, int *out_size TSRMLS_DC);

#define smart_str_append_const(str, const) \
	smart_str_appendl(str,const,sizeof(const)-1)

/* Proxy HTTP Authentication */
void proxy_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC)
{
	zval **login, **password;

	if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_login", sizeof("_proxy_login"), (void **)&login) == SUCCESS) {
		unsigned char* buf;
		int len;
		smart_str auth = {0};

		smart_str_appendl(&auth, Z_STRVAL_PP(login), Z_STRLEN_PP(login));
		smart_str_appendc(&auth, ':');
		if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_password", sizeof("_proxy_password"), (void **)&password) == SUCCESS) {
			smart_str_appendl(&auth, Z_STRVAL_PP(password), Z_STRLEN_PP(password));
		}
		smart_str_0(&auth);
		buf = php_base64_encode((unsigned char*)auth.c, auth.len, &len);
		smart_str_append_const(soap_headers, "Proxy-Authorization: Basic ");
		smart_str_appendl(soap_headers, (char*)buf, len);
		smart_str_append_const(soap_headers, "\r\n");
		efree(buf);
		smart_str_free(&auth);
	}
}
		if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_password", sizeof("_proxy_password"), (void **)&password) == SUCCESS) {
			smart_str_appendl(&auth, Z_STRVAL_PP(password), Z_STRLEN_PP(password));
		}
		smart_str_0(&auth);
		buf = php_base64_encode((unsigned char*)auth.c, auth.len, &len);
		smart_str_append_const(soap_headers, "Proxy-Authorization: Basic ");
		smart_str_appendl(soap_headers, (char*)buf, len);
		smart_str_append_const(soap_headers, "\r\n");
		efree(buf);
		smart_str_free(&auth);
	}
}

/* HTTP Authentication */
void basic_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC)
{
	zval **login, **password;

	if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_login", sizeof("_login"), (void **)&login) == SUCCESS &&
			!zend_hash_exists(Z_OBJPROP_P(this_ptr), "_digest", sizeof("_digest"))) {
		unsigned char* buf;
		int len;
		smart_str auth = {0};

		smart_str_appendl(&auth, Z_STRVAL_PP(login), Z_STRLEN_PP(login));
		smart_str_appendc(&auth, ':');
		if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_password", sizeof("_password"), (void **)&password) == SUCCESS) {
			smart_str_appendl(&auth, Z_STRVAL_PP(password), Z_STRLEN_PP(password));
		}
		if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_password", sizeof("_password"), (void **)&password) == SUCCESS) {
			smart_str_appendl(&auth, Z_STRVAL_PP(password), Z_STRLEN_PP(password));
		}
		smart_str_0(&auth);
		buf = php_base64_encode((unsigned char*)auth.c, auth.len, &len);
		smart_str_append_const(soap_headers, "Authorization: Basic ");
		smart_str_appendl(soap_headers, (char*)buf, len);
		smart_str_append_const(soap_headers, "\r\n");
		efree(buf);
		smart_str_free(&auth);
	}
}

static php_stream* http_connect(zval* this_ptr, php_url *phpurl, int use_ssl, php_stream_context *context, int *use_proxy TSRMLS_DC)
{
	php_stream *stream;
	zval **proxy_host, **proxy_port, **tmp;
	char *host;
	char *name;
	long namelen;
	int port;
	int old_error_reporting;
	struct timeval tv;
	struct timeval *timeout = NULL;

	if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_host", sizeof("_proxy_host"), (void **) &proxy_host) == SUCCESS &&
	    Z_TYPE_PP(proxy_host) == IS_STRING &&
	    zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_port", sizeof("_proxy_port"), (void **) &proxy_port) == SUCCESS &&
	    Z_TYPE_PP(proxy_port) == IS_LONG) {
		host = Z_STRVAL_PP(proxy_host);
		port = Z_LVAL_PP(proxy_port);
		*use_proxy = 1;
	} else {
		host = phpurl->host;
		port = phpurl->port;
	}
	    } else {
	      n = 3;
				ZVAL_STRING(&func, "gzencode", 0);
				smart_str_append_const(&soap_headers_z,"Content-Encoding: gzip\r\n");
				ZVAL_LONG(params[2], 1);
	    }
			if (call_user_function(CG(function_table), (zval**)NULL, &func, &retval, n, params TSRMLS_CC) == SUCCESS &&
			    Z_TYPE(retval) == IS_STRING) {
				request = Z_STRVAL(retval);
				request_size = Z_STRLEN(retval);
			} else {
				if (request != buf) {efree(request);}
				    Z_TYPE_PP(password) == IS_STRING) {
					smart_str_appendl(&auth, Z_STRVAL_PP(password), Z_STRLEN_PP(password));
				}
				smart_str_0(&auth);
				buf = php_base64_encode((unsigned char*)auth.c, auth.len, &len);
				smart_str_append_const(&soap_headers, "Authorization: Basic ");
				smart_str_appendl(&soap_headers, (char*)buf, len);
				smart_str_append_const(&soap_headers, "\r\n");
				efree(buf);
				smart_str_free(&auth);
			}
		}

		/* Proxy HTTP Authentication */
		if (use_proxy && !use_ssl) {
			has_proxy_authorization = 1;
			proxy_authentication(this_ptr, &soap_headers TSRMLS_CC);
		}
						      (use_ssl || zend_hash_index_find(Z_ARRVAL_PP(data), 3, (void**)&tmp) == FAILURE)) {
								smart_str_appendl(&soap_headers, key, strlen(key));
								smart_str_appendc(&soap_headers, '=');
								smart_str_appendl(&soap_headers, Z_STRVAL_PP(value), Z_STRLEN_PP(value));
								smart_str_appendc(&soap_headers, ';');
							}
						}
					}
					zend_hash_move_forward(Z_ARRVAL_PP(cookies));
				}
				smart_str_append_const(&soap_headers, "\r\n");
			}
		}

		if (context &&
			php_stream_context_get_option(context, "http", "header", &tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_STRING && Z_STRLEN_PP(tmp)) {
			char *s = Z_STRVAL_PP(tmp);
			char *p;
			int name_len;

			while (*s) {
				/* skip leading newlines and spaces */
				while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
					s++;
				}
				/* extract header name */
				p = s;
				name_len = -1;
				while (*p) {
					if (*p == ':') {
						if (name_len < 0) name_len = p - s;
						break;
					} else if (*p == ' ' || *p == '\t') {
						if (name_len < 0) name_len = p - s;
					} else if (*p == '\r' || *p == '\n') {
						break;
					}
					p++;
				}
				if (*p == ':') {
					/* extract header value */
					while (*p && *p != '\r' && *p != '\n') {
						p++;
					}
					/* skip some predefined headers */
					if ((name_len != sizeof("host")-1 ||
					     strncasecmp(s, "host", sizeof("host")-1) != 0) &&
					    (name_len != sizeof("connection")-1 ||
					     strncasecmp(s, "connection", sizeof("connection")-1) != 0) &&
					    (name_len != sizeof("user-agent")-1 ||
					     strncasecmp(s, "user-agent", sizeof("user-agent")-1) != 0) &&
					    (name_len != sizeof("content-length")-1 ||
					     strncasecmp(s, "content-length", sizeof("content-length")-1) != 0) &&
					    (name_len != sizeof("content-type")-1 ||
					     strncasecmp(s, "content-type", sizeof("content-type")-1) != 0) &&
					    (!has_cookies ||
					     name_len != sizeof("cookie")-1 ||
					     strncasecmp(s, "cookie", sizeof("cookie")-1) != 0) &&
					    (!has_authorization ||
					     name_len != sizeof("authorization")-1 ||
					     strncasecmp(s, "authorization", sizeof("authorization")-1) != 0) &&
					    (!has_proxy_authorization ||
					     name_len != sizeof("proxy-authorization")-1 ||
					     strncasecmp(s, "proxy-authorization", sizeof("proxy-authorization")-1) != 0)) {
					    /* add header */
						smart_str_appendl(&soap_headers, s, p-s);
						smart_str_append_const(&soap_headers, "\r\n");
					}
				}
				s = (*p) ? (p + 1) : p;
			}
		}

		smart_str_append_const(&soap_headers, "\r\n");
		smart_str_0(&soap_headers);
		if (zend_hash_find(Z_OBJPROP_P(this_ptr), "trace", sizeof("trace"), (void **) &trace) == SUCCESS &&
		    Z_LVAL_PP(trace) > 0) {
			add_property_stringl(this_ptr, "__last_request_headers", soap_headers.c, soap_headers.len, 1);
		}
		smart_str_appendl(&soap_headers, request, request_size);
		smart_str_0(&soap_headers);

		err = php_stream_write(stream, soap_headers.c, soap_headers.len);
		if (err != soap_headers.len) {
			if (request != buf) {efree(request);}
			php_stream_close(stream);
			zend_hash_del(Z_OBJPROP_P(this_ptr), "httpurl", sizeof("httpurl"));
			zend_hash_del(Z_OBJPROP_P(this_ptr), "httpsocket", sizeof("httpsocket"));
			zend_hash_del(Z_OBJPROP_P(this_ptr), "_use_proxy", sizeof("_use_proxy"));
			add_soap_fault(this_ptr, "HTTP", "Failed Sending HTTP SOAP request", NULL, NULL TSRMLS_CC);
			smart_str_free(&soap_headers_z);
			return FALSE;
		}