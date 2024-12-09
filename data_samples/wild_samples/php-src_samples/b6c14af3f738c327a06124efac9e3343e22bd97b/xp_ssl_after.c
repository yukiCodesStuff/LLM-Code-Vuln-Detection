		do {
			struct timeval	cur_time,
							elapsed_time = {0};
			
			if (sslsock->is_client) {
				n = SSL_connect(sslsock->ssl_handle);
			} else {
				n = SSL_accept(sslsock->ssl_handle);
			}

			if (has_timeout) {
				gettimeofday(&cur_time, NULL);
				elapsed_time.tv_sec  = cur_time.tv_sec  - start_time.tv_sec;
				elapsed_time.tv_usec = cur_time.tv_usec - start_time.tv_usec;
				if (cur_time.tv_usec < start_time.tv_usec) {
					elapsed_time.tv_sec  -= 1L;
					elapsed_time.tv_usec += 1000000L;
				}
			
				if (elapsed_time.tv_sec > timeout->tv_sec ||
						(elapsed_time.tv_sec == timeout->tv_sec &&
						elapsed_time.tv_usec > timeout->tv_usec)) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "SSL: crypto enabling timeout");
					return -1;
				}
			}

			if (n <= 0) {
				/* in case of SSL_ERROR_WANT_READ/WRITE, do not retry in non-blocking mode */
				retry = handle_ssl_error(stream, n, blocked TSRMLS_CC);
				if (retry) {
					/* wait until something interesting happens in the socket. It may be a
					 * timeout. Also consider the unlikely of possibility of a write block  */
					int err = SSL_get_error(sslsock->ssl_handle, n);
					struct timeval left_time;
					
					if (has_timeout) {
						left_time.tv_sec  = timeout->tv_sec  - elapsed_time.tv_sec;
						left_time.tv_usec =	timeout->tv_usec - elapsed_time.tv_usec;
						if (timeout->tv_usec < elapsed_time.tv_usec) {
							left_time.tv_sec  -= 1L;
							left_time.tv_usec += 1000000L;
						}
					}
					php_pollfd_for(sslsock->s.socket, (err == SSL_ERROR_WANT_READ) ?
						(POLLIN|POLLPRI) : POLLOUT, has_timeout ? &left_time : NULL);
				}
			} else {
				retry = 0;
			}
		} while (retry);

		if (sslsock->s.is_blocked != blocked && SUCCESS == php_set_sock_blocking(sslsock->s.socket, blocked TSRMLS_CC)) {
			sslsock->s.is_blocked = blocked;
		}

		if (n == 1) {
			X509 *peer_cert;

			peer_cert = SSL_get_peer_certificate(sslsock->ssl_handle);

			if (FAILURE == php_openssl_apply_verification_policy(sslsock->ssl_handle, peer_cert, stream TSRMLS_CC)) {
				SSL_shutdown(sslsock->ssl_handle);
				n = -1;
			} else {	
				sslsock->ssl_active = 1;

				/* allow the script to capture the peer cert
				 * and/or the certificate chain */
				if (stream->context) {
					zval **val, *zcert;

					if (SUCCESS == php_stream_context_get_option(
								stream->context, "ssl",
								"capture_peer_cert", &val) &&
							zval_is_true(*val)) {
						MAKE_STD_ZVAL(zcert);
						ZVAL_RESOURCE(zcert, zend_list_insert(peer_cert, 
									php_openssl_get_x509_list_id() TSRMLS_CC));
						php_stream_context_set_option(stream->context,
								"ssl", "peer_certificate",
								zcert);
						peer_cert = NULL;
						FREE_ZVAL(zcert);
					}

					if (SUCCESS == php_stream_context_get_option(
								stream->context, "ssl",
								"capture_peer_cert_chain", &val) &&
							zval_is_true(*val)) {
						zval *arr;
						STACK_OF(X509) *chain;

						MAKE_STD_ZVAL(arr);
						chain = SSL_get_peer_cert_chain(
									sslsock->ssl_handle);

						if (chain && sk_X509_num(chain) > 0) {
							int i;
							array_init(arr);

							for (i = 0; i < sk_X509_num(chain); i++) {
								X509 *mycert = X509_dup(
										sk_X509_value(chain, i));
								MAKE_STD_ZVAL(zcert);
								ZVAL_RESOURCE(zcert,
										zend_list_insert(mycert,
											php_openssl_get_x509_list_id() TSRMLS_CC));
								add_next_index_zval(arr, zcert);
							}

						} else {
							ZVAL_NULL(arr);
						}

						php_stream_context_set_option(stream->context,
								"ssl", "peer_certificate_chain",
								arr);
						zval_dtor(arr);
						efree(arr);
					}
				}
			}

			if (peer_cert) {
				X509_free(peer_cert);
			}
		} else  {
			n = errno == EAGAIN ? 0 : -1;
		}

		return n;

	} else if (!cparam->inputs.activate && sslsock->ssl_active) {
		/* deactivate - common for server/client */
		SSL_shutdown(sslsock->ssl_handle);
		sslsock->ssl_active = 0;
	}
	return -1;
}

