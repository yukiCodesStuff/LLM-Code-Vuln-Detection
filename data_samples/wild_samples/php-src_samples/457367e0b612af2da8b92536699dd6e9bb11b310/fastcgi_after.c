			if (sa.sa_inet.sin_addr.s_addr == INADDR_NONE) {
				struct hostent *hep;

				if(strlen(host) > MAXHOSTNAMELEN) {
					hep = NULL;
				} else {
					hep = gethostbyname(host);
				}
				if (!hep || hep->h_addrtype != AF_INET || !hep->h_addr_list[0]) {
					fprintf(stderr, "Cannot resolve host name '%s'!\n", host);
					return -1;
				} else if (hep->h_addr_list[1]) {
					fprintf(stderr, "Host '%s' has multiple addresses. You must choose one explicitly!\n", host);
					return -1;
				}
				sa.sa_inet.sin_addr.s_addr = ((struct in_addr*)hep->h_addr_list[0])->s_addr;
			}
		}
		if (!req->tcp) {
			unsigned int in_len = tmp > UINT_MAX ? UINT_MAX : (unsigned int)tmp;

			ret = read(req->fd, ((char*)buf)+n, in_len);
		} else {
			int in_len = tmp > INT_MAX ? INT_MAX : (int)tmp;

			ret = recv(req->fd, ((char*)buf)+n, in_len, 0);
			if (ret <= 0) {
				errno = WSAGetLastError();
			}
		}
#else
		ret = read(req->fd, ((char*)buf)+n, count-n);
#endif
		if (ret > 0) {
			n += ret;
		} else if (ret == 0 && errno == 0) {
			return n;
		} else if (ret <= 0 && errno != 0 && errno != EINTR) {
			return ret;
		}
	} while (n != count);
	return n;
}

static inline int fcgi_make_header(fcgi_header *hdr, fcgi_request_type type, int req_id, int len)
{
	int pad = ((len + 7) & ~7) - len;

	hdr->contentLengthB0 = (unsigned char)(len & 0xff);
	hdr->contentLengthB1 = (unsigned char)((len >> 8) & 0xff);
	hdr->paddingLength = (unsigned char)pad;
	hdr->requestIdB0 = (unsigned char)(req_id & 0xff);
	hdr->requestIdB1 = (unsigned char)((req_id >> 8) & 0xff);
	hdr->reserved = 0;
	hdr->type = type;
	hdr->version = FCGI_VERSION_1;
	if (pad) {
		memset(((unsigned char*)hdr) + sizeof(fcgi_header) + len, 0, pad);
	}
	return pad;
}

static int fcgi_get_params(fcgi_request *req, unsigned char *p, unsigned char *end)
{
	unsigned int name_len, val_len;

	while (p < end) {
		name_len = *p++;
		if (UNEXPECTED(name_len >= 128)) {
			if (UNEXPECTED(p + 3 >= end)) return 0;
			name_len = ((name_len & 0x7f) << 24);
			name_len |= (*p++ << 16);
			name_len |= (*p++ << 8);
			name_len |= *p++;
		}
		if (UNEXPECTED(p >= end)) return 0;
		val_len = *p++;
		if (UNEXPECTED(val_len >= 128)) {
			if (UNEXPECTED(p + 3 >= end)) return 0;
			val_len = ((val_len & 0x7f) << 24);
			val_len |= (*p++ << 16);
			val_len |= (*p++ << 8);
			val_len |= *p++;
		}
		if (UNEXPECTED(name_len + val_len > (unsigned int) (end - p))) {
			/* Malformated request */
			return 0;
		}
		fcgi_hash_set(&req->env, FCGI_HASH_FUNC(p, name_len), (char*)p, name_len, (char*)p + name_len, val_len);
		p += name_len + val_len;
	}
	return 1;
}

static int fcgi_read_request(fcgi_request *req)
{
	fcgi_header hdr;
	int len, padding;
	unsigned char buf[FCGI_MAX_LENGTH+8];

	req->keep = 0;
	req->closed = 0;
	req->in_len = 0;
	req->out_hdr = NULL;
	req->out_pos = req->out_buf;
	req->has_env = 1;

	if (safe_read(req, &hdr, sizeof(fcgi_header)) != sizeof(fcgi_header) ||
	    hdr.version < FCGI_VERSION_1) {
		return 0;
	}

	len = (hdr.contentLengthB1 << 8) | hdr.contentLengthB0;
	padding = hdr.paddingLength;

	while (hdr.type == FCGI_STDIN && len == 0) {
		if (safe_read(req, &hdr, sizeof(fcgi_header)) != sizeof(fcgi_header) ||
		    hdr.version < FCGI_VERSION_1) {
			return 0;
		}

		len = (hdr.contentLengthB1 << 8) | hdr.contentLengthB0;
		padding = hdr.paddingLength;
	}

	if (len + padding > FCGI_MAX_LENGTH) {
		return 0;
	}

	req->id = (hdr.requestIdB1 << 8) + hdr.requestIdB0;

	if (hdr.type == FCGI_BEGIN_REQUEST && len == sizeof(fcgi_begin_request)) {
		if (safe_read(req, buf, len+padding) != len+padding) {
			return 0;
		}

		req->keep = (((fcgi_begin_request*)buf)->flags & FCGI_KEEP_CONN);
#ifdef TCP_NODELAY
		if (req->keep && req->tcp && !req->nodelay) {
# ifdef _WIN32
			BOOL on = 1;
# else
			int on = 1;
# endif

			setsockopt(req->fd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on));
			req->nodelay = 1;
		}
#endif
		switch ((((fcgi_begin_request*)buf)->roleB1 << 8) + ((fcgi_begin_request*)buf)->roleB0) {
			case FCGI_RESPONDER:
				fcgi_hash_set(&req->env, FCGI_HASH_FUNC("FCGI_ROLE", sizeof("FCGI_ROLE")-1), "FCGI_ROLE", sizeof("FCGI_ROLE")-1, "RESPONDER", sizeof("RESPONDER")-1);
				break;
			case FCGI_AUTHORIZER:
				fcgi_hash_set(&req->env, FCGI_HASH_FUNC("FCGI_ROLE", sizeof("FCGI_ROLE")-1), "FCGI_ROLE", sizeof("FCGI_ROLE")-1, "AUTHORIZER", sizeof("AUTHORIZER")-1);
				break;
			case FCGI_FILTER:
				fcgi_hash_set(&req->env, FCGI_HASH_FUNC("FCGI_ROLE", sizeof("FCGI_ROLE")-1), "FCGI_ROLE", sizeof("FCGI_ROLE")-1, "FILTER", sizeof("FILTER")-1);
				break;
			default:
				return 0;
		}

		if (safe_read(req, &hdr, sizeof(fcgi_header)) != sizeof(fcgi_header) ||
		    hdr.version < FCGI_VERSION_1) {
			return 0;
		}

		len = (hdr.contentLengthB1 << 8) | hdr.contentLengthB0;
		padding = hdr.paddingLength;

		while (hdr.type == FCGI_PARAMS && len > 0) {
			if (len + padding > FCGI_MAX_LENGTH) {
				return 0;
			}

			if (safe_read(req, buf, len+padding) != len+padding) {
				req->keep = 0;
				return 0;
			}

			if (!fcgi_get_params(req, buf, buf+len)) {
				req->keep = 0;
				return 0;
			}

			if (safe_read(req, &hdr, sizeof(fcgi_header)) != sizeof(fcgi_header) ||
			    hdr.version < FCGI_VERSION_1) {
				req->keep = 0;
				return 0;
			}
			len = (hdr.contentLengthB1 << 8) | hdr.contentLengthB0;
			padding = hdr.paddingLength;
		}
	} else if (hdr.type == FCGI_GET_VALUES) {
		unsigned char *p = buf + sizeof(fcgi_header);
		zval *value;
		unsigned int zlen;
		fcgi_hash_bucket *q;

		if (safe_read(req, buf, len+padding) != len+padding) {
			req->keep = 0;
			return 0;
		}

		if (!fcgi_get_params(req, buf, buf+len)) {
			req->keep = 0;
			return 0;
		}

		q = req->env.list;
		while (q != NULL) {
			if ((value = zend_hash_str_find(&fcgi_mgmt_vars, q->var, q->var_len)) == NULL) {
				continue;
			}
			zlen = (unsigned int)Z_STRLEN_P(value);
			if ((p + 4 + 4 + q->var_len + zlen) >= (buf + sizeof(buf))) {
				break;
			}
			if (q->var_len < 0x80) {
				*p++ = q->var_len;
			} else {
				*p++ = ((q->var_len >> 24) & 0xff) | 0x80;
				*p++ = (q->var_len >> 16) & 0xff;
				*p++ = (q->var_len >> 8) & 0xff;
				*p++ = q->var_len & 0xff;
			}
			if (zlen < 0x80) {
				*p++ = zlen;
			} else {
				*p++ = ((zlen >> 24) & 0xff) | 0x80;
				*p++ = (zlen >> 16) & 0xff;
				*p++ = (zlen >> 8) & 0xff;
				*p++ = zlen & 0xff;
			}
			memcpy(p, q->var, q->var_len);
			p += q->var_len;
			memcpy(p, Z_STRVAL_P(value), zlen);
			p += zlen;
		}
		len = (int)(p - buf - sizeof(fcgi_header));
		len += fcgi_make_header((fcgi_header*)buf, FCGI_GET_VALUES_RESULT, 0, len);
		if (safe_write(req, buf, sizeof(fcgi_header)+len) != (int)sizeof(fcgi_header)+len) {
			req->keep = 0;
			return 0;
		}
		return 0;
	} else {
		return 0;
	}

	return 1;
}

