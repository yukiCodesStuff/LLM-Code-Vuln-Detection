/* }}} */


/* {{{ mysqlnd_conn_data::simple_command_send_request */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, simple_command_send_request)(MYSQLND_CONN_DATA * conn, enum php_mysqlnd_server_command command,
			   const zend_uchar * const arg, size_t arg_len, zend_bool silent, zend_bool ignore_upsert_status TSRMLS_DC)
{
	enum_func_status ret = PASS;
	MYSQLND_PACKET_COMMAND * cmd_packet;

	DBG_ENTER("mysqlnd_conn_data::simple_command_send_request");
	DBG_INF_FMT("command=%s silent=%u", mysqlnd_command_to_text[command], silent);

	switch (CONN_GET_STATE(conn)) {
		case CONN_READY:
			break;
			php_error(E_WARNING, "Error while sending %s packet. PID=%d", mysqlnd_command_to_text[command], getpid());
		}
		CONN_SET_STATE(conn, CONN_QUIT_SENT);
		conn->m->send_close(conn TSRMLS_CC);
		DBG_ERR("Server is gone");
		ret = FAIL;
	}
	PACKET_FREE(cmd_packet);
	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_conn_data::simple_command */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, simple_command)(MYSQLND_CONN_DATA * conn, enum php_mysqlnd_server_command command,
			   const zend_uchar * const arg, size_t arg_len, enum mysqlnd_packet_type ok_packet, zend_bool silent,
			   zend_bool ignore_upsert_status TSRMLS_DC)
{
	enum_func_status ret;
	DBG_ENTER("mysqlnd_conn_data::simple_command");

	ret = conn->m->simple_command_send_request(conn, command, arg, arg_len, silent, ignore_upsert_status TSRMLS_CC);
	if (PASS == ret && ok_packet != PROT_LAST) {
		ret = conn->m->simple_command_handle_response(conn, ok_packet, silent, command, ignore_upsert_status TSRMLS_CC);
	}

	DBG_INF(ret == PASS ? "PASS":"FAIL");
	DBG_RETURN(ret);
}
/* }}} */
		DBG_INF("Switching to SSL");
		if (!PACKET_WRITE(auth_packet, conn)) {
			CONN_SET_STATE(conn, CONN_QUIT_SENT);
			conn->m->send_close(conn TSRMLS_CC);
			SET_CLIENT_ERROR(*conn->error_info, CR_SERVER_GONE_ERROR, UNKNOWN_SQLSTATE, mysqlnd_server_gone);
			goto end;
		}

/* }}} */


/* {{{ mysqlnd_conn_data::fetch_auth_plugin_by_name */
static struct st_mysqlnd_authentication_plugin *
MYSQLND_METHOD(mysqlnd_conn_data, fetch_auth_plugin_by_name)(const char * const requested_protocol TSRMLS_DC)
{
	struct st_mysqlnd_authentication_plugin * auth_plugin;
	char * plugin_name = NULL;
	DBG_ENTER("mysqlnd_conn_data::fetch_auth_plugin_by_name");

	mnd_sprintf(&plugin_name, 0, "auth_plugin_%s", requested_protocol);
	DBG_INF_FMT("looking for %s auth plugin", plugin_name);
	auth_plugin = mysqlnd_plugin_find(plugin_name);
	mnd_sprintf_free(plugin_name);

	DBG_RETURN(auth_plugin);
}
/* }}} */


/* {{{ mysqlnd_run_authentication */
static enum_func_status
mysqlnd_run_authentication(
			MYSQLND_CONN_DATA * conn,
			const char * const user,
			const char * const passwd,
			const size_t passwd_len,
			const char * const db,
			const size_t db_len,
			const zend_uchar * const auth_plugin_data,
			const size_t auth_plugin_data_len,
			const char * const auth_protocol,
			unsigned int charset_no,
			const MYSQLND_OPTIONS * const options,
			unsigned long mysql_flags,
			zend_bool silent,
			zend_bool is_change_user
			TSRMLS_DC)
{
	enum_func_status ret = FAIL;
	zend_bool first_call = TRUE;

	char * switch_to_auth_protocol = NULL;
	size_t switch_to_auth_protocol_len = 0;
	char * requested_protocol = NULL;
	zend_uchar * plugin_data;
	size_t plugin_data_len;

	DBG_ENTER("mysqlnd_run_authentication");

	plugin_data_len = auth_plugin_data_len;
	plugin_data = mnd_emalloc(plugin_data_len + 1);
	if (!plugin_data) {
		goto end;
	}
	memcpy(plugin_data, auth_plugin_data, plugin_data_len);
	plugin_data[plugin_data_len] = '\0';

	requested_protocol = mnd_pestrdup(auth_protocol? auth_protocol : MYSQLND_DEFAULT_AUTH_PROTOCOL, FALSE);
	if (!requested_protocol) {
		goto end;
	}

	do {
		struct st_mysqlnd_authentication_plugin * auth_plugin = conn->m->fetch_auth_plugin_by_name(requested_protocol TSRMLS_CC);

		if (!auth_plugin) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "The server requested authentication method unknown to the client [%s]", requested_protocol);
			SET_CLIENT_ERROR(*conn->error_info, CR_NOT_IMPLEMENTED, UNKNOWN_SQLSTATE, "The server requested authentication method umknown to the client");
			goto end;
		}
		DBG_INF("plugin found");

		{
			zend_uchar * switch_to_auth_protocol_data = NULL;
			size_t switch_to_auth_protocol_data_len = 0;
			zend_uchar * scrambled_data = NULL;
			size_t scrambled_data_len = 0;

			switch_to_auth_protocol = NULL;
			switch_to_auth_protocol_len = 0;

			if (conn->auth_plugin_data) {
				mnd_pefree(conn->auth_plugin_data, conn->persistent);
				conn->auth_plugin_data = NULL;
			}
			conn->auth_plugin_data_len = plugin_data_len;
			conn->auth_plugin_data = mnd_pemalloc(conn->auth_plugin_data_len, conn->persistent);
			if (!conn->auth_plugin_data) {
				SET_OOM_ERROR(*conn->error_info);
				goto end;
			}
			memcpy(conn->auth_plugin_data, plugin_data, plugin_data_len);

			DBG_INF_FMT("salt=[%*s]", plugin_data_len - 1, plugin_data);
			/* The data should be allocated with malloc() */
			scrambled_data =
				auth_plugin->methods.get_auth_data(NULL, &scrambled_data_len, conn, user, passwd, passwd_len,
												   plugin_data, plugin_data_len, options, mysql_flags TSRMLS_CC);

			if (FALSE == is_change_user) {
				ret = mysqlnd_auth_handshake(conn, user, passwd, passwd_len, db, db_len, options, mysql_flags,
											charset_no,
											first_call,
											requested_protocol,
											scrambled_data, scrambled_data_len,
											&switch_to_auth_protocol, &switch_to_auth_protocol_len,
											&switch_to_auth_protocol_data, &switch_to_auth_protocol_data_len
											TSRMLS_CC);
			} else {
				ret = mysqlnd_auth_change_user(conn, user, strlen(user), passwd, passwd_len, db, db_len, silent,
											   first_call,
											   requested_protocol,
											   scrambled_data, scrambled_data_len,
											   &switch_to_auth_protocol, &switch_to_auth_protocol_len,
											   &switch_to_auth_protocol_data, &switch_to_auth_protocol_data_len
											   TSRMLS_CC);				
			}
			first_call = FALSE;
			free(scrambled_data);

			DBG_INF_FMT("switch_to_auth_protocol=%s", switch_to_auth_protocol? switch_to_auth_protocol:"n/a");
			if (requested_protocol && switch_to_auth_protocol) {
				mnd_efree(requested_protocol);
				requested_protocol = switch_to_auth_protocol;
			}

			if (plugin_data) {
				mnd_efree(plugin_data);
			}
			plugin_data_len = switch_to_auth_protocol_data_len;
			plugin_data = switch_to_auth_protocol_data;
		}
		DBG_INF_FMT("conn->error_info->error_no = %d", conn->error_info->error_no);
	} while (ret == FAIL && conn->error_info->error_no == 0 && switch_to_auth_protocol != NULL);
	if (plugin_data) {
		mnd_efree(plugin_data);
	}
		
	if (ret == PASS) {
		DBG_INF_FMT("saving requested_protocol=%s", requested_protocol);
		conn->m->set_client_option(conn, MYSQLND_OPT_AUTH_PROTOCOL, requested_protocol TSRMLS_CC);
	}

	if (requested_protocol) {
		mnd_efree(requested_protocol);
	}
end:
	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_connect_run_authentication */
static enum_func_status
mysqlnd_connect_run_authentication(
			MYSQLND_CONN_DATA * conn,
			const char * const user,
			const char * const passwd,
			const char * const db,
			size_t db_len,
			size_t passwd_len,
			const MYSQLND_PACKET_GREET * const greet_packet,
			const MYSQLND_OPTIONS * const options,
			unsigned long mysql_flags
			TSRMLS_DC)
{
	enum_func_status ret = FAIL;
	DBG_ENTER("mysqlnd_connect_run_authentication");

	ret = mysqlnd_switch_to_ssl_if_needed(conn, greet_packet, options, mysql_flags TSRMLS_CC);
	if (PASS == ret) {
		ret = mysqlnd_run_authentication(conn, user, passwd, passwd_len, db, db_len,
										 greet_packet->auth_plugin_data, greet_packet->auth_plugin_data_len, greet_packet->auth_protocol,
										 greet_packet->charset_no, options, mysql_flags, FALSE /*silent*/, FALSE/*is_change*/ TSRMLS_CC);
	}
	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_conn_data::execute_init_commands */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, execute_init_commands)(MYSQLND_CONN_DATA * conn TSRMLS_DC)
{
	enum_func_status ret = PASS;

	DBG_ENTER("mysqlnd_conn_data::execute_init_commands");
	if (conn->options->init_commands) {
		unsigned int current_command = 0;
		for (; current_command < conn->options->num_commands; ++current_command) {
			const char * const command = conn->options->init_commands[current_command];
			if (command) {
				MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_INIT_COMMAND_EXECUTED_COUNT);
				if (PASS != conn->m->query(conn, command, strlen(command) TSRMLS_CC)) {
					MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_INIT_COMMAND_FAILED_COUNT);
					ret = FAIL;
					break;
				}
				if (conn->last_query_type == QUERY_SELECT) {
					MYSQLND_RES * result = conn->m->use_result(conn TSRMLS_CC);
					if (result) {
						result->m.free_result(result, TRUE TSRMLS_CC);
					}
				}
			}
		}
	}
	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_conn_data::get_updated_connect_flags */
static unsigned int
MYSQLND_METHOD(mysqlnd_conn_data, get_updated_connect_flags)(MYSQLND_CONN_DATA * conn, unsigned int mysql_flags TSRMLS_DC)
{
	MYSQLND_NET * net = conn->net;

	DBG_ENTER("mysqlnd_conn_data::get_updated_connect_flags");
	/* we allow load data local infile by default */
	mysql_flags |= MYSQLND_CAPABILITIES;

	if (PG(open_basedir) && strlen(PG(open_basedir))) {
		mysql_flags ^= CLIENT_LOCAL_FILES;
	}

#ifndef MYSQLND_COMPRESSION_ENABLED
	if (mysql_flags & CLIENT_COMPRESS) {
		mysql_flags &= ~CLIENT_COMPRESS;
	}
#else
	if (net && net->data->options.flags & MYSQLND_NET_FLAG_USE_COMPRESSION) {
		mysql_flags |= CLIENT_COMPRESS;
	}
#endif
#ifndef MYSQLND_SSL_SUPPORTED
	if (mysql_flags & CLIENT_SSL) {
		mysql_flags &= ~CLIENT_SSL;
	}
#else
	if (net && (net->data->options.ssl_key || net->data->options.ssl_cert ||
		net->data->options.ssl_ca || net->data->options.ssl_capath || net->data->options.ssl_cipher))
	{
		mysql_flags |= CLIENT_SSL;
	}
#endif

	DBG_RETURN(mysql_flags);
}
/* }}} */


/* {{{ mysqlnd_conn_data::connect_handshake */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, connect_handshake)(MYSQLND_CONN_DATA * conn,
						const char * const host, const char * const user,
						const char * const passwd, const unsigned int passwd_len,
						const char * const db, const unsigned int db_len,
						const unsigned int mysql_flags TSRMLS_DC)
{
	MYSQLND_PACKET_GREET * greet_packet;
	MYSQLND_NET * net = conn->net;

	DBG_ENTER("mysqlnd_conn_data::connect_handshake");

	greet_packet = conn->protocol->m.get_greet_packet(conn->protocol, FALSE TSRMLS_CC);
	if (!greet_packet) {
		SET_OOM_ERROR(*conn->error_info);
		DBG_RETURN(FAIL); /* OOM */
	}

	if (FAIL == net->data->m.connect_ex(conn->net, conn->scheme, conn->scheme_len, conn->persistent,
										conn->stats, conn->error_info TSRMLS_CC))
	{
		goto err;
	}

	DBG_INF_FMT("stream=%p", net->data->m.get_stream(net TSRMLS_CC));

	if (FAIL == PACKET_READ(greet_packet, conn)) {
		DBG_ERR("Error while reading greeting packet");
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error while reading greeting packet. PID=%d", getpid());
		goto err;
	} else if (greet_packet->error_no) {
		DBG_ERR_FMT("errorno=%u error=%s", greet_packet->error_no, greet_packet->error);
		SET_CLIENT_ERROR(*conn->error_info, greet_packet->error_no, greet_packet->sqlstate, greet_packet->error);
		goto err;
	} else if (greet_packet->pre41) {
		DBG_ERR_FMT("Connecting to 3.22, 3.23 & 4.0 is not supported. Server is %-.32s", greet_packet->server_version);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connecting to 3.22, 3.23 & 4.0 "
						" is not supported. Server is %-.32s", greet_packet->server_version);
		SET_CLIENT_ERROR(*conn->error_info, CR_NOT_IMPLEMENTED, UNKNOWN_SQLSTATE,
						 "Connecting to 3.22, 3.23 & 4.0 servers is not supported");
		goto err;
	}

	conn->thread_id			= greet_packet->thread_id;
	conn->protocol_version	= greet_packet->protocol_version;
	conn->server_version	= mnd_pestrdup(greet_packet->server_version, conn->persistent);

	conn->greet_charset = mysqlnd_find_charset_nr(greet_packet->charset_no);

	if (FAIL == mysqlnd_connect_run_authentication(conn, user, passwd, db, db_len, (size_t) passwd_len,
												   greet_packet, conn->options, mysql_flags TSRMLS_CC))
	{
		goto err;
	}
	conn->client_flag			= mysql_flags;
	conn->server_capabilities 	= greet_packet->server_capabilities;
	conn->upsert_status->warning_count = 0;
	conn->upsert_status->server_status = greet_packet->server_status;
	conn->upsert_status->affected_rows = 0;

	PACKET_FREE(greet_packet);
	DBG_RETURN(PASS);
err:
	PACKET_FREE(greet_packet);
	DBG_RETURN(FAIL);
}
/* }}} */


/* {{{ mysqlnd_conn_data::connect */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, connect)(MYSQLND_CONN_DATA * conn,
						 const char *host, const char *user,
	zend_bool local_tx_started = FALSE;
	MYSQLND_NET * net = conn->net;

	DBG_ENTER("mysqlnd_conn_data::connect");

	if (PASS != conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		goto err;
		DBG_INF_FMT("no db given, using empty string");
		db = "";
		db_len = 0;
	} else {
		mysql_flags |= CLIENT_CONNECT_WITH_DB;	
	}

	host_len = strlen(host);
	{
		}
	}

	mysql_flags = conn->m->get_updated_connect_flags(conn, mysql_flags TSRMLS_CC);

	if (FAIL == conn->m->connect_handshake(conn, host, user, passwd, passwd_len, db, db_len, mysql_flags TSRMLS_CC)) {
		goto err;
	}

	{
			}
			conn->unix_socket_len = strlen(conn->unix_socket);
		}
		conn->max_packet_size	= MYSQLND_ASSEMBLED_PACKET_MAX_SIZE;
		/* todo: check if charset is available */

		SET_EMPTY_ERROR(*conn->error_info);

		mysqlnd_local_infile_default(conn);
			DBG_INF("unicode set");
		}
#endif
		if (FAIL == conn->m->execute_init_commands(conn TSRMLS_CC)) {
			goto err;
		}

		MYSQLND_INC_CONN_STATISTIC_W_VALUE2(conn->stats, STAT_CONNECT_SUCCESS, 1, STAT_OPENED_CONNECTIONS, 1);
		if (reconnect) {
			MYSQLND_INC_GLOBAL_STATISTIC(STAT_RECONNECT);
		}

		DBG_INF_FMT("connection_id=%llu", conn->thread_id);

		conn->m->local_tx_end(conn, this_func, PASS TSRMLS_CC);
		DBG_RETURN(PASS);
	}
err:

	DBG_ERR_FMT("[%u] %.128s (trying to connect via %s)", conn->error_info->error_no, conn->error_info->error, conn->scheme);
	if (!conn->error_info->error_no) {
		SET_CLIENT_ERROR(*conn->error_info, CR_CONNECTION_ERROR, UNKNOWN_SQLSTATE, conn->error_info->error? conn->error_info->error:"Unknown error");
			SET_ERROR_AFF_ROWS(conn);
		} else if (PASS == (ret = conn->m->simple_command(conn, COM_PROCESS_KILL, buff, 4, PROT_LAST, FALSE, TRUE TSRMLS_CC))) {
			CONN_SET_STATE(conn, CONN_QUIT_SENT);
			conn->m->send_close(conn TSRMLS_CC);
		}

		conn->m->local_tx_end(conn, this_func, ret TSRMLS_CC);
	}
	  connection does increase it by 1.
	*/
	CONN_SET_STATE(conn, CONN_QUIT_SENT);
	net->data->m.close_stream(net, conn->stats, conn->error_info TSRMLS_CC);

	DBG_RETURN(ret);
}
/* }}} */
					DBG_ERR_FMT("Serious error. %s::%u", __FILE__, __LINE__);
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Serious error. PID=%d", getpid());
					CONN_SET_STATE(conn, CONN_QUIT_SENT);
					conn->m->send_close(conn TSRMLS_CC);
				} else {
					DBG_INF_FMT("Error from the server : (%u) %s", conn->error_info->error_no, conn->error_info->error);
				}
				break;
										  TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, change_user);
	enum_func_status ret = FAIL;

	DBG_ENTER("mysqlnd_conn_data::change_user");
	DBG_INF_FMT("conn=%llu user=%s passwd=%s db=%s silent=%u",
				conn->thread_id, user?user:"", passwd?"***":"null", db?db:"", (silent == TRUE)?1:0 );
	if (PASS != conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		goto end;
	}

	SET_EMPTY_ERROR(*conn->error_info);
	SET_ERROR_AFF_ROWS(conn);

	}
	if (!db) {
		db = "";

	}

	/* XXX: passwords that have \0 inside work during auth, but in this case won't work with change user */
	ret = mysqlnd_run_authentication(conn, user, passwd, strlen(passwd), db, strlen(db),
									conn->auth_plugin_data, conn->auth_plugin_data_len, conn->options->auth_protocol,
									0 /*charset not used*/, conn->options, conn->server_capabilities, silent, TRUE/*is_change*/ TSRMLS_CC);

	/*
	  Here we should close all statements. Unbuffered queries should not be a
	  problem as we won't allow sending COM_CHANGE_USER.
	*/
	conn->m->local_tx_end(conn, this_func, ret TSRMLS_CC);	
end:
	DBG_INF(ret == PASS? "PASS":"FAIL");
	DBG_RETURN(ret);
}
/* }}} */
	MYSQLND_METHOD(mysqlnd_conn_data, tx_commit),
	MYSQLND_METHOD(mysqlnd_conn_data, tx_rollback),
	MYSQLND_METHOD(mysqlnd_conn_data, local_tx_start),
	MYSQLND_METHOD(mysqlnd_conn_data, local_tx_end),
	MYSQLND_METHOD(mysqlnd_conn_data, execute_init_commands),
	MYSQLND_METHOD(mysqlnd_conn_data, get_updated_connect_flags),
	MYSQLND_METHOD(mysqlnd_conn_data, connect_handshake),
	MYSQLND_METHOD(mysqlnd_conn_data, simple_command_send_request),
	MYSQLND_METHOD(mysqlnd_conn_data, fetch_auth_plugin_by_name)	
MYSQLND_CLASS_METHODS_END;


/* {{{ mysqlnd_conn::get_reference */