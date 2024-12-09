			} else {
				DBG_INF_FMT("OK from server");
			}
			PACKET_FREE(ok_response);
			break;
		}
		default:
			SET_CLIENT_ERROR(*conn->error_info, CR_MALFORMED_PACKET, UNKNOWN_SQLSTATE, "Malformed packet");
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Wrong response packet %u passed to the function", ok_packet);
			break;
	}
	DBG_INF(ret == PASS ? "PASS":"FAIL");
	DBG_RETURN(ret);
}
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
		case CONN_QUIT_SENT:
			SET_CLIENT_ERROR(*conn->error_info, CR_SERVER_GONE_ERROR, UNKNOWN_SQLSTATE, mysqlnd_server_gone);
			DBG_ERR("Server is gone");
			DBG_RETURN(FAIL);
		default:
			SET_CLIENT_ERROR(*conn->error_info, CR_COMMANDS_OUT_OF_SYNC, UNKNOWN_SQLSTATE, mysqlnd_out_of_sync);
			DBG_ERR_FMT("Command out of sync. State=%u", CONN_GET_STATE(conn));
			DBG_RETURN(FAIL);
	}

	/* clean UPSERT info */
	if (!ignore_upsert_status) {
		memset(conn->upsert_status, 0, sizeof(*conn->upsert_status));
	}
	SET_ERROR_AFF_ROWS(conn);
	SET_EMPTY_ERROR(*conn->error_info);

	cmd_packet = conn->protocol->m.get_command_packet(conn->protocol, FALSE TSRMLS_CC);
	if (!cmd_packet) {
		SET_OOM_ERROR(*conn->error_info);
		DBG_RETURN(FAIL);
	}

	cmd_packet->command = command;
	if (arg && arg_len) {
		cmd_packet->argument = arg;
		cmd_packet->arg_len  = arg_len;
	}

	MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_COM_QUIT + command - 1 /* because of COM_SLEEP */ );

	if (! PACKET_WRITE(cmd_packet, conn)) {
		if (!silent) {
			DBG_ERR_FMT("Error while sending %s packet", mysqlnd_command_to_text[command]);
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
		if (!silent) {
			DBG_ERR_FMT("Error while sending %s packet", mysqlnd_command_to_text[command]);
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


/* {{{ mysqlnd_conn_data::set_server_option */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, set_server_option)(MYSQLND_CONN_DATA * const conn, enum_mysqlnd_server_option option TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, set_server_option);
	zend_uchar buffer[2];
	enum_func_status ret = FAIL;
	DBG_ENTER("mysqlnd_conn_data::set_server_option");
	if (PASS == conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {

		int2store(buffer, (unsigned int) option);
		ret = conn->m->simple_command(conn, COM_SET_OPTION, buffer, sizeof(buffer), PROT_EOF_PACKET, FALSE, TRUE TSRMLS_CC);
	
		conn->m->local_tx_end(conn, this_func, ret TSRMLS_CC);
	}
	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_conn_data::restart_psession */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, restart_psession)(MYSQLND_CONN_DATA * conn TSRMLS_DC)
{
	DBG_ENTER("mysqlnd_conn_data::restart_psession");
	MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_CONNECT_REUSED);
	/* Free here what should not be seen by the next script */
	if (conn->last_message) {
		mnd_pefree(conn->last_message, conn->persistent);
		conn->last_message = NULL;
	}
	DBG_RETURN(PASS);
}
/* }}} */


/* {{{ mysqlnd_conn_data::end_psession */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, end_psession)(MYSQLND_CONN_DATA * conn TSRMLS_DC)
{
	DBG_ENTER("mysqlnd_conn_data::end_psession");
	DBG_RETURN(PASS);
}
/* }}} */


/* {{{ mysqlnd_switch_to_ssl_if_needed */
static enum_func_status
mysqlnd_switch_to_ssl_if_needed(
			MYSQLND_CONN_DATA * conn,
			const MYSQLND_PACKET_GREET * const greet_packet,
			const MYSQLND_OPTIONS * const options,
			unsigned long mysql_flags
			TSRMLS_DC
		)
{
	enum_func_status ret = FAIL;
	const MYSQLND_CHARSET * charset;
	MYSQLND_PACKET_AUTH * auth_packet;
	DBG_ENTER("mysqlnd_switch_to_ssl_if_needed");

	auth_packet = conn->protocol->m.get_auth_packet(conn->protocol, FALSE TSRMLS_CC);
	if (!auth_packet) {
		SET_OOM_ERROR(*conn->error_info);
		goto end;
	}
	auth_packet->client_flags = mysql_flags;
	auth_packet->max_packet_size = MYSQLND_ASSEMBLED_PACKET_MAX_SIZE;

	if (options->charset_name && (charset = mysqlnd_find_charset_name(options->charset_name))) {
		auth_packet->charset_no	= charset->nr;
	} else {
#if MYSQLND_UNICODE
		auth_packet->charset_no	= 200;/* utf8 - swedish collation, check mysqlnd_charset.c */
#else
		auth_packet->charset_no	= greet_packet->charset_no;
#endif
	}

#ifdef MYSQLND_SSL_SUPPORTED
	if ((greet_packet->server_capabilities & CLIENT_SSL) && (mysql_flags & CLIENT_SSL)) {
		zend_bool verify = mysql_flags & CLIENT_SSL_VERIFY_SERVER_CERT? TRUE:FALSE;
		DBG_INF("Switching to SSL");
		if (!PACKET_WRITE(auth_packet, conn)) {
			CONN_SET_STATE(conn, CONN_QUIT_SENT);
			conn->m->send_close(conn TSRMLS_CC);
			SET_CLIENT_ERROR(*conn->error_info, CR_SERVER_GONE_ERROR, UNKNOWN_SQLSTATE, mysqlnd_server_gone);
			goto end;
		}
		if (!PACKET_WRITE(auth_packet, conn)) {
			CONN_SET_STATE(conn, CONN_QUIT_SENT);
			conn->m->send_close(conn TSRMLS_CC);
			SET_CLIENT_ERROR(*conn->error_info, CR_SERVER_GONE_ERROR, UNKNOWN_SQLSTATE, mysqlnd_server_gone);
			goto end;
		}
		if (FAIL == conn->net->data->m.enable_ssl(conn->net TSRMLS_CC)) {
			goto end;
		}
	}
#endif
	ret = PASS;
end:
	PACKET_FREE(auth_packet);
	DBG_RETURN(ret);
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
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, connect);
	size_t host_len;
	zend_bool unix_socket = FALSE;
	zend_bool named_pipe = FALSE;
	zend_bool reconnect = FALSE;
	zend_bool saved_compression = FALSE;
	zend_bool local_tx_started = FALSE;
	MYSQLND_NET * net = conn->net;

	DBG_ENTER("mysqlnd_conn_data::connect");

	if (PASS != conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		goto err;
	}
	if (!db) {
		DBG_INF_FMT("no db given, using empty string");
		db = "";
		db_len = 0;
	} else {
		mysql_flags |= CLIENT_CONNECT_WITH_DB;	
	}
		if (!conn->scheme) {
			goto err; /* OOM */
		}
	}

	mysql_flags = conn->m->get_updated_connect_flags(conn, mysql_flags TSRMLS_CC);

	if (FAIL == conn->m->connect_handshake(conn, host, user, passwd, passwd_len, db, db_len, mysql_flags TSRMLS_CC)) {
		goto err;
	}

	{
		CONN_SET_STATE(conn, CONN_READY);

		if (saved_compression) {
			net->data->compressed = TRUE;
		}
			if (!conn->unix_socket || !conn->host_info) {
				SET_OOM_ERROR(*conn->error_info);
				goto err; /* OOM */
			}
			conn->unix_socket_len = strlen(conn->unix_socket);
		}
		conn->max_packet_size	= MYSQLND_ASSEMBLED_PACKET_MAX_SIZE;
		/* todo: check if charset is available */

		SET_EMPTY_ERROR(*conn->error_info);

		mysqlnd_local_infile_default(conn);

#if MYSQLND_UNICODE
		{
			unsigned int as_unicode = 1;
			conn->m->set_client_option(conn, MYSQLND_OPT_NUMERIC_AND_DATETIME_AS_UNICODE, (char *)&as_unicode TSRMLS_CC);
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
		if (conn->persistent) {
			MYSQLND_INC_CONN_STATISTIC_W_VALUE2(conn->stats, STAT_PCONNECT_SUCCESS, 1, STAT_OPENED_PERSISTENT_CONNECTIONS, 1);
		}

		DBG_INF_FMT("connection_id=%llu", conn->thread_id);

		conn->m->local_tx_end(conn, this_func, PASS TSRMLS_CC);
		DBG_RETURN(PASS);
	}
err:

	DBG_ERR_FMT("[%u] %.128s (trying to connect via %s)", conn->error_info->error_no, conn->error_info->error, conn->scheme);
	if (!conn->error_info->error_no) {
		SET_CLIENT_ERROR(*conn->error_info, CR_CONNECTION_ERROR, UNKNOWN_SQLSTATE, conn->error_info->error? conn->error_info->error:"Unknown error");
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "[%u] %.128s (trying to connect via %s)",
						 conn->error_info->error_no, conn->error_info->error, conn->scheme);
	}

	conn->m->free_contents(conn TSRMLS_CC);
	MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_CONNECT_FAILURE);
	if (TRUE == local_tx_started) {
		conn->m->local_tx_end(conn, this_func, FAIL TSRMLS_CC);
	}

	DBG_RETURN(FAIL);
}
/* }}} */


/* {{{ mysqlnd_conn::connect */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn, connect)(MYSQLND * conn_handle,
						 const char * host, const char * user,
						 const char * passwd, unsigned int passwd_len,
						 const char * db, unsigned int db_len,
						 unsigned int port,
						 const char * socket_or_pipe,
						 unsigned int mysql_flags
						 TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, connect);
	enum_func_status ret = FAIL;
	MYSQLND_CONN_DATA * conn = conn_handle->data;

	DBG_ENTER("mysqlnd_conn::connect");

	if (PASS == conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		ret = conn->m->connect(conn, host, user, passwd, passwd_len, db, db_len, port, socket_or_pipe, mysql_flags TSRMLS_CC);

		conn->m->local_tx_end(conn, this_func, FAIL TSRMLS_CC);
	}
	DBG_RETURN(ret);
}
/* }}} */

/* {{{ mysqlnd_connect */
PHPAPI MYSQLND * mysqlnd_connect(MYSQLND * conn_handle,
						 const char * host, const char * user,
						 const char * passwd, unsigned int passwd_len,
						 const char * db, unsigned int db_len,
						 unsigned int port,
						 const char * socket_or_pipe,
						 unsigned int mysql_flags
						 TSRMLS_DC)
{
	enum_func_status ret = FAIL;
	zend_bool self_alloced = FALSE;

	DBG_ENTER("mysqlnd_connect");
	DBG_INF_FMT("host=%s user=%s db=%s port=%u flags=%u", host?host:"", user?user:"", db?db:"", port, mysql_flags);

	if (!conn_handle) {
		self_alloced = TRUE;
		if (!(conn_handle = mysqlnd_init(FALSE))) {
			/* OOM */
			DBG_RETURN(NULL);
		}
	}

	ret = conn_handle->m->connect(conn_handle, host, user, passwd, passwd_len, db, db_len, port, socket_or_pipe, mysql_flags TSRMLS_CC);

	if (ret == FAIL) {
		if (self_alloced) {
			/*
			  We have alloced, thus there are no references to this
			  object - we are free to kill it!
			*/
			conn_handle->m->dtor(conn_handle TSRMLS_CC);
		}
		DBG_RETURN(NULL);
	}
	DBG_RETURN(conn_handle);
}
/* }}} */


/* {{{ mysqlnd_conn_data::query */
/*
  If conn->error_info->error_no is not zero, then we had an error.
  Still the result from the query is PASS
*/
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, query)(MYSQLND_CONN_DATA * conn, const char * query, unsigned int query_len TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, query);
	enum_func_status ret = FAIL;
	DBG_ENTER("mysqlnd_conn_data::query");
	DBG_INF_FMT("conn=%llu query=%s", conn->thread_id, query);

	if (PASS == conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		if (PASS == conn->m->send_query(conn, query, query_len TSRMLS_CC) &&
			PASS == conn->m->reap_query(conn TSRMLS_CC))
		{
			ret = PASS;
			if (conn->last_query_type == QUERY_UPSERT && conn->upsert_status->affected_rows) {
				MYSQLND_INC_CONN_STATISTIC_W_VALUE(conn->stats, STAT_ROWS_AFFECTED_NORMAL, conn->upsert_status->affected_rows);
			}
		{
			unsigned int as_unicode = 1;
			conn->m->set_client_option(conn, MYSQLND_OPT_NUMERIC_AND_DATETIME_AS_UNICODE, (char *)&as_unicode TSRMLS_CC);
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
		if (conn->persistent) {
			MYSQLND_INC_CONN_STATISTIC_W_VALUE2(conn->stats, STAT_PCONNECT_SUCCESS, 1, STAT_OPENED_PERSISTENT_CONNECTIONS, 1);
		}

		DBG_INF_FMT("connection_id=%llu", conn->thread_id);

		conn->m->local_tx_end(conn, this_func, PASS TSRMLS_CC);
		DBG_RETURN(PASS);
	}
err:

	DBG_ERR_FMT("[%u] %.128s (trying to connect via %s)", conn->error_info->error_no, conn->error_info->error, conn->scheme);
	if (!conn->error_info->error_no) {
		SET_CLIENT_ERROR(*conn->error_info, CR_CONNECTION_ERROR, UNKNOWN_SQLSTATE, conn->error_info->error? conn->error_info->error:"Unknown error");
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "[%u] %.128s (trying to connect via %s)",
						 conn->error_info->error_no, conn->error_info->error, conn->scheme);
	}

	conn->m->free_contents(conn TSRMLS_CC);
	MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_CONNECT_FAILURE);
	if (TRUE == local_tx_started) {
		conn->m->local_tx_end(conn, this_func, FAIL TSRMLS_CC);
	}

	DBG_RETURN(FAIL);
}
/* }}} */