static inline int php_openssl_tcp_sockop_accept(php_stream *stream, php_openssl_netstream_data_t *sock,
		php_stream_xport_param *xparam STREAMS_DC TSRMLS_DC)
{
	int clisock;

	xparam->outputs.client = NULL;

	clisock = php_network_accept_incoming(sock->s.socket,
			xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
			xparam->want_textaddr ? &xparam->outputs.textaddrlen : NULL,
			xparam->want_addr ? &xparam->outputs.addr : NULL,
			xparam->want_addr ? &xparam->outputs.addrlen : NULL,
			xparam->inputs.timeout,
			xparam->want_errortext ? &xparam->outputs.error_text : NULL,
			&xparam->outputs.error_code
			TSRMLS_CC);

	if (clisock >= 0) {
		php_openssl_netstream_data_t *clisockdata;

		clisockdata = emalloc(sizeof(*clisockdata));

		if (clisockdata == NULL) {
			closesocket(clisock);
			/* technically a fatal error */
		} else {
			/* copy underlying tcp fields */
			memset(clisockdata, 0, sizeof(*clisockdata));
			memcpy(clisockdata, sock, sizeof(clisockdata->s));

			clisockdata->s.socket = clisock;
			
			xparam->outputs.client = php_stream_alloc_rel(stream->ops, clisockdata, NULL, "r+");
			if (xparam->outputs.client) {
				xparam->outputs.client->context = stream->context;
				if (stream->context) {
					zend_list_addref(stream->context->rsrc_id);
				}
			}
		}

		if (xparam->outputs.client && sock->enable_on_connect) {
			/* apply crypto */
			switch (sock->method) {
				case STREAM_CRYPTO_METHOD_SSLv23_CLIENT:
					sock->method = STREAM_CRYPTO_METHOD_SSLv23_SERVER;
					break;
				case STREAM_CRYPTO_METHOD_SSLv2_CLIENT:
					sock->method = STREAM_CRYPTO_METHOD_SSLv2_SERVER;
					break;
				case STREAM_CRYPTO_METHOD_SSLv3_CLIENT:
					sock->method = STREAM_CRYPTO_METHOD_SSLv3_SERVER;
					break;
				case STREAM_CRYPTO_METHOD_TLS_CLIENT:
					sock->method = STREAM_CRYPTO_METHOD_TLS_SERVER;
					break;
				default:
					break;
			}

			clisockdata->method = sock->method;

			if (php_stream_xport_crypto_setup(xparam->outputs.client, clisockdata->method,
					NULL TSRMLS_CC) < 0 || php_stream_xport_crypto_enable(
					xparam->outputs.client, 1 TSRMLS_CC) < 0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to enable crypto");

				php_stream_close(xparam->outputs.client);
				xparam->outputs.client = NULL;
				xparam->outputs.returncode = -1;
			}
		}
	}
	
	return xparam->outputs.client == NULL ? -1 : 0;
}
static int php_openssl_sockop_set_option(php_stream *stream, int option, int value, void *ptrparam TSRMLS_DC)
{
	php_openssl_netstream_data_t *sslsock = (php_openssl_netstream_data_t*)stream->abstract;
	php_stream_xport_crypto_param *cparam = (php_stream_xport_crypto_param *)ptrparam;
	php_stream_xport_param *xparam = (php_stream_xport_param *)ptrparam;

	switch (option) {
		case PHP_STREAM_OPTION_CHECK_LIVENESS:
			{
				struct timeval tv;
				char buf;
				int alive = 1;

				if (value == -1) {
					if (sslsock->s.timeout.tv_sec == -1) {
						tv.tv_sec = FG(default_socket_timeout);
						tv.tv_usec = 0;
					} else {
						tv = sslsock->connect_timeout;
					}
				} else {
					tv.tv_sec = value;
					tv.tv_usec = 0;
				}

				if (sslsock->s.socket == -1) {
					alive = 0;
				} else if (php_pollfd_for(sslsock->s.socket, PHP_POLLREADABLE|POLLPRI, &tv) > 0) {
					if (sslsock->ssl_active) {
						int n;

						do {
							n = SSL_peek(sslsock->ssl_handle, &buf, sizeof(buf));
							if (n <= 0) {
								int err = SSL_get_error(sslsock->ssl_handle, n);

								if (err == SSL_ERROR_SYSCALL) {
									alive = php_socket_errno() == EAGAIN;
									break;
								}

								if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
									/* re-negotiate */
									continue;
								}

								/* any other problem is a fatal error */
								alive = 0;
							}
							/* either peek succeeded or there was an error; we
							 * have set the alive flag appropriately */
							break;
						} while (1);
					} else if (0 == recv(sslsock->s.socket, &buf, sizeof(buf), MSG_PEEK) && php_socket_errno() != EAGAIN) {
						alive = 0;
					}
				}
				return alive ? PHP_STREAM_OPTION_RETURN_OK : PHP_STREAM_OPTION_RETURN_ERR;
			}
			
		case PHP_STREAM_OPTION_CRYPTO_API:

			switch(cparam->op) {

				case STREAM_XPORT_CRYPTO_OP_SETUP:
					cparam->outputs.returncode = php_openssl_setup_crypto(stream, sslsock, cparam TSRMLS_CC);
					return PHP_STREAM_OPTION_RETURN_OK;
					break;
				case STREAM_XPORT_CRYPTO_OP_ENABLE:
					cparam->outputs.returncode = php_openssl_enable_crypto(stream, sslsock, cparam TSRMLS_CC);
					return PHP_STREAM_OPTION_RETURN_OK;
					break;
				default:
					/* fall through */
					break;
			}

			break;

		case PHP_STREAM_OPTION_XPORT_API:
			switch(xparam->op) {

				case STREAM_XPORT_OP_CONNECT:
				case STREAM_XPORT_OP_CONNECT_ASYNC:
					/* TODO: Async connects need to check the enable_on_connect option when
					 * we notice that the connect has actually been established */
					php_stream_socket_ops.set_option(stream, option, value, ptrparam TSRMLS_CC);

					if ((sslsock->enable_on_connect) &&
						((xparam->outputs.returncode == 0) ||
						(xparam->op == STREAM_XPORT_OP_CONNECT_ASYNC && 
						xparam->outputs.returncode == 1 && xparam->outputs.error_code == EINPROGRESS)))
					{
						if (php_stream_xport_crypto_setup(stream, sslsock->method, NULL TSRMLS_CC) < 0 ||
								php_stream_xport_crypto_enable(stream, 1 TSRMLS_CC) < 0) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to enable crypto");
							xparam->outputs.returncode = -1;
						}
					}
					return PHP_STREAM_OPTION_RETURN_OK;

				case STREAM_XPORT_OP_ACCEPT:
					/* we need to copy the additional fields that the underlying tcp transport
					 * doesn't know about */
					xparam->outputs.returncode = php_openssl_tcp_sockop_accept(stream, sslsock, xparam STREAMS_CC TSRMLS_CC);

					
					return PHP_STREAM_OPTION_RETURN_OK;

				default:
					/* fall through */
					break;
			}
	}

	return php_stream_socket_ops.set_option(stream, option, value, ptrparam TSRMLS_CC);
}

