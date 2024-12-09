
typedef enum_func_status	(*func_mysqlnd_conn_data__local_tx_start)(MYSQLND_CONN_DATA * conn, size_t this_func TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__local_tx_end)(MYSQLND_CONN_DATA * conn, size_t this_func, enum_func_status status TSRMLS_DC);


struct st_mysqlnd_conn_data_methods
{
	func_mysqlnd_conn_data__init init;

	func_mysqlnd_conn_data__local_tx_start local_tx_start;
	func_mysqlnd_conn_data__local_tx_end local_tx_end;
};


typedef enum_func_status	(*func_mysqlnd_data__connect)(MYSQLND * conn, const char * host, const char * user, const char * passwd, unsigned int passwd_len, const char * db, unsigned int db_len, unsigned int port, const char * socket_or_pipe, unsigned int mysql_flags TSRMLS_DC);