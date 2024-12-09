ftp_get(ftpbuf_t *ftp, php_stream *outstream, const char *path, ftptype_t type, long resumepos TSRMLS_DC)
{
	databuf_t		*data = NULL;
	size_t			rcvd;
	char			arg[11];

	if (ftp == NULL) {
		goto bail;
	}

	while ((rcvd = my_recv(ftp, data->fd, data->buf, FTP_BUFSIZE))) {
		if (rcvd == -1) {
			goto bail;
		}
int
ftp_getresp(ftpbuf_t *ftp)
{
	if (ftp == NULL) {
		return 0;
	}
	ftp->resp = 0;

	while (1) {