static int php_openssl_sockop_cast(php_stream *stream, int castas, void **ret TSRMLS_DC)
{
	php_openssl_netstream_data_t *sslsock = (php_openssl_netstream_data_t*)stream->abstract;

	switch(castas)	{
		case PHP_STREAM_AS_STDIO:
			if (sslsock->ssl_active) {
				return FAILURE;
			}
			if (ret)	{
				*ret = fdopen(sslsock->s.socket, stream->mode);
				if (*ret) {
					return SUCCESS;
				}
				return FAILURE;
			}
			return SUCCESS;

		case PHP_STREAM_AS_FD_FOR_SELECT:
			if (ret) {
				*(int *)ret = sslsock->s.socket;
			}
			return SUCCESS;

		case PHP_STREAM_AS_FD:
		case PHP_STREAM_AS_SOCKETD:
			if (sslsock->ssl_active) {
				return FAILURE;
			}
			if (ret) {
				*(int *)ret = sslsock->s.socket;
			}
			return SUCCESS;
		default:
			return FAILURE;
	}
}

php_stream_ops php_openssl_socket_ops = {
	php_openssl_sockop_write, php_openssl_sockop_read,
	php_openssl_sockop_close, php_openssl_sockop_flush,
	"tcp_socket/ssl",
	NULL, /* seek */
	php_openssl_sockop_cast,
	php_openssl_sockop_stat,
	php_openssl_sockop_set_option,
};

static char * get_sni(php_stream_context *ctx, const char *resourcename, size_t resourcenamelen, int is_persistent TSRMLS_DC) {

	php_url *url;

	if (ctx) {
		zval **val = NULL;

		if (php_stream_context_get_option(ctx, "ssl", "SNI_enabled", &val) == SUCCESS && !zend_is_true(*val)) {
			return NULL;
		}
		if (php_stream_context_get_option(ctx, "ssl", "SNI_server_name", &val) == SUCCESS) {
			convert_to_string_ex(val);
			return pestrdup(Z_STRVAL_PP(val), is_persistent);
		}
	}

	if (!resourcename) {
		return NULL;
	}

	url = php_url_parse_ex(resourcename, resourcenamelen);
	if (!url) {
		return NULL;
	}

	if (url->host) {
		const char * host = url->host;
		char * sni = NULL;
		size_t len = strlen(host);

		/* skip trailing dots */
		while (len && host[len-1] == '.') {
			--len;
		}

		if (len) {
			sni = pestrndup(host, len, is_persistent);
		}

		php_url_free(url);
		return sni;
	}

	php_url_free(url);
	return NULL;
}

php_stream *php_openssl_ssl_socket_factory(const char *proto, size_t protolen,
		const char *resourcename, size_t resourcenamelen,
		const char *persistent_id, int options, int flags,
		struct timeval *timeout,
		php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	php_stream *stream = NULL;
	php_openssl_netstream_data_t *sslsock = NULL;
	
	sslsock = pemalloc(sizeof(php_openssl_netstream_data_t), persistent_id ? 1 : 0);
	memset(sslsock, 0, sizeof(*sslsock));

	sslsock->s.is_blocked = 1;
	/* this timeout is used by standard stream funcs, therefor it should use the default value */
	sslsock->s.timeout.tv_sec = FG(default_socket_timeout);
	sslsock->s.timeout.tv_usec = 0;

	/* use separate timeout for our private funcs */
	sslsock->connect_timeout.tv_sec = timeout->tv_sec;
	sslsock->connect_timeout.tv_usec = timeout->tv_usec;

	/* we don't know the socket until we have determined if we are binding or
	 * connecting */
	sslsock->s.socket = -1;
	
	/* Initialize context as NULL */
	sslsock->ctx = NULL;	
	
	stream = php_stream_alloc_rel(&php_openssl_socket_ops, sslsock, persistent_id, "r+");

	if (stream == NULL)	{
		pefree(sslsock, persistent_id ? 1 : 0);
		return NULL;
	}

	sslsock->sni = get_sni(context, resourcename, resourcenamelen, !!persistent_id TSRMLS_CC);
	
	if (strncmp(proto, "ssl", protolen) == 0) {
		sslsock->enable_on_connect = 1;
		sslsock->method = STREAM_CRYPTO_METHOD_SSLv23_CLIENT;
	} else if (strncmp(proto, "sslv2", protolen) == 0) {
#ifdef OPENSSL_NO_SSL2
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "SSLv2 support is not compiled into the OpenSSL library PHP is linked against");
		return NULL;
#else
		sslsock->enable_on_connect = 1;
		sslsock->method = STREAM_CRYPTO_METHOD_SSLv2_CLIENT;
#endif
	} else if (strncmp(proto, "sslv3", protolen) == 0) {
		sslsock->enable_on_connect = 1;
		sslsock->method = STREAM_CRYPTO_METHOD_SSLv3_CLIENT;
	} else if (strncmp(proto, "tls", protolen) == 0) {
		sslsock->enable_on_connect = 1;
		sslsock->method = STREAM_CRYPTO_METHOD_TLS_CLIENT;
	}

	return stream;
}



/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */