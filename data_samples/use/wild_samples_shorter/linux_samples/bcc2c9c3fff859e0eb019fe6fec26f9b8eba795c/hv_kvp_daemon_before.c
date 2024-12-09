	pfd.fd = fd;

	while (1) {
		pfd.events = POLLIN;
		pfd.revents = 0;
		poll(&pfd, 1, -1);

		len = recv(fd, kvp_recv_buffer, sizeof(kvp_recv_buffer), 0);

		if (len < 0) {
			syslog(LOG_ERR, "recv failed; error:%d", len);
			close(fd);
			return -1;
		}