int fcgi_read(fcgi_request *req, char *str, int len)
{
	int ret, n, rest;
	fcgi_header hdr;
	unsigned char buf[255];

	n = 0;
	rest = len;
	while (rest > 0) {
		if (req->in_len == 0) {
			if (safe_read(req, &hdr, sizeof(fcgi_header)) != sizeof(fcgi_header) ||
			    hdr.version < FCGI_VERSION_1 ||
			    hdr.type != FCGI_STDIN) {
				req->keep = 0;
				return 0;
			}
			req->in_len = (hdr.contentLengthB1 << 8) | hdr.contentLengthB0;
			req->in_pad = hdr.paddingLength;
			if (req->in_len == 0) {
				return n;
			}
		}

		if (req->in_len >= rest) {
			ret = (int)safe_read(req, str, rest);
		} else {
			ret = (int)safe_read(req, str, req->in_len);
		}
		if (ret < 0) {
			req->keep = 0;
			return ret;
		} else if (ret > 0) {
			req->in_len -= ret;
			rest -= ret;
			n += ret;
			str += ret;
			if (req->in_len == 0) {
				if (req->in_pad) {
					if (safe_read(req, buf, req->in_pad) != req->in_pad) {
						req->keep = 0;
						return ret;
					}
				}
			} else {
				return n;
			}
		} else {
			return n;
		}
	}
	return n;
}

static inline void fcgi_close(fcgi_request *req, int force, int destroy)
{
	if (destroy && req->has_env) {
		fcgi_hash_clean(&req->env);
		req->has_env = 0;
	}

#ifdef _WIN32
	if (is_impersonate && !req->tcp) {
		RevertToSelf();
	}
#endif

	if ((force || !req->keep) && req->fd >= 0) {
#ifdef _WIN32
		if (!req->tcp) {
			HANDLE pipe = (HANDLE)_get_osfhandle(req->fd);

			if (!force) {
				FlushFileBuffers(pipe);
			}
			DisconnectNamedPipe(pipe);
		} else {
			if (!force) {
				fcgi_header buf;

				shutdown(req->fd, 1);
				/* read the last FCGI_STDIN header (it may be omitted) */
				recv(req->fd, (char *)(&buf), sizeof(buf), 0);
			}
			closesocket(req->fd);
		}
#else
		if (!force) {
			fcgi_header buf;

			shutdown(req->fd, 1);
			/* read the last FCGI_STDIN header (it may be omitted) */
			recv(req->fd, (char *)(&buf), sizeof(buf), 0);
		}
		close(req->fd);
#endif
#ifdef TCP_NODELAY
		req->nodelay = 0;
#endif
		req->fd = -1;
	}
}

int fcgi_accept_request(fcgi_request *req)
{
#ifdef _WIN32
	HANDLE pipe;
	OVERLAPPED ov;
#endif

	while (1) {
		if (req->fd < 0) {
			while (1) {
				if (in_shutdown) {
					return -1;
				}
#ifdef _WIN32
				if (!req->tcp) {
					pipe = (HANDLE)_get_osfhandle(req->listen_socket);
					FCGI_LOCK(req->listen_socket);
					ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					if (!ConnectNamedPipe(pipe, &ov)) {
						errno = GetLastError();
						if (errno == ERROR_IO_PENDING) {
							while (WaitForSingleObject(ov.hEvent, 1000) == WAIT_TIMEOUT) {
								if (in_shutdown) {
									CloseHandle(ov.hEvent);
									FCGI_UNLOCK(req->listen_socket);
									return -1;
								}
							}
						} else if (errno != ERROR_PIPE_CONNECTED) {
						}
					}
					CloseHandle(ov.hEvent);
					req->fd = req->listen_socket;
					FCGI_UNLOCK(req->listen_socket);
				} else {
					SOCKET listen_socket = (SOCKET)_get_osfhandle(req->listen_socket);
#else
				{
					int listen_socket = req->listen_socket;
#endif
					sa_t sa;
					socklen_t len = sizeof(sa);

					FCGI_LOCK(req->listen_socket);
					req->fd = accept(listen_socket, (struct sockaddr *)&sa, &len);
					FCGI_UNLOCK(req->listen_socket);
					if (req->fd >= 0) {
						if (((struct sockaddr *)&sa)->sa_family == AF_INET) {
#ifndef _WIN32
							req->tcp = 1;
#endif
							if (allowed_clients) {
								int n = 0;
								int allowed = 0;

								while (allowed_clients[n] != INADDR_NONE) {
									if (allowed_clients[n] == sa.sa_inet.sin_addr.s_addr) {
										allowed = 1;
										break;
									}
									n++;
								}
								if (!allowed) {
									fprintf(stderr, "Connection from disallowed IP address '%s' is dropped.\n", inet_ntoa(sa.sa_inet.sin_addr));
									closesocket(req->fd);
									req->fd = -1;
									continue;
								}
							}
#ifndef _WIN32
						} else {
							req->tcp = 0;
#endif
						}
					}
				}

#ifdef _WIN32
				if (req->fd < 0 && (in_shutdown || errno != EINTR)) {
#else
				if (req->fd < 0 && (in_shutdown || (errno != EINTR && errno != ECONNABORTED))) {
#endif
					return -1;
				}

#ifdef _WIN32
				break;
#else
				if (req->fd >= 0) {
#if defined(HAVE_SYS_POLL_H) && defined(HAVE_POLL)
					struct pollfd fds;
					int ret;

					fds.fd = req->fd;
					fds.events = POLLIN;
					fds.revents = 0;
					do {
						errno = 0;
						ret = poll(&fds, 1, 5000);
					} while (ret < 0 && errno == EINTR);
					if (ret > 0 && (fds.revents & POLLIN)) {
						break;
					}
					fcgi_close(req, 1, 0);
#else
					if (req->fd < FD_SETSIZE) {
						struct timeval tv = {5,0};
						fd_set set;
						int ret;

						FD_ZERO(&set);
						FD_SET(req->fd, &set);
						do {
							errno = 0;
							ret = select(req->fd + 1, &set, NULL, NULL, &tv) >= 0;
						} while (ret < 0 && errno == EINTR);
						if (ret > 0 && FD_ISSET(req->fd, &set)) {
							break;
						}
						fcgi_close(req, 1, 0);
					} else {
						fprintf(stderr, "Too many open file descriptors. FD_SETSIZE limit exceeded.");
						fcgi_close(req, 1, 0);
					}
#endif
				}
#endif
			}
		} else if (in_shutdown) {
			return -1;
		}
		if (fcgi_read_request(req)) {
#ifdef _WIN32
			if (is_impersonate && !req->tcp) {
				pipe = (HANDLE)_get_osfhandle(req->fd);
				if (!ImpersonateNamedPipeClient(pipe)) {
					fcgi_close(req, 1, 1);
					continue;
				}
			}
#endif
			return req->fd;
		} else {
			fcgi_close(req, 1, 1);
		}
	}
}

static inline fcgi_header* open_packet(fcgi_request *req, fcgi_request_type type)
{
	req->out_hdr = (fcgi_header*) req->out_pos;
	req->out_hdr->type = type;
	req->out_pos += sizeof(fcgi_header);
	return req->out_hdr;
}

static inline void close_packet(fcgi_request *req)
{
	if (req->out_hdr) {
		int len = (int)(req->out_pos - ((unsigned char*)req->out_hdr + sizeof(fcgi_header)));

		req->out_pos += fcgi_make_header(req->out_hdr, (fcgi_request_type)req->out_hdr->type, req->id, len);
		req->out_hdr = NULL;
	}
}

int fcgi_flush(fcgi_request *req, int close)
{
	int len;

	close_packet(req);

	len = (int)(req->out_pos - req->out_buf);

	if (close) {
		fcgi_end_request_rec *rec = (fcgi_end_request_rec*)(req->out_pos);

		fcgi_make_header(&rec->hdr, FCGI_END_REQUEST, req->id, sizeof(fcgi_end_request));
		rec->body.appStatusB3 = 0;
		rec->body.appStatusB2 = 0;
		rec->body.appStatusB1 = 0;
		rec->body.appStatusB0 = 0;
		rec->body.protocolStatus = FCGI_REQUEST_COMPLETE;
		len += sizeof(fcgi_end_request_rec);
	}

	if (safe_write(req, req->out_buf, len) != len) {
		req->keep = 0;
		req->out_pos = req->out_buf;
		return 0;
	}

	req->out_pos = req->out_buf;
	return 1;
}

int fcgi_write(fcgi_request *req, fcgi_request_type type, const char *str, int len)
{
	int limit, rest;

	if (len <= 0) {
		return 0;
	}

	if (req->out_hdr && req->out_hdr->type != type) {
		close_packet(req);
	}
#if 0
	/* Unoptimized, but clear version */
	rest = len;
	while (rest > 0) {
		limit = sizeof(req->out_buf) - (req->out_pos - req->out_buf);

		if (!req->out_hdr) {
			if (limit < sizeof(fcgi_header)) {
				if (!fcgi_flush(req, 0)) {
					return -1;
				}
			}
			open_packet(req, type);
		}
		limit = sizeof(req->out_buf) - (req->out_pos - req->out_buf);
		if (rest < limit) {
			memcpy(req->out_pos, str, rest);
			req->out_pos += rest;
			return len;
		} else {
			memcpy(req->out_pos, str, limit);
			req->out_pos += limit;
			rest -= limit;
			str += limit;
			if (!fcgi_flush(req, 0)) {
				return -1;
			}
		}
	}
#else
	/* Optimized version */
	limit = (int)(sizeof(req->out_buf) - (req->out_pos - req->out_buf));
	if (!req->out_hdr) {
		limit -= sizeof(fcgi_header);
		if (limit < 0) limit = 0;
	}

	if (len < limit) {
		if (!req->out_hdr) {
			open_packet(req, type);
		}
		memcpy(req->out_pos, str, len);
		req->out_pos += len;
	} else if (len - limit < sizeof(req->out_buf) - sizeof(fcgi_header)) {
		if (!req->out_hdr) {
			open_packet(req, type);
		}
		if (limit > 0) {
			memcpy(req->out_pos, str, limit);
			req->out_pos += limit;
		}
		if (!fcgi_flush(req, 0)) {
			return -1;
		}
		if (len > limit) {
			open_packet(req, type);
			memcpy(req->out_pos, str + limit, len - limit);
			req->out_pos += len - limit;
		}
	} else {
		int pos = 0;
		int pad;

		close_packet(req);
		while ((len - pos) > 0xffff) {
			open_packet(req, type);
			fcgi_make_header(req->out_hdr, type, req->id, 0xfff8);
			req->out_hdr = NULL;
			if (!fcgi_flush(req, 0)) {
				return -1;
			}
			if (safe_write(req, str + pos, 0xfff8) != 0xfff8) {
				req->keep = 0;
				return -1;
			}
			pos += 0xfff8;
		}

		pad = (((len - pos) + 7) & ~7) - (len - pos);
		rest = pad ? 8 - pad : 0;

		open_packet(req, type);
		fcgi_make_header(req->out_hdr, type, req->id, (len - pos) - rest);
		req->out_hdr = NULL;
		if (!fcgi_flush(req, 0)) {
			return -1;
		}
		if (safe_write(req, str + pos, (len - pos) - rest) != (len - pos) - rest) {
			req->keep = 0;
			return -1;
		}
		if (pad) {
			open_packet(req, type);
			memcpy(req->out_pos, str + len - rest,  rest);
			req->out_pos += rest;
		}
	}
#endif
	return len;
}

int fcgi_finish_request(fcgi_request *req, int force_close)
{
	int ret = 1;

	if (req->fd >= 0) {
		if (!req->closed) {
			ret = fcgi_flush(req, 1);
			req->closed = 1;
		}
		fcgi_close(req, force_close, 1);
	}
	return ret;
}

char* fcgi_getenv(fcgi_request *req, const char* var, int var_len)
{
	unsigned int val_len;

	if (!req) return NULL;

	return fcgi_hash_get(&req->env, FCGI_HASH_FUNC(var, var_len), (char*)var, var_len, &val_len);
}

char* fcgi_quick_getenv(fcgi_request *req, const char* var, int var_len, unsigned int hash_value)
{
	unsigned int val_len;

	return fcgi_hash_get(&req->env, hash_value, (char*)var, var_len, &val_len);
}

char* fcgi_putenv(fcgi_request *req, char* var, int var_len, char* val)
{
	if (!req) return NULL;
	if (val == NULL) {
		fcgi_hash_del(&req->env, FCGI_HASH_FUNC(var, var_len), var, var_len);
		return NULL;
	} else {
		return fcgi_hash_set(&req->env, FCGI_HASH_FUNC(var, var_len), var, var_len, val, (unsigned int)strlen(val));
	}
}

char* fcgi_quick_putenv(fcgi_request *req, char* var, int var_len, unsigned int hash_value, char* val)
{
	if (val == NULL) {
		fcgi_hash_del(&req->env, hash_value, var, var_len);
		return NULL;
	} else {
		return fcgi_hash_set(&req->env, hash_value, var, var_len, val, (unsigned int)strlen(val));
	}
}

void fcgi_loadenv(fcgi_request *req, fcgi_apply_func func, zval *array)
{
	fcgi_hash_apply(&req->env, func, array);
}

#ifdef _WIN32
void fcgi_impersonate(void)
{
	char *os_name;

	os_name = getenv("OS");
	if (os_name && stricmp(os_name, "Windows_NT") == 0) {
		is_impersonate = 1;
	}
}
#endif

void fcgi_set_mgmt_var(const char * name, size_t name_len, const char * value, size_t value_len)
{
	zval zvalue;
	ZVAL_NEW_STR(&zvalue, zend_string_init(value, value_len, 1));
	zend_hash_str_add(&fcgi_mgmt_vars, name, name_len, &zvalue);
}

void fcgi_free_mgmt_var_cb(zval *zv)
{
	pefree(Z_STR_P(zv), 1);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */