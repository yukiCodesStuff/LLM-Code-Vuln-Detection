		
		do {
			struct timeval	cur_time,
							elapsed_time;
			
			if (sslsock->is_client) {
				n = SSL_connect(sslsock->ssl_handle);
			} else {