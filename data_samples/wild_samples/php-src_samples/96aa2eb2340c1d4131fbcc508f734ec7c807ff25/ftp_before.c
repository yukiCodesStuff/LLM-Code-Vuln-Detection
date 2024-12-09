{
#if HAVE_OPENSSL_EXT
	SSL_CTX	*ctx = NULL;
#endif
	if (ftp == NULL) {
		return 0;
	}
		if (ctx == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create the SSL context");
			return 0;
		}

		SSL_CTX_set_options(ctx, SSL_OP_ALL);

		ftp->ssl_handle = SSL_new(ctx);
		if (ftp->ssl_handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create the SSL handle");
			SSL_CTX_free(ctx);
			return 0;
		}

		SSL_set_fd(ftp->ssl_handle, ftp->fd);

		if (SSL_connect(ftp->ssl_handle) <= 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "SSL/TLS handshake failed");
			SSL_shutdown(ftp->ssl_handle);
			return 0;
		}

		ftp->ssl_active = 1;

		if (!ftp->old_ssl) {

			/* set protection buffersize to zero */
			if (!ftp_putcmd(ftp, "PBSZ", "0")) {
				return 0;
			}
{
	php_sockaddr_storage addr;
	socklen_t			size;

#if HAVE_OPENSSL_EXT
	SSL_CTX		*ctx;
#endif

	if (data->fd != -1) {
		goto data_accepted;
	}
		if (ctx == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_accept: failed to create the SSL context");
			return 0;
		}

		SSL_CTX_set_options(ctx, SSL_OP_ALL);

		data->ssl_handle = SSL_new(ctx);
		if (data->ssl_handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_accept: failed to create the SSL handle");
			SSL_CTX_free(ctx);
			return 0;
		}
			
		
		SSL_set_fd(data->ssl_handle, data->fd);

		if (ftp->old_ssl) {
			SSL_copy_session_id(data->ssl_handle, ftp->ssl_handle);
		}
			
		if (SSL_connect(data->ssl_handle) <= 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_accept: SSL/TLS handshake failed");
			SSL_shutdown(data->ssl_handle);
			return 0;
		}
			
		data->ssl_active = 1;
	}	

#endif

	return data;
}
/* }}} */

/* {{{ data_close
 */
databuf_t*
data_close(ftpbuf_t *ftp, databuf_t *data)
{
	if (data == NULL) {
		return NULL;
	}
	if (data->listener != -1) {
#if HAVE_OPENSSL_EXT
		if (data->ssl_active) {
			SSL_shutdown(data->ssl_handle);
			data->ssl_active = 0;
		}
#endif				
		closesocket(data->listener);
	}	
	if (data->fd != -1) {
#if HAVE_OPENSSL_EXT
		if (data->ssl_active) {
			SSL_shutdown(data->ssl_handle);
			data->ssl_active = 0;
		}
#endif				
		closesocket(data->fd);
	}	
	if (ftp) {
		ftp->data = NULL;
	}
	efree(data);
	return NULL;
}
/* }}} */

/* {{{ ftp_genlist
 */
char**
ftp_genlist(ftpbuf_t *ftp, const char *cmd, const char *path TSRMLS_DC)
{
	php_stream	*tmpstream = NULL;
	databuf_t	*data = NULL;
	char		*ptr;
	int		ch, lastch;
	int		size, rcvd;
	int		lines;
	char		**ret = NULL;
	char		**entry;
	char		*text;


	if ((tmpstream = php_stream_fopen_tmpfile()) == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to create temporary file.  Check permissions in temporary files directory.");
		return NULL;
	}

	if (!ftp_type(ftp, FTPTYPE_ASCII)) {
		goto bail;
	}

	if ((data = ftp_getdata(ftp TSRMLS_CC)) == NULL) {
		goto bail;
	}
	ftp->data = data;	

	if (!ftp_putcmd(ftp, cmd, path)) {
		goto bail;
	}
	if (!ftp_getresp(ftp) || (ftp->resp != 150 && ftp->resp != 125 && ftp->resp != 226)) {
		goto bail;
	}

	/* some servers don't open a ftp-data connection if the directory is empty */
	if (ftp->resp == 226) {
		ftp->data = data_close(ftp, data);
		php_stream_close(tmpstream);
		return ecalloc(1, sizeof(char**));
	}

	/* pull data buffer into tmpfile */
	if ((data = data_accept(data, ftp TSRMLS_CC)) == NULL) {
		goto bail;
	}
	size = 0;
	lines = 0;
	lastch = 0;
	while ((rcvd = my_recv(ftp, data->fd, data->buf, FTP_BUFSIZE))) {
		if (rcvd == -1) {
			goto bail;
		}

		php_stream_write(tmpstream, data->buf, rcvd);

		size += rcvd;
		for (ptr = data->buf; rcvd; rcvd--, ptr++) {
			if (*ptr == '\n' && lastch == '\r') {
				lines++;
			} else {
				size++;
			}
			lastch = *ptr;
		}