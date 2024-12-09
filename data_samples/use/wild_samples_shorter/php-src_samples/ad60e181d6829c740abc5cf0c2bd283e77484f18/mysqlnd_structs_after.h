
typedef enum_func_status	(*func_mysqlnd_conn_data__local_tx_start)(MYSQLND_CONN_DATA * conn, size_t this_func TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__local_tx_end)(MYSQLND_CONN_DATA * conn, size_t this_func, enum_func_status status TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__execute_init_commands)(MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_conn_data__get_updated_connect_flags)(MYSQLND_CONN_DATA * conn, unsigned int mysql_flags TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__connect_handshake)(MYSQLND_CONN_DATA * conn, const char * const host, const char * const user, const char * const passwd, const unsigned int passwd_len, const char * const db, const unsigned int db_len, const unsigned int mysql_flags TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__simple_command_send_request)(MYSQLND_CONN_DATA * conn, enum php_mysqlnd_server_command command, const zend_uchar * const arg, size_t arg_len, zend_bool silent, zend_bool ignore_upsert_status TSRMLS_DC);
typedef struct st_mysqlnd_authentication_plugin * (*func_mysqlnd_conn_data__fetch_auth_plugin_by_name)(const char * const requested_protocol TSRMLS_DC);

struct st_mysqlnd_conn_data_methods
{
	func_mysqlnd_conn_data__init init;

	func_mysqlnd_conn_data__local_tx_start local_tx_start;
	func_mysqlnd_conn_data__local_tx_end local_tx_end;

	func_mysqlnd_conn_data__execute_init_commands execute_init_commands;
	func_mysqlnd_conn_data__get_updated_connect_flags get_updated_connect_flags;
	func_mysqlnd_conn_data__connect_handshake connect_handshake;
	func_mysqlnd_conn_data__simple_command_send_request simple_command_send_request;
	func_mysqlnd_conn_data__fetch_auth_plugin_by_name fetch_auth_plugin_by_name;
};


typedef enum_func_status	(*func_mysqlnd_data__connect)(MYSQLND * conn, const char * host, const char * user, const char * passwd, unsigned int passwd_len, const char * db, unsigned int db_len, unsigned int port, const char * socket_or_pipe, unsigned int mysql_flags TSRMLS_DC);