/* {{{ mysqlnd_conn::connect */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn, connect)(MYSQLND * conn_handle,
						 const char * host, const char * user,
						 const char * passwd, unsigned int passwd_len,
						 const char * db, unsigned int db_len,
						 unsigned int port,
						 const char * socket_or_pipe,
						 unsigned int mysql_flags
						 TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, connect);
	enum_func_status ret = FAIL;
	MYSQLND_CONN_DATA * conn = conn_handle->data;

	DBG_ENTER("mysqlnd_conn::connect");

	if (PASS == conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		ret = conn->m->connect(conn, host, user, passwd, passwd_len, db, db_len, port, socket_or_pipe, mysql_flags TSRMLS_CC);

		conn->m->local_tx_end(conn, this_func, FAIL TSRMLS_CC);
	}
	DBG_RETURN(ret);
}
/* }}} */

/* {{{ mysqlnd_connect */
PHPAPI MYSQLND * mysqlnd_connect(MYSQLND * conn_handle,
						 const char * host, const char * user,
						 const char * passwd, unsigned int passwd_len,
						 const char * db, unsigned int db_len,
						 unsigned int port,
						 const char * socket_or_pipe,
						 unsigned int mysql_flags
						 TSRMLS_DC)
{
	enum_func_status ret = FAIL;
	zend_bool self_alloced = FALSE;

	DBG_ENTER("mysqlnd_connect");
	DBG_INF_FMT("host=%s user=%s db=%s port=%u flags=%u", host?host:"", user?user:"", db?db:"", port, mysql_flags);

	if (!conn_handle) {
		self_alloced = TRUE;
		if (!(conn_handle = mysqlnd_init(FALSE))) {
			/* OOM */
			DBG_RETURN(NULL);
		}
	}

	ret = conn_handle->m->connect(conn_handle, host, user, passwd, passwd_len, db, db_len, port, socket_or_pipe, mysql_flags TSRMLS_CC);

	if (ret == FAIL) {
		if (self_alloced) {
			/*
			  We have alloced, thus there are no references to this
			  object - we are free to kill it!
			*/
			conn_handle->m->dtor(conn_handle TSRMLS_CC);
		}
		DBG_RETURN(NULL);
	}
	DBG_RETURN(conn_handle);
}
/* }}} */


/* {{{ mysqlnd_conn_data::query */
/*
  If conn->error_info->error_no is not zero, then we had an error.
  Still the result from the query is PASS
*/
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, query)(MYSQLND_CONN_DATA * conn, const char * query, unsigned int query_len TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, query);
	enum_func_status ret = FAIL;
	DBG_ENTER("mysqlnd_conn_data::query");
	DBG_INF_FMT("conn=%llu query=%s", conn->thread_id, query);

	if (PASS == conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		if (PASS == conn->m->send_query(conn, query, query_len TSRMLS_CC) &&
			PASS == conn->m->reap_query(conn TSRMLS_CC))
		{
			ret = PASS;
			if (conn->last_query_type == QUERY_UPSERT && conn->upsert_status->affected_rows) {
				MYSQLND_INC_CONN_STATISTIC_W_VALUE(conn->stats, STAT_ROWS_AFFECTED_NORMAL, conn->upsert_status->affected_rows);
			}
		if (conn->persistent) {
			MYSQLND_INC_CONN_STATISTIC_W_VALUE2(conn->stats, STAT_PCONNECT_SUCCESS, 1, STAT_OPENED_PERSISTENT_CONNECTIONS, 1);
		}

		DBG_INF_FMT("connection_id=%llu", conn->thread_id);

		conn->m->local_tx_end(conn, this_func, PASS TSRMLS_CC);
		DBG_RETURN(PASS);
	}
err:

	DBG_ERR_FMT("[%u] %.128s (trying to connect via %s)", conn->error_info->error_no, conn->error_info->error, conn->scheme);
	if (!conn->error_info->error_no) {
		SET_CLIENT_ERROR(*conn->error_info, CR_CONNECTION_ERROR, UNKNOWN_SQLSTATE, conn->error_info->error? conn->error_info->error:"Unknown error");
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "[%u] %.128s (trying to connect via %s)",
						 conn->error_info->error_no, conn->error_info->error, conn->scheme);
	}

	conn->m->free_contents(conn TSRMLS_CC);
	MYSQLND_INC_CONN_STATISTIC(conn->stats, STAT_CONNECT_FAILURE);
	if (TRUE == local_tx_started) {
		conn->m->local_tx_end(conn, this_func, FAIL TSRMLS_CC);
	}

	DBG_RETURN(FAIL);
}
/* }}} */


/* {{{ mysqlnd_conn::connect */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn, connect)(MYSQLND * conn_handle,
						 const char * host, const char * user,
						 const char * passwd, unsigned int passwd_len,
						 const char * db, unsigned int db_len,
						 unsigned int port,
						 const char * socket_or_pipe,
						 unsigned int mysql_flags
						 TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, connect);
	enum_func_status ret = FAIL;
	MYSQLND_CONN_DATA * conn = conn_handle->data;

	DBG_ENTER("mysqlnd_conn::connect");

	if (PASS == conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		ret = conn->m->connect(conn, host, user, passwd, passwd_len, db, db_len, port, socket_or_pipe, mysql_flags TSRMLS_CC);

		conn->m->local_tx_end(conn, this_func, FAIL TSRMLS_CC);
	}
	DBG_RETURN(ret);
}
/* }}} */

/* {{{ mysqlnd_connect */
PHPAPI MYSQLND * mysqlnd_connect(MYSQLND * conn_handle,
						 const char * host, const char * user,
						 const char * passwd, unsigned int passwd_len,
						 const char * db, unsigned int db_len,
						 unsigned int port,
						 const char * socket_or_pipe,
						 unsigned int mysql_flags
						 TSRMLS_DC)
{
	enum_func_status ret = FAIL;
	zend_bool self_alloced = FALSE;

	DBG_ENTER("mysqlnd_connect");
	DBG_INF_FMT("host=%s user=%s db=%s port=%u flags=%u", host?host:"", user?user:"", db?db:"", port, mysql_flags);

	if (!conn_handle) {
		self_alloced = TRUE;
		if (!(conn_handle = mysqlnd_init(FALSE))) {
			/* OOM */
			DBG_RETURN(NULL);
		}
		} else if (PASS == (ret = conn->m->simple_command(conn, COM_PROCESS_KILL, buff, 4, PROT_LAST, FALSE, TRUE TSRMLS_CC))) {
			CONN_SET_STATE(conn, CONN_QUIT_SENT);
			conn->m->send_close(conn TSRMLS_CC);
		}
			if (net_stream) {
				ret = conn->m->simple_command(conn, COM_QUIT, NULL, 0, PROT_LAST, TRUE, TRUE TSRMLS_CC);
			}
			/* Do nothing */
			break;
		case CONN_SENDING_LOAD_DATA:
			/*
			  Don't send COM_QUIT if we are in a middle of a LOAD DATA or we
			  will crash (assert) a debug server.
			*/
		case CONN_NEXT_RESULT_PENDING:
		case CONN_QUERY_SENT:
		case CONN_FETCHING_DATA:
			MYSQLND_INC_GLOBAL_STATISTIC(STAT_CLOSE_IN_MIDDLE);
			DBG_ERR_FMT("Brutally closing connection [%p][%s]", conn, conn->scheme);
			/*
			  Do nothing, the connection will be brutally closed
			  and the server will catch it and free close from its side.
			*/
		case CONN_ALLOCED:
			/*
			  Allocated but not connected or there was failure when trying
			  to connect with pre-allocated connect.

			  Fall-through
			*/
		case CONN_QUIT_SENT:
			/* The user has killed its own connection */
			break;
	}
	/*
	  We hold one reference, and every other object which needs the
	  connection does increase it by 1.
	*/
	CONN_SET_STATE(conn, CONN_QUIT_SENT);
	net->data->m.close_stream(net, conn->stats, conn->error_info TSRMLS_CC);

	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_conn_data::get_reference */
static MYSQLND_CONN_DATA *
MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, get_reference)(MYSQLND_CONN_DATA * const conn TSRMLS_DC)
{
	DBG_ENTER("mysqlnd_conn_data::get_reference");
	++conn->refcount;
	DBG_INF_FMT("conn=%llu new_refcount=%u", conn->thread_id, conn->refcount);
	DBG_RETURN(conn);
}
/* }}} */


/* {{{ mysqlnd_conn_data::free_reference */
static enum_func_status
MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, free_reference)(MYSQLND_CONN_DATA * const conn TSRMLS_DC)
{
	enum_func_status ret = PASS;
	DBG_ENTER("mysqlnd_conn_data::free_reference");
	DBG_INF_FMT("conn=%llu old_refcount=%u", conn->thread_id, conn->refcount);
	if (!(--conn->refcount)) {
		/*
		  No multithreading issues as we don't share the connection :)
		  This will free the object too, of course because references has
		  reached zero.
		*/
		ret = conn->m->send_close(conn TSRMLS_CC);
		conn->m->dtor(conn TSRMLS_CC);
	}
				if (!conn->error_info->error_no) {
					DBG_ERR_FMT("Serious error. %s::%u", __FILE__, __LINE__);
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Serious error. PID=%d", getpid());
					CONN_SET_STATE(conn, CONN_QUIT_SENT);
					conn->m->send_close(conn TSRMLS_CC);
				} else {
					DBG_INF_FMT("Error from the server : (%u) %s", conn->error_info->error_no, conn->error_info->error);
				}
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

	if (!user) {
		user = "";
	}
	if (!passwd) {
		passwd = "";
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
	if (PASS != conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		goto end;
	}

	SET_EMPTY_ERROR(*conn->error_info);
	SET_ERROR_AFF_ROWS(conn);

	if (!user) {
		user = "";
	}
	if (!passwd) {
		passwd = "";
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


/* {{{ mysqlnd_conn_data::set_client_option */
static enum_func_status
MYSQLND_METHOD(mysqlnd_conn_data, set_client_option)(MYSQLND_CONN_DATA * const conn,
												enum mysqlnd_option option,
												const char * const value
												TSRMLS_DC)
{
	size_t this_func = STRUCT_OFFSET(struct st_mysqlnd_conn_data_methods, set_client_option);
	enum_func_status ret = PASS;
	DBG_ENTER("mysqlnd_conn_data::set_client_option");
	DBG_INF_FMT("conn=%llu option=%u", conn->thread_id, option);

	if (PASS != conn->m->local_tx_start(conn, this_func TSRMLS_CC)) {
		goto end;
	}
	switch (option) {
		case MYSQL_OPT_COMPRESS:
#ifdef WHEN_SUPPORTED_BY_MYSQLI
		case MYSQL_OPT_READ_TIMEOUT:
		case MYSQL_OPT_WRITE_TIMEOUT:
#endif
		case MYSQLND_OPT_SSL_KEY:
		case MYSQLND_OPT_SSL_CERT:
		case MYSQLND_OPT_SSL_CA:
		case MYSQLND_OPT_SSL_CAPATH:
		case MYSQLND_OPT_SSL_CIPHER:
		case MYSQL_OPT_SSL_VERIFY_SERVER_CERT:
		case MYSQL_OPT_CONNECT_TIMEOUT:
		case MYSQLND_OPT_NET_CMD_BUFFER_SIZE:
		case MYSQLND_OPT_NET_READ_BUFFER_SIZE:
			ret = conn->net->data->m.set_client_option(conn->net, option, value TSRMLS_CC);
			break;
#if MYSQLND_UNICODE
		case MYSQLND_OPT_NUMERIC_AND_DATETIME_AS_UNICODE:
			conn->options->numeric_and_datetime_as_unicode = *(unsigned int*) value;
			break;
#endif
#ifdef MYSQLND_STRING_TO_INT_CONVERSION
		case MYSQLND_OPT_INT_AND_FLOAT_NATIVE:
			conn->options->int_and_float_native = *(unsigned int*) value;
			break;
#endif
		case MYSQL_OPT_LOCAL_INFILE:
			if (!value || (*(unsigned int*) value) ? 1 : 0) {
				conn->options->flags |= CLIENT_LOCAL_FILES;
			} else {
				conn->options->flags &= ~CLIENT_LOCAL_FILES;
			}
			break;
		case MYSQL_INIT_COMMAND:
		{
			char ** new_init_commands;
			char * new_command;
			/* when num_commands is 0, then realloc will be effectively a malloc call, internally */
			/* Don't assign to conn->options->init_commands because in case of OOM we will lose the pointer and leak */
			new_init_commands = mnd_perealloc(conn->options->init_commands, sizeof(char *) * (conn->options->num_commands + 1), conn->persistent);
			if (!new_init_commands) {
				goto oom;
			}
			conn->options->init_commands = new_init_commands;
			new_command = mnd_pestrdup(value, conn->persistent);
			if (!new_command) {
				goto oom;
			}
			conn->options->init_commands[conn->options->num_commands] = new_command;
			++conn->options->num_commands;
			break;
		}
		case MYSQL_READ_DEFAULT_FILE:
		case MYSQL_READ_DEFAULT_GROUP:
#ifdef WHEN_SUPPORTED_BY_MYSQLI
		case MYSQL_SET_CLIENT_IP:
		case MYSQL_REPORT_DATA_TRUNCATION:
#endif
			/* currently not supported. Todo!! */
			break;
		case MYSQL_SET_CHARSET_NAME:
		{
			char * new_charset_name = mnd_pestrdup(value, conn->persistent);
			if (!new_charset_name) {
				goto oom;
			}
			if (conn->options->charset_name) {
				mnd_pefree(conn->options->charset_name, conn->persistent);
			}
			conn->options->charset_name = new_charset_name;
			DBG_INF_FMT("charset=%s", conn->options->charset_name);
			break;
		}
		case MYSQL_OPT_NAMED_PIPE:
			conn->options->protocol = MYSQL_PROTOCOL_PIPE;
			break;
		case MYSQL_OPT_PROTOCOL:
			if (*(unsigned int*) value < MYSQL_PROTOCOL_LAST) {
				conn->options->protocol = *(unsigned int*) value;
			}
			break;
#ifdef WHEN_SUPPORTED_BY_MYSQLI
		case MYSQL_SET_CHARSET_DIR:
		case MYSQL_OPT_RECONNECT:
			/* we don't need external character sets, all character sets are
			   compiled in. For compatibility we just ignore this setting.
			   Same for protocol, we don't support old protocol */
		case MYSQL_OPT_USE_REMOTE_CONNECTION:
		case MYSQL_OPT_USE_EMBEDDED_CONNECTION:
		case MYSQL_OPT_GUESS_CONNECTION:
			/* todo: throw an error, we don't support embedded */
			break;
#endif
		case MYSQLND_OPT_MAX_ALLOWED_PACKET:
			if (*(unsigned int*) value > (1<<16)) {
				conn->options->max_allowed_packet = *(unsigned int*) value;
			}
			break;
		case MYSQLND_OPT_AUTH_PROTOCOL:
		{
			char * new_auth_protocol = value? mnd_pestrdup(value, conn->persistent) : NULL;
			if (value && !new_auth_protocol) {
				goto oom;
			}
			if (conn->options->auth_protocol) {
				mnd_pefree(conn->options->auth_protocol, conn->persistent);
			}
			conn->options->auth_protocol = new_auth_protocol;
			DBG_INF_FMT("auth_protocol=%s", conn->options->auth_protocol);
			break;
		}
#ifdef WHEN_SUPPORTED_BY_MYSQLI
		case MYSQL_SHARED_MEMORY_BASE_NAME:
		case MYSQL_OPT_USE_RESULT:
		case MYSQL_SECURE_AUTH:
			/* not sure, todo ? */
#endif
		default:
			ret = FAIL;
	}
	if (!db) {
		db = "";

	}
{
	DBG_ENTER("mysqlnd_conn_data::init");
	mysqlnd_stats_init(&conn->stats, STAT_LAST);
	SET_ERROR_AFF_ROWS(conn);

	conn->net = mysqlnd_net_init(conn->persistent, conn->stats, conn->error_info TSRMLS_CC);
	conn->protocol = mysqlnd_protocol_init(conn->persistent TSRMLS_CC);

	DBG_RETURN(conn->stats && conn->net && conn->protocol? PASS:FAIL);
}
/* }}} */


MYSQLND_STMT * _mysqlnd_stmt_init(MYSQLND_CONN_DATA * const conn TSRMLS_DC);


MYSQLND_CLASS_METHODS_START(mysqlnd_conn_data)
	MYSQLND_METHOD(mysqlnd_conn_data, init),
	MYSQLND_METHOD(mysqlnd_conn_data, connect),

	MYSQLND_METHOD(mysqlnd_conn_data, escape_string),
	MYSQLND_METHOD(mysqlnd_conn_data, set_charset),
	MYSQLND_METHOD(mysqlnd_conn_data, query),
	MYSQLND_METHOD(mysqlnd_conn_data, send_query),
	MYSQLND_METHOD(mysqlnd_conn_data, reap_query),
	MYSQLND_METHOD(mysqlnd_conn_data, use_result),
	MYSQLND_METHOD(mysqlnd_conn_data, store_result),
	MYSQLND_METHOD(mysqlnd_conn_data, next_result),
	MYSQLND_METHOD(mysqlnd_conn_data, more_results),

	_mysqlnd_stmt_init,

	MYSQLND_METHOD(mysqlnd_conn_data, shutdown),
	MYSQLND_METHOD(mysqlnd_conn_data, refresh),

	MYSQLND_METHOD(mysqlnd_conn_data, ping),
	MYSQLND_METHOD(mysqlnd_conn_data, kill),
	MYSQLND_METHOD(mysqlnd_conn_data, select_db),
	MYSQLND_METHOD(mysqlnd_conn_data, dump_debug_info),
	MYSQLND_METHOD(mysqlnd_conn_data, change_user),

	MYSQLND_METHOD(mysqlnd_conn_data, errno),
	MYSQLND_METHOD(mysqlnd_conn_data, error),
	MYSQLND_METHOD(mysqlnd_conn_data, sqlstate),
	MYSQLND_METHOD(mysqlnd_conn_data, thread_id),

	MYSQLND_METHOD(mysqlnd_conn_data, get_connection_stats),

	MYSQLND_METHOD(mysqlnd_conn_data, get_server_version),
	MYSQLND_METHOD(mysqlnd_conn_data, get_server_info),
	MYSQLND_METHOD(mysqlnd_conn_data, statistic),
	MYSQLND_METHOD(mysqlnd_conn_data, get_host_info),
	MYSQLND_METHOD(mysqlnd_conn_data, get_proto_info),
	MYSQLND_METHOD(mysqlnd_conn_data, info),
	MYSQLND_METHOD(mysqlnd_conn_data, charset_name),
	MYSQLND_METHOD(mysqlnd_conn_data, list_fields),
	MYSQLND_METHOD(mysqlnd_conn_data, list_method),

	MYSQLND_METHOD(mysqlnd_conn_data, insert_id),
	MYSQLND_METHOD(mysqlnd_conn_data, affected_rows),
	MYSQLND_METHOD(mysqlnd_conn_data, warning_count),
	MYSQLND_METHOD(mysqlnd_conn_data, field_count),

	MYSQLND_METHOD(mysqlnd_conn_data, server_status),

	MYSQLND_METHOD(mysqlnd_conn_data, set_server_option),
	MYSQLND_METHOD(mysqlnd_conn_data, set_client_option),
	MYSQLND_METHOD(mysqlnd_conn_data, free_contents),
	MYSQLND_METHOD(mysqlnd_conn_data, free_options),

	MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, dtor),

	mysqlnd_query_read_result_set_header,

	MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, get_reference),
	MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, free_reference),
	MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, get_state),
	MYSQLND_METHOD_PRIVATE(mysqlnd_conn_data, set_state),

	MYSQLND_METHOD(mysqlnd_conn_data, simple_command),
	MYSQLND_METHOD(mysqlnd_conn_data, simple_command_handle_response),
	MYSQLND_METHOD(mysqlnd_conn_data, restart_psession),
	MYSQLND_METHOD(mysqlnd_conn_data, end_psession),
	MYSQLND_METHOD(mysqlnd_conn_data, send_close),

	MYSQLND_METHOD(mysqlnd_conn_data, ssl_set),
	mysqlnd_result_init,
	MYSQLND_METHOD(mysqlnd_conn_data, set_autocommit),
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
static MYSQLND *
MYSQLND_METHOD(mysqlnd_conn, clone_object)(MYSQLND * const conn TSRMLS_DC)
{
	MYSQLND * ret;
	DBG_ENTER("mysqlnd_conn::get_reference");
	ret = MYSQLND_CLASS_METHOD_TABLE_NAME(mysqlnd_object_factory).clone_connection_object(conn TSRMLS_CC);
	DBG_RETURN(ret);
}