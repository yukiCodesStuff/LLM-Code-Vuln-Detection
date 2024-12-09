{
	func_mysqlnd_object_factory__get_connection get_connection;
	func_mysqlnd_object_factory__clone_connection_object clone_connection_object;
	func_mysqlnd_object_factory__get_prepared_statement get_prepared_statement;
	func_mysqlnd_object_factory__get_io_channel get_io_channel;
	func_mysqlnd_object_factory__get_protocol_decoder get_protocol_decoder;
};


typedef enum_func_status	(*func_mysqlnd_conn_data__init)(MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__connect)(MYSQLND_CONN_DATA * conn, const char * host, const char * user, const char * passwd, unsigned int passwd_len, const char * db, unsigned int db_len, unsigned int port, const char * socket_or_pipe, unsigned int mysql_flags TSRMLS_DC);
typedef ulong				(*func_mysqlnd_conn_data__escape_string)(MYSQLND_CONN_DATA * const conn, char *newstr, const char *escapestr, size_t escapestr_len TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__set_charset)(MYSQLND_CONN_DATA * const conn, const char * const charset TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__query)(MYSQLND_CONN_DATA * conn, const char * query, unsigned int query_len TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__send_query)(MYSQLND_CONN_DATA * conn, const char *query, unsigned int query_len TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__reap_query)(MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_conn_data__use_result)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_conn_data__store_result)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__next_result)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef zend_bool			(*func_mysqlnd_conn_data__more_results)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);

typedef MYSQLND_STMT *		(*func_mysqlnd_conn_data__stmt_init)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_conn_data__shutdown_server)(MYSQLND_CONN_DATA * const conn, uint8_t level TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__refresh_server)(MYSQLND_CONN_DATA * const conn, uint8_t options TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_conn_data__ping)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__kill_connection)(MYSQLND_CONN_DATA * conn, unsigned int pid TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__select_db)(MYSQLND_CONN_DATA * const conn, const char * const db, unsigned int db_len TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__server_dump_debug_information)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__change_user)(MYSQLND_CONN_DATA * const conn, const char * user, const char * passwd, const char * db, zend_bool silent, size_t passwd_len TSRMLS_DC);

typedef unsigned int		(*func_mysqlnd_conn_data__get_error_no)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef const char *		(*func_mysqlnd_conn_data__get_error_str)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef const char *		(*func_mysqlnd_conn_data__get_sqlstate)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_conn_data__get_thread_id)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef void				(*func_mysqlnd_conn_data__get_statistics)(const MYSQLND_CONN_DATA * const conn, zval *return_value TSRMLS_DC ZEND_FILE_LINE_DC);

typedef unsigned long		(*func_mysqlnd_conn_data__get_server_version)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef const char *		(*func_mysqlnd_conn_data__get_server_information)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__get_server_statistics)(MYSQLND_CONN_DATA * conn, char **message, unsigned int * message_len TSRMLS_DC);
typedef const char *		(*func_mysqlnd_conn_data__get_host_information)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_conn_data__get_protocol_information)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef const char *		(*func_mysqlnd_conn_data__get_last_message)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef const char *		(*func_mysqlnd_conn_data__charset_name)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_conn_data__list_fields)(MYSQLND_CONN_DATA * conn, const char * table, const char * achtung_wild TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_conn_data__list_method)(MYSQLND_CONN_DATA * conn, const char * query, const char * achtung_wild, char *par1 TSRMLS_DC);

typedef uint64_t			(*func_mysqlnd_conn_data__get_last_insert_id)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_conn_data__get_affected_rows)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_conn_data__get_warning_count)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);

typedef unsigned int		(*func_mysqlnd_conn_data__get_field_count)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);

typedef unsigned int		(*func_mysqlnd_conn_data__get_server_status)(const MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__set_server_option)(MYSQLND_CONN_DATA * const conn, enum_mysqlnd_server_option option TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__set_client_option)(MYSQLND_CONN_DATA * const conn, enum_mysqlnd_option option, const char * const value TSRMLS_DC);
typedef void				(*func_mysqlnd_conn_data__free_contents)(MYSQLND_CONN_DATA * conn TSRMLS_DC);/* private */
typedef void				(*func_mysqlnd_conn_data__free_options)(MYSQLND_CONN_DATA * conn TSRMLS_DC);	/* private */
typedef void				(*func_mysqlnd_conn_data__dtor)(MYSQLND_CONN_DATA * conn TSRMLS_DC);	/* private */

typedef enum_func_status	(*func_mysqlnd_conn_data__query_read_result_set_header)(MYSQLND_CONN_DATA * conn, MYSQLND_STMT * stmt TSRMLS_DC);

typedef MYSQLND_CONN_DATA *	(*func_mysqlnd_conn_data__get_reference)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__free_reference)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef enum mysqlnd_connection_state (*func_mysqlnd_conn_data__get_state)(MYSQLND_CONN_DATA * const conn TSRMLS_DC);
typedef void				(*func_mysqlnd_conn_data__set_state)(MYSQLND_CONN_DATA * const conn, enum mysqlnd_connection_state new_state TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_conn_data__simple_command)(MYSQLND_CONN_DATA * conn, enum php_mysqlnd_server_command command, const zend_uchar * const arg, size_t arg_len, enum mysqlnd_packet_type ok_packet, zend_bool silent, zend_bool ignore_upsert_status TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__simple_command_handle_response)(MYSQLND_CONN_DATA * conn, enum mysqlnd_packet_type ok_packet, zend_bool silent, enum php_mysqlnd_server_command command, zend_bool ignore_upsert_status TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_conn_data__restart_psession)(MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__end_psession)(MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__send_close)(MYSQLND_CONN_DATA * conn TSRMLS_DC);

typedef enum_func_status    (*func_mysqlnd_conn_data__ssl_set)(MYSQLND_CONN_DATA * const conn, const char * key, const char * const cert, const char * const ca, const char * const capath, const char * const cipher TSRMLS_DC);

typedef MYSQLND_RES * 		(*func_mysqlnd_conn_data__result_init)(unsigned int field_count, zend_bool persistent TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_conn_data__set_autocommit)(MYSQLND_CONN_DATA * conn, unsigned int mode TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__tx_commit)(MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__tx_rollback)(MYSQLND_CONN_DATA * conn TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_conn_data__local_tx_start)(MYSQLND_CONN_DATA * conn, size_t this_func TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn_data__local_tx_end)(MYSQLND_CONN_DATA * conn, size_t this_func, enum_func_status status TSRMLS_DC);


struct st_mysqlnd_conn_data_methods
{
	func_mysqlnd_conn_data__init init;
	func_mysqlnd_conn_data__connect connect;
	func_mysqlnd_conn_data__escape_string escape_string;
	func_mysqlnd_conn_data__set_charset set_charset;
	func_mysqlnd_conn_data__query query;
	func_mysqlnd_conn_data__send_query send_query;
	func_mysqlnd_conn_data__reap_query reap_query;
	func_mysqlnd_conn_data__use_result use_result;
	func_mysqlnd_conn_data__store_result store_result;
	func_mysqlnd_conn_data__next_result next_result;
	func_mysqlnd_conn_data__more_results more_results;

	func_mysqlnd_conn_data__stmt_init stmt_init;

	func_mysqlnd_conn_data__shutdown_server shutdown_server;
	func_mysqlnd_conn_data__refresh_server refresh_server;

	func_mysqlnd_conn_data__ping ping;
	func_mysqlnd_conn_data__kill_connection kill_connection;
	func_mysqlnd_conn_data__select_db select_db;
	func_mysqlnd_conn_data__server_dump_debug_information server_dump_debug_information;
	func_mysqlnd_conn_data__change_user change_user;

	func_mysqlnd_conn_data__get_error_no get_error_no;
	func_mysqlnd_conn_data__get_error_str get_error_str;
	func_mysqlnd_conn_data__get_sqlstate get_sqlstate;
	func_mysqlnd_conn_data__get_thread_id get_thread_id;
	func_mysqlnd_conn_data__get_statistics get_statistics;

	func_mysqlnd_conn_data__get_server_version get_server_version;
	func_mysqlnd_conn_data__get_server_information get_server_information;
	func_mysqlnd_conn_data__get_server_statistics get_server_statistics;
	func_mysqlnd_conn_data__get_host_information get_host_information;
	func_mysqlnd_conn_data__get_protocol_information get_protocol_information;
	func_mysqlnd_conn_data__get_last_message get_last_message;
	func_mysqlnd_conn_data__charset_name charset_name;
	func_mysqlnd_conn_data__list_fields list_fields;
	func_mysqlnd_conn_data__list_method list_method;

	func_mysqlnd_conn_data__get_last_insert_id get_last_insert_id;
	func_mysqlnd_conn_data__get_affected_rows get_affected_rows;
	func_mysqlnd_conn_data__get_warning_count get_warning_count;

	func_mysqlnd_conn_data__get_field_count get_field_count;

	func_mysqlnd_conn_data__get_server_status get_server_status;
	
	func_mysqlnd_conn_data__set_server_option set_server_option;
	func_mysqlnd_conn_data__set_client_option set_client_option;
	func_mysqlnd_conn_data__free_contents free_contents;
	func_mysqlnd_conn_data__free_options free_options;
	func_mysqlnd_conn_data__dtor dtor;

	func_mysqlnd_conn_data__query_read_result_set_header query_read_result_set_header;

	func_mysqlnd_conn_data__get_reference get_reference;
	func_mysqlnd_conn_data__free_reference free_reference;
	func_mysqlnd_conn_data__get_state get_state;
	func_mysqlnd_conn_data__set_state set_state;

	func_mysqlnd_conn_data__simple_command simple_command;
	func_mysqlnd_conn_data__simple_command_handle_response simple_command_handle_response;

	func_mysqlnd_conn_data__restart_psession restart_psession;
	func_mysqlnd_conn_data__end_psession end_psession;
	func_mysqlnd_conn_data__send_close send_close;

	func_mysqlnd_conn_data__ssl_set ssl_set;

	func_mysqlnd_conn_data__result_init result_init;
	func_mysqlnd_conn_data__set_autocommit set_autocommit;
	func_mysqlnd_conn_data__tx_commit tx_commit;
	func_mysqlnd_conn_data__tx_rollback tx_rollback;

	func_mysqlnd_conn_data__local_tx_start local_tx_start;
	func_mysqlnd_conn_data__local_tx_end local_tx_end;
};


typedef enum_func_status	(*func_mysqlnd_data__connect)(MYSQLND * conn, const char * host, const char * user, const char * passwd, unsigned int passwd_len, const char * db, unsigned int db_len, unsigned int port, const char * socket_or_pipe, unsigned int mysql_flags TSRMLS_DC);
typedef MYSQLND *			(*func_mysqlnd_conn__clone_object)(MYSQLND * const conn TSRMLS_DC);
typedef void				(*func_mysqlnd_conn__dtor)(MYSQLND * conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn__close)(MYSQLND * conn, enum_connection_close_type close_type TSRMLS_DC);

struct st_mysqlnd_conn_methods
{
	func_mysqlnd_data__connect connect;
	func_mysqlnd_conn__clone_object clone_object;
	func_mysqlnd_conn__dtor dtor;
	func_mysqlnd_conn__close close;
};


typedef mysqlnd_fetch_row_func	fetch_row;
typedef mysqlnd_fetch_row_func	fetch_row_normal_buffered; /* private */
typedef mysqlnd_fetch_row_func	fetch_row_normal_unbuffered; /* private */

typedef MYSQLND_RES *		(*func_mysqlnd_res__use_result)(MYSQLND_RES * const result, zend_bool ps_protocol TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_res__store_result)(MYSQLND_RES * result, MYSQLND_CONN_DATA * const conn, zend_bool ps TSRMLS_DC);
typedef void 				(*func_mysqlnd_res__fetch_into)(MYSQLND_RES *result, unsigned int flags, zval *return_value, enum_mysqlnd_extension ext TSRMLS_DC ZEND_FILE_LINE_DC);
typedef MYSQLND_ROW_C 		(*func_mysqlnd_res__fetch_row_c)(MYSQLND_RES *result TSRMLS_DC);
typedef void 				(*func_mysqlnd_res__fetch_all)(MYSQLND_RES *result, unsigned int flags, zval *return_value TSRMLS_DC ZEND_FILE_LINE_DC);
typedef void 				(*func_mysqlnd_res__fetch_field_data)(MYSQLND_RES *result, unsigned int offset, zval *return_value TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_res__num_rows)(const MYSQLND_RES * const result TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_res__num_fields)(const MYSQLND_RES * const result TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_res__skip_result)(MYSQLND_RES * const result TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_res__seek_data)(MYSQLND_RES * result, uint64_t row TSRMLS_DC);
typedef MYSQLND_FIELD_OFFSET (*func_mysqlnd_res__seek_field)(MYSQLND_RES * const result, MYSQLND_FIELD_OFFSET field_offset TSRMLS_DC);
typedef MYSQLND_FIELD_OFFSET (*func_mysqlnd_res__field_tell)(const MYSQLND_RES * const result TSRMLS_DC);
typedef const MYSQLND_FIELD *(*func_mysqlnd_res__fetch_field)(MYSQLND_RES * const result TSRMLS_DC);
typedef const MYSQLND_FIELD *(*func_mysqlnd_res__fetch_field_direct)(MYSQLND_RES * const result, MYSQLND_FIELD_OFFSET fieldnr TSRMLS_DC);
typedef const MYSQLND_FIELD *(*func_mysqlnd_res__fetch_fields)(MYSQLND_RES * const result TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_res__read_result_metadata)(MYSQLND_RES * result, MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef unsigned long *		(*func_mysqlnd_res__fetch_lengths)(MYSQLND_RES * const result TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_res__store_result_fetch_data)(MYSQLND_CONN_DATA * const conn, MYSQLND_RES * result, MYSQLND_RES_METADATA *meta, zend_bool binary_protocol TSRMLS_DC);
typedef enum_func_status 	(*func_mysqlnd_res__initialize_result_set_rest)(MYSQLND_RES * const result TSRMLS_DC);

typedef void				(*func_mysqlnd_res__free_result_buffers)(MYSQLND_RES * result TSRMLS_DC);	/* private */
typedef enum_func_status	(*func_mysqlnd_res__free_result)(MYSQLND_RES * result, zend_bool implicit TSRMLS_DC);
typedef void				(*func_mysqlnd_res__free_result_internal)(MYSQLND_RES *result TSRMLS_DC);
typedef void				(*func_mysqlnd_res__free_result_contents)(MYSQLND_RES *result TSRMLS_DC);
typedef void				(*func_mysqlnd_res__free_buffered_data)(MYSQLND_RES *result TSRMLS_DC);
typedef void				(*func_mysqlnd_res__unbuffered_free_last_data)(MYSQLND_RES *result TSRMLS_DC);

	/* for decoding - binary or text protocol */
typedef enum_func_status	(*func_mysqlnd_res__row_decoder)(MYSQLND_MEMORY_POOL_CHUNK * row_buffer, zval ** fields,
									unsigned int field_count, MYSQLND_FIELD *fields_metadata,
									zend_bool as_unicode, zend_bool as_int_or_float,
									MYSQLND_STATS * stats TSRMLS_DC);

typedef MYSQLND_RES_METADATA * (*func_mysqlnd_res__result_meta_init)(unsigned int field_count, zend_bool persistent TSRMLS_DC);

struct st_mysqlnd_res_methods
{
	mysqlnd_fetch_row_func	fetch_row;
	mysqlnd_fetch_row_func	fetch_row_normal_buffered; /* private */
	mysqlnd_fetch_row_func	fetch_row_normal_unbuffered; /* private */

	func_mysqlnd_res__use_result use_result;
	func_mysqlnd_res__store_result store_result;
	func_mysqlnd_res__fetch_into fetch_into;
	func_mysqlnd_res__fetch_row_c fetch_row_c;
	func_mysqlnd_res__fetch_all fetch_all;
	func_mysqlnd_res__fetch_field_data fetch_field_data;
	func_mysqlnd_res__num_rows num_rows;
	func_mysqlnd_res__num_fields num_fields;
	func_mysqlnd_res__skip_result skip_result;
	func_mysqlnd_res__seek_data seek_data;
	func_mysqlnd_res__seek_field seek_field;
	func_mysqlnd_res__field_tell field_tell;
	func_mysqlnd_res__fetch_field fetch_field;
	func_mysqlnd_res__fetch_field_direct fetch_field_direct;
	func_mysqlnd_res__fetch_fields fetch_fields;
	func_mysqlnd_res__read_result_metadata read_result_metadata;
	func_mysqlnd_res__fetch_lengths fetch_lengths;
	func_mysqlnd_res__store_result_fetch_data store_result_fetch_data;
	func_mysqlnd_res__initialize_result_set_rest initialize_result_set_rest;
	func_mysqlnd_res__free_result_buffers free_result_buffers;
	func_mysqlnd_res__free_result free_result;
	func_mysqlnd_res__free_result_internal free_result_internal;
	func_mysqlnd_res__free_result_contents free_result_contents;
	func_mysqlnd_res__free_buffered_data free_buffered_data;
	func_mysqlnd_res__unbuffered_free_last_data unbuffered_free_last_data;

	/* for decoding - binary or text protocol */
	func_mysqlnd_res__row_decoder row_decoder;

	func_mysqlnd_res__result_meta_init result_meta_init;

	void * unused1;
	void * unused2;
	void * unused3;
	void * unused4;
	void * unused5;
};


typedef const MYSQLND_FIELD *	(*func_mysqlnd_res_meta__fetch_field)(MYSQLND_RES_METADATA * const meta TSRMLS_DC);
typedef const MYSQLND_FIELD *	(*func_mysqlnd_res_meta__fetch_field_direct)(const MYSQLND_RES_METADATA * const meta, MYSQLND_FIELD_OFFSET fieldnr TSRMLS_DC);
typedef const MYSQLND_FIELD *	(*func_mysqlnd_res_meta__fetch_fields)(MYSQLND_RES_METADATA * const meta TSRMLS_DC);
typedef MYSQLND_FIELD_OFFSET	(*func_mysqlnd_res_meta__field_tell)(const MYSQLND_RES_METADATA * const meta TSRMLS_DC);
typedef enum_func_status		(*func_mysqlnd_res_meta__read_metadata)(MYSQLND_RES_METADATA * const meta, MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef MYSQLND_RES_METADATA *	(*func_mysqlnd_res_meta__clone_metadata)(const MYSQLND_RES_METADATA * const meta, zend_bool persistent TSRMLS_DC);
typedef void					(*func_mysqlnd_res_meta__free_metadata)(MYSQLND_RES_METADATA * meta TSRMLS_DC);

struct st_mysqlnd_res_meta_methods
{
	func_mysqlnd_res_meta__fetch_field fetch_field;
	func_mysqlnd_res_meta__fetch_field_direct fetch_field_direct;
	func_mysqlnd_res_meta__fetch_fields fetch_fields;
	func_mysqlnd_res_meta__field_tell field_tell;
	func_mysqlnd_res_meta__read_metadata read_metadata;
	func_mysqlnd_res_meta__clone_metadata clone_metadata;
	func_mysqlnd_res_meta__free_metadata free_metadata;
};


typedef enum_func_status	(*func_mysqlnd_stmt__prepare)(MYSQLND_STMT * const stmt, const char * const query, unsigned int query_len TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__execute)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__use_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__store_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__get_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef zend_bool			(*func_mysqlnd_stmt__more_results)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__next_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__free_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__seek_data)(const MYSQLND_STMT * const stmt, uint64_t row TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__reset)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__net_close)(MYSQLND_STMT * const stmt, zend_bool implicit TSRMLS_DC); /* private */
typedef enum_func_status	(*func_mysqlnd_stmt__dtor)(MYSQLND_STMT * const stmt, zend_bool implicit TSRMLS_DC); /* use this for mysqlnd_stmt_close */
typedef enum_func_status	(*func_mysqlnd_stmt__fetch)(MYSQLND_STMT * const stmt, zend_bool * const fetched_anything TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_parameters)(MYSQLND_STMT * const stmt, MYSQLND_PARAM_BIND * const param_bind TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_one_parameter)(MYSQLND_STMT * const stmt, unsigned int param_no, zval * const zv, zend_uchar	type TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__refresh_bind_param)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_result)(MYSQLND_STMT * const stmt, MYSQLND_RESULT_BIND * const result_bind TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_one_result)(MYSQLND_STMT * const stmt, unsigned int param_no TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__send_long_data)(MYSQLND_STMT * const stmt, unsigned int param_num, const char * const data, unsigned long length TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__get_parameter_metadata)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__get_result_metadata)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_stmt__get_last_insert_id)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_stmt__get_affected_rows)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_stmt__get_num_rows)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_param_count)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_field_count)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_warning_count)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_error_no)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef const char *		(*func_mysqlnd_stmt__get_error_str)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef const char *		(*func_mysqlnd_stmt__get_sqlstate)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__get_attribute)(const MYSQLND_STMT * const stmt, enum mysqlnd_stmt_attr attr_type, void * const value TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__set_attribute)(MYSQLND_STMT * const stmt, enum mysqlnd_stmt_attr attr_type, const void * const value TSRMLS_DC);
typedef MYSQLND_PARAM_BIND *(*func_mysqlnd_stmt__alloc_param_bind)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RESULT_BIND*(*func_mysqlnd_stmt__alloc_result_bind)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef	void 				(*func_mysqlnd_stmt__free_parameter_bind)(MYSQLND_STMT * const stmt, MYSQLND_PARAM_BIND * TSRMLS_DC);
typedef	void 				(*func_mysqlnd_stmt__free_result_bind)(MYSQLND_STMT * const stmt, MYSQLND_RESULT_BIND * TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__server_status)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status 	(*func_mysqlnd_stmt__generate_execute_request)(MYSQLND_STMT * const s, zend_uchar ** request, size_t *request_len, zend_bool * free_buffer TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__parse_execute_response)(MYSQLND_STMT * const s TSRMLS_DC);
typedef void 				(*func_mysqlnd_stmt__free_stmt_content)(MYSQLND_STMT * const s TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__flush)(MYSQLND_STMT * const stmt TSRMLS_DC);

struct st_mysqlnd_stmt_methods
{
	func_mysqlnd_stmt__prepare prepare;
	func_mysqlnd_stmt__execute execute;
	func_mysqlnd_stmt__use_result use_result;
	func_mysqlnd_stmt__store_result store_result;
	func_mysqlnd_stmt__get_result get_result;
	func_mysqlnd_stmt__more_results more_results;
	func_mysqlnd_stmt__next_result next_result;
	func_mysqlnd_stmt__free_result free_result;
	func_mysqlnd_stmt__seek_data seek_data;
	func_mysqlnd_stmt__reset reset;
	func_mysqlnd_stmt__net_close net_close;
	func_mysqlnd_stmt__dtor dtor;
	func_mysqlnd_stmt__fetch fetch;

	func_mysqlnd_stmt__bind_parameters bind_parameters;
	func_mysqlnd_stmt__bind_one_parameter bind_one_parameter;
	func_mysqlnd_stmt__refresh_bind_param refresh_bind_param;
	func_mysqlnd_stmt__bind_result bind_result;
	func_mysqlnd_stmt__bind_one_result bind_one_result;
	func_mysqlnd_stmt__send_long_data send_long_data;
	func_mysqlnd_stmt__get_parameter_metadata get_parameter_metadata;
	func_mysqlnd_stmt__get_result_metadata get_result_metadata;

	func_mysqlnd_stmt__get_last_insert_id get_last_insert_id;
	func_mysqlnd_stmt__get_affected_rows get_affected_rows;
	func_mysqlnd_stmt__get_num_rows get_num_rows;

	func_mysqlnd_stmt__get_param_count get_param_count;
	func_mysqlnd_stmt__get_field_count get_field_count;
	func_mysqlnd_stmt__get_warning_count get_warning_count;

	func_mysqlnd_stmt__get_error_no get_error_no;
	func_mysqlnd_stmt__get_error_str get_error_str;
	func_mysqlnd_stmt__get_sqlstate get_sqlstate;

	func_mysqlnd_stmt__get_attribute get_attribute;
	func_mysqlnd_stmt__set_attribute set_attribute;

	func_mysqlnd_stmt__alloc_param_bind alloc_parameter_bind;
	func_mysqlnd_stmt__alloc_result_bind alloc_result_bind;

	func_mysqlnd_stmt__free_parameter_bind free_parameter_bind;
	func_mysqlnd_stmt__free_result_bind free_result_bind;

	func_mysqlnd_stmt__server_status get_server_status;

	func_mysqlnd_stmt__generate_execute_request generate_execute_request;
	func_mysqlnd_stmt__parse_execute_response parse_execute_response;

	func_mysqlnd_stmt__free_stmt_content free_stmt_content;

	func_mysqlnd_stmt__flush flush;
};


struct st_mysqlnd_net_data
{
	php_stream			*stream;
	zend_bool			compressed;
#ifdef MYSQLND_DO_WIRE_CHECK_BEFORE_COMMAND
	zend_uchar			last_command;
#else
	zend_uchar			unused_pad1;
#endif
	MYSQLND_NET_OPTIONS	options;

	unsigned int		refcount;

	zend_bool			persistent;

	struct st_mysqlnd_net_methods m;
};


struct st_mysqlnd_net
{
	struct st_mysqlnd_net_data * data;

	/* sequence for simple checking of correct packets */
	zend_uchar			packet_no;
	zend_uchar			compressed_envelope_packet_no;

#ifdef MYSQLND_COMPRESSION_ENABLED
	MYSQLND_READ_BUFFER	* uncompressed_data;
#else
	void * 				unused_pad1;
#endif

	/* cmd buffer */
	MYSQLND_CMD_BUFFER	cmd_buffer;

	zend_bool persistent;
};


struct st_mysqlnd_protocol
{
	zend_bool persistent;
	struct st_mysqlnd_protocol_methods m;
};


struct st_mysqlnd_connection_data
{
/* Operation related */
	MYSQLND_NET		* net;
	MYSQLND_PROTOCOL * protocol;

/* Information related */
	char			*host;
	unsigned int	host_len;
	char			*unix_socket;
	unsigned int	unix_socket_len;
	char			*user;
	unsigned int	user_len;
	char			*passwd;
	unsigned int	passwd_len;
	char			*scheme;
	unsigned int	scheme_len;
	uint64_t		thread_id;
	char			*server_version;
	char			*host_info;
	zend_uchar		*auth_plugin_data;
	size_t			auth_plugin_data_len;
	const MYSQLND_CHARSET *charset;
	const MYSQLND_CHARSET *greet_charset;
	char			*connect_or_select_db;
	unsigned int	connect_or_select_db_len;
	MYSQLND_INFILE	infile;
	unsigned int	protocol_version;
	unsigned long	max_packet_size;
	unsigned int	port;
	unsigned long	client_flag;
	unsigned long	server_capabilities;

	/* For UPSERT queries */
	MYSQLND_UPSERT_STATUS * upsert_status;
	MYSQLND_UPSERT_STATUS upsert_status_impl;
	char			*last_message;
	unsigned int	last_message_len;

	/* If error packet, we use these */
	MYSQLND_ERROR_INFO	* error_info;
	MYSQLND_ERROR_INFO	error_info_impl;

	/*
	  To prevent queries during unbuffered fetches. Also to
	  mark the connection as destroyed for garbage collection.
	*/
	enum mysqlnd_connection_state	state;
	enum_mysqlnd_query_type			last_query_type;
	/* Temporary storage between query and (use|store)_result() call */
	MYSQLND_RES						*current_result;

	/*
	  How many result sets reference this connection.
	  It won't be freed until this number reaches 0.
	  The last one, please close the door! :-)
	  The result set objects can determine by inspecting
	  'quit_sent' whether the connection is still valid.
	*/
	unsigned int	refcount;

	/* Temporal storage for mysql_query */
	unsigned int	field_count;

	/* persistent connection */
	zend_bool		persistent;

	/* options */
	MYSQLND_OPTIONS	* options;
	MYSQLND_OPTIONS	options_impl;

	/* stats */
	MYSQLND_STATS	* stats;

	struct st_mysqlnd_conn_data_methods * m;
};


struct st_mysqlnd_connection
{
	MYSQLND_CONN_DATA * data;
	zend_bool persistent;
	struct st_mysqlnd_conn_methods * m;
};


struct mysqlnd_field_hash_key
{
	zend_bool		is_numeric;
	unsigned long	key;
#if MYSQLND_UNICODE
	zstr			ustr;
	unsigned int	ulen;
#endif
};


struct st_mysqlnd_result_metadata
{
	MYSQLND_FIELD					*fields;
	struct mysqlnd_field_hash_key	*zend_hash_keys;
	unsigned int					current_field;
	unsigned int					field_count;
	/* We need this to make fast allocs in rowp_read */
	unsigned int					bit_fields_count;
	size_t							bit_fields_total_len; /* trailing \0 not counted */
	zend_bool						persistent;

	struct st_mysqlnd_res_meta_methods *m;
};


struct st_mysqlnd_buffered_result
{
	zval				**data;
	zval				**data_cursor;
	MYSQLND_MEMORY_POOL_CHUNK **row_buffers;
	uint64_t			row_count;
	uint64_t			initialized_rows;

	unsigned int		references;

	MYSQLND_ERROR_INFO	error_info;
};


struct st_mysqlnd_unbuffered_result
{
	/* For unbuffered (both normal and PS) */
	zval				**last_row_data;
	MYSQLND_MEMORY_POOL_CHUNK *last_row_buffer;

	uint64_t			row_count;
	zend_bool			eof_reached;
};


struct st_mysqlnd_res
{
	MYSQLND_CONN_DATA		*conn;
	enum_mysqlnd_res_type	type;
	unsigned int			field_count;

	/* For metadata functions */
	MYSQLND_RES_METADATA	*meta;

	/* To be used with store_result() - both normal and PS */
	MYSQLND_RES_BUFFERED		*stored_data;
	MYSQLND_RES_UNBUFFERED		*unbuf;

	/*
	  Column lengths of current row - both buffered and unbuffered.
	  For buffered results it duplicates the data found in **data
	*/
	unsigned long			*lengths;

	struct st_mysqlnd_packet_row * row_packet;

	MYSQLND_MEMORY_POOL		* result_set_memory_pool;
	zend_bool				persistent;

	struct st_mysqlnd_res_methods m;
};


struct st_mysqlnd_param_bind
{
	zval		*zv;
	zend_uchar	type;
	enum_param_bind_flags	flags;
};

struct st_mysqlnd_result_bind
{
	zval		*zv;
	zend_bool	bound;
};


struct st_mysqlnd_stmt_data
{
	MYSQLND_CONN_DATA			*conn;
	unsigned long				stmt_id;
	unsigned long				flags;/* cursor is set here */
	enum_mysqlnd_stmt_state		state;
	unsigned int				warning_count;
	MYSQLND_RES					*result;
	unsigned int				field_count;
	unsigned int				param_count;
	unsigned char				send_types_to_server;
	MYSQLND_PARAM_BIND			*param_bind;
	MYSQLND_RESULT_BIND			*result_bind;
	zend_bool					result_zvals_separated_once;
	zend_bool					persistent;

	MYSQLND_UPSERT_STATUS * 	upsert_status;
	MYSQLND_UPSERT_STATUS 		upsert_status_impl;

	MYSQLND_ERROR_INFO *		error_info;
	MYSQLND_ERROR_INFO			error_info_impl;

	zend_bool					update_max_length;
	unsigned long				prefetch_rows;

	zend_bool					cursor_exists;
	mysqlnd_stmt_use_or_store_func default_rset_handler;

	MYSQLND_CMD_BUFFER			execute_cmd_buffer;
	unsigned int				execute_count;/* count how many times the stmt was executed */
};


struct st_mysqlnd_stmt
{
	MYSQLND_STMT_DATA * data;
	struct st_mysqlnd_stmt_methods	*m;
	zend_bool persistent;
};


typedef struct st_mysqlnd_string
{
	char	*s;
	size_t	l;
} MYSQLND_STRING;


struct st_mysqlnd_plugin_header
{
	unsigned int	plugin_api_version; 
	const char *	plugin_name;
	unsigned long	plugin_version;
	const char *	plugin_string_version;
	const char *	plugin_license;
	const char *	plugin_author;
	struct
	{
		MYSQLND_STATS *			values;
		const MYSQLND_STRING *	names;
	} plugin_stats;

	struct
	{
		enum_func_status (*plugin_shutdown)(void * plugin TSRMLS_DC);
	} m;
};


struct st_mysqlnd_plugin_core
{
	struct st_mysqlnd_plugin_header plugin_header;
};


struct st_mysqlnd_typeii_plugin_example
{
	struct st_mysqlnd_plugin_header plugin_header;
	void * methods;
	unsigned int counter;
};

struct st_mysqlnd_authentication_plugin;

typedef zend_uchar * (*func_auth_plugin__get_auth_data)(struct st_mysqlnd_authentication_plugin * self,
														size_t * auth_data_len,
														MYSQLND_CONN_DATA * conn, const char * const user, const char * const passwd,
														const size_t passwd_len, zend_uchar * auth_plugin_data, size_t auth_plugin_data_len,
														const MYSQLND_OPTIONS * const options, unsigned long mysql_flags
														TSRMLS_DC);

struct st_mysqlnd_authentication_plugin
{
	struct st_mysqlnd_plugin_header plugin_header;
	struct {
		func_auth_plugin__get_auth_data get_auth_data;
	} methods;
};


#endif /* MYSQLND_STRUCTS_H */
{
	func_mysqlnd_conn_data__init init;
	func_mysqlnd_conn_data__connect connect;
	func_mysqlnd_conn_data__escape_string escape_string;
	func_mysqlnd_conn_data__set_charset set_charset;
	func_mysqlnd_conn_data__query query;
	func_mysqlnd_conn_data__send_query send_query;
	func_mysqlnd_conn_data__reap_query reap_query;
	func_mysqlnd_conn_data__use_result use_result;
	func_mysqlnd_conn_data__store_result store_result;
	func_mysqlnd_conn_data__next_result next_result;
	func_mysqlnd_conn_data__more_results more_results;

	func_mysqlnd_conn_data__stmt_init stmt_init;

	func_mysqlnd_conn_data__shutdown_server shutdown_server;
	func_mysqlnd_conn_data__refresh_server refresh_server;

	func_mysqlnd_conn_data__ping ping;
	func_mysqlnd_conn_data__kill_connection kill_connection;
	func_mysqlnd_conn_data__select_db select_db;
	func_mysqlnd_conn_data__server_dump_debug_information server_dump_debug_information;
	func_mysqlnd_conn_data__change_user change_user;

	func_mysqlnd_conn_data__get_error_no get_error_no;
	func_mysqlnd_conn_data__get_error_str get_error_str;
	func_mysqlnd_conn_data__get_sqlstate get_sqlstate;
	func_mysqlnd_conn_data__get_thread_id get_thread_id;
	func_mysqlnd_conn_data__get_statistics get_statistics;

	func_mysqlnd_conn_data__get_server_version get_server_version;
	func_mysqlnd_conn_data__get_server_information get_server_information;
	func_mysqlnd_conn_data__get_server_statistics get_server_statistics;
	func_mysqlnd_conn_data__get_host_information get_host_information;
	func_mysqlnd_conn_data__get_protocol_information get_protocol_information;
	func_mysqlnd_conn_data__get_last_message get_last_message;
	func_mysqlnd_conn_data__charset_name charset_name;
	func_mysqlnd_conn_data__list_fields list_fields;
	func_mysqlnd_conn_data__list_method list_method;

	func_mysqlnd_conn_data__get_last_insert_id get_last_insert_id;
	func_mysqlnd_conn_data__get_affected_rows get_affected_rows;
	func_mysqlnd_conn_data__get_warning_count get_warning_count;

	func_mysqlnd_conn_data__get_field_count get_field_count;

	func_mysqlnd_conn_data__get_server_status get_server_status;
	
	func_mysqlnd_conn_data__set_server_option set_server_option;
	func_mysqlnd_conn_data__set_client_option set_client_option;
	func_mysqlnd_conn_data__free_contents free_contents;
	func_mysqlnd_conn_data__free_options free_options;
	func_mysqlnd_conn_data__dtor dtor;

	func_mysqlnd_conn_data__query_read_result_set_header query_read_result_set_header;

	func_mysqlnd_conn_data__get_reference get_reference;
	func_mysqlnd_conn_data__free_reference free_reference;
	func_mysqlnd_conn_data__get_state get_state;
	func_mysqlnd_conn_data__set_state set_state;

	func_mysqlnd_conn_data__simple_command simple_command;
	func_mysqlnd_conn_data__simple_command_handle_response simple_command_handle_response;

	func_mysqlnd_conn_data__restart_psession restart_psession;
	func_mysqlnd_conn_data__end_psession end_psession;
	func_mysqlnd_conn_data__send_close send_close;

	func_mysqlnd_conn_data__ssl_set ssl_set;

	func_mysqlnd_conn_data__result_init result_init;
	func_mysqlnd_conn_data__set_autocommit set_autocommit;
	func_mysqlnd_conn_data__tx_commit tx_commit;
	func_mysqlnd_conn_data__tx_rollback tx_rollback;

	func_mysqlnd_conn_data__local_tx_start local_tx_start;
	func_mysqlnd_conn_data__local_tx_end local_tx_end;
};


typedef enum_func_status	(*func_mysqlnd_data__connect)(MYSQLND * conn, const char * host, const char * user, const char * passwd, unsigned int passwd_len, const char * db, unsigned int db_len, unsigned int port, const char * socket_or_pipe, unsigned int mysql_flags TSRMLS_DC);
typedef MYSQLND *			(*func_mysqlnd_conn__clone_object)(MYSQLND * const conn TSRMLS_DC);
typedef void				(*func_mysqlnd_conn__dtor)(MYSQLND * conn TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_conn__close)(MYSQLND * conn, enum_connection_close_type close_type TSRMLS_DC);

struct st_mysqlnd_conn_methods
{
	func_mysqlnd_data__connect connect;
	func_mysqlnd_conn__clone_object clone_object;
	func_mysqlnd_conn__dtor dtor;
	func_mysqlnd_conn__close close;
};


typedef mysqlnd_fetch_row_func	fetch_row;
typedef mysqlnd_fetch_row_func	fetch_row_normal_buffered; /* private */
typedef mysqlnd_fetch_row_func	fetch_row_normal_unbuffered; /* private */

typedef MYSQLND_RES *		(*func_mysqlnd_res__use_result)(MYSQLND_RES * const result, zend_bool ps_protocol TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_res__store_result)(MYSQLND_RES * result, MYSQLND_CONN_DATA * const conn, zend_bool ps TSRMLS_DC);
typedef void 				(*func_mysqlnd_res__fetch_into)(MYSQLND_RES *result, unsigned int flags, zval *return_value, enum_mysqlnd_extension ext TSRMLS_DC ZEND_FILE_LINE_DC);
typedef MYSQLND_ROW_C 		(*func_mysqlnd_res__fetch_row_c)(MYSQLND_RES *result TSRMLS_DC);
typedef void 				(*func_mysqlnd_res__fetch_all)(MYSQLND_RES *result, unsigned int flags, zval *return_value TSRMLS_DC ZEND_FILE_LINE_DC);
typedef void 				(*func_mysqlnd_res__fetch_field_data)(MYSQLND_RES *result, unsigned int offset, zval *return_value TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_res__num_rows)(const MYSQLND_RES * const result TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_res__num_fields)(const MYSQLND_RES * const result TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_res__skip_result)(MYSQLND_RES * const result TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_res__seek_data)(MYSQLND_RES * result, uint64_t row TSRMLS_DC);
typedef MYSQLND_FIELD_OFFSET (*func_mysqlnd_res__seek_field)(MYSQLND_RES * const result, MYSQLND_FIELD_OFFSET field_offset TSRMLS_DC);
typedef MYSQLND_FIELD_OFFSET (*func_mysqlnd_res__field_tell)(const MYSQLND_RES * const result TSRMLS_DC);
typedef const MYSQLND_FIELD *(*func_mysqlnd_res__fetch_field)(MYSQLND_RES * const result TSRMLS_DC);
typedef const MYSQLND_FIELD *(*func_mysqlnd_res__fetch_field_direct)(MYSQLND_RES * const result, MYSQLND_FIELD_OFFSET fieldnr TSRMLS_DC);
typedef const MYSQLND_FIELD *(*func_mysqlnd_res__fetch_fields)(MYSQLND_RES * const result TSRMLS_DC);

typedef enum_func_status	(*func_mysqlnd_res__read_result_metadata)(MYSQLND_RES * result, MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef unsigned long *		(*func_mysqlnd_res__fetch_lengths)(MYSQLND_RES * const result TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_res__store_result_fetch_data)(MYSQLND_CONN_DATA * const conn, MYSQLND_RES * result, MYSQLND_RES_METADATA *meta, zend_bool binary_protocol TSRMLS_DC);
typedef enum_func_status 	(*func_mysqlnd_res__initialize_result_set_rest)(MYSQLND_RES * const result TSRMLS_DC);

typedef void				(*func_mysqlnd_res__free_result_buffers)(MYSQLND_RES * result TSRMLS_DC);	/* private */
typedef enum_func_status	(*func_mysqlnd_res__free_result)(MYSQLND_RES * result, zend_bool implicit TSRMLS_DC);
typedef void				(*func_mysqlnd_res__free_result_internal)(MYSQLND_RES *result TSRMLS_DC);
typedef void				(*func_mysqlnd_res__free_result_contents)(MYSQLND_RES *result TSRMLS_DC);
typedef void				(*func_mysqlnd_res__free_buffered_data)(MYSQLND_RES *result TSRMLS_DC);
typedef void				(*func_mysqlnd_res__unbuffered_free_last_data)(MYSQLND_RES *result TSRMLS_DC);

	/* for decoding - binary or text protocol */
typedef enum_func_status	(*func_mysqlnd_res__row_decoder)(MYSQLND_MEMORY_POOL_CHUNK * row_buffer, zval ** fields,
									unsigned int field_count, MYSQLND_FIELD *fields_metadata,
									zend_bool as_unicode, zend_bool as_int_or_float,
									MYSQLND_STATS * stats TSRMLS_DC);

typedef MYSQLND_RES_METADATA * (*func_mysqlnd_res__result_meta_init)(unsigned int field_count, zend_bool persistent TSRMLS_DC);

struct st_mysqlnd_res_methods
{
	mysqlnd_fetch_row_func	fetch_row;
	mysqlnd_fetch_row_func	fetch_row_normal_buffered; /* private */
	mysqlnd_fetch_row_func	fetch_row_normal_unbuffered; /* private */

	func_mysqlnd_res__use_result use_result;
	func_mysqlnd_res__store_result store_result;
	func_mysqlnd_res__fetch_into fetch_into;
	func_mysqlnd_res__fetch_row_c fetch_row_c;
	func_mysqlnd_res__fetch_all fetch_all;
	func_mysqlnd_res__fetch_field_data fetch_field_data;
	func_mysqlnd_res__num_rows num_rows;
	func_mysqlnd_res__num_fields num_fields;
	func_mysqlnd_res__skip_result skip_result;
	func_mysqlnd_res__seek_data seek_data;
	func_mysqlnd_res__seek_field seek_field;
	func_mysqlnd_res__field_tell field_tell;
	func_mysqlnd_res__fetch_field fetch_field;
	func_mysqlnd_res__fetch_field_direct fetch_field_direct;
	func_mysqlnd_res__fetch_fields fetch_fields;
	func_mysqlnd_res__read_result_metadata read_result_metadata;
	func_mysqlnd_res__fetch_lengths fetch_lengths;
	func_mysqlnd_res__store_result_fetch_data store_result_fetch_data;
	func_mysqlnd_res__initialize_result_set_rest initialize_result_set_rest;
	func_mysqlnd_res__free_result_buffers free_result_buffers;
	func_mysqlnd_res__free_result free_result;
	func_mysqlnd_res__free_result_internal free_result_internal;
	func_mysqlnd_res__free_result_contents free_result_contents;
	func_mysqlnd_res__free_buffered_data free_buffered_data;
	func_mysqlnd_res__unbuffered_free_last_data unbuffered_free_last_data;

	/* for decoding - binary or text protocol */
	func_mysqlnd_res__row_decoder row_decoder;

	func_mysqlnd_res__result_meta_init result_meta_init;

	void * unused1;
	void * unused2;
	void * unused3;
	void * unused4;
	void * unused5;
};


typedef const MYSQLND_FIELD *	(*func_mysqlnd_res_meta__fetch_field)(MYSQLND_RES_METADATA * const meta TSRMLS_DC);
typedef const MYSQLND_FIELD *	(*func_mysqlnd_res_meta__fetch_field_direct)(const MYSQLND_RES_METADATA * const meta, MYSQLND_FIELD_OFFSET fieldnr TSRMLS_DC);
typedef const MYSQLND_FIELD *	(*func_mysqlnd_res_meta__fetch_fields)(MYSQLND_RES_METADATA * const meta TSRMLS_DC);
typedef MYSQLND_FIELD_OFFSET	(*func_mysqlnd_res_meta__field_tell)(const MYSQLND_RES_METADATA * const meta TSRMLS_DC);
typedef enum_func_status		(*func_mysqlnd_res_meta__read_metadata)(MYSQLND_RES_METADATA * const meta, MYSQLND_CONN_DATA * conn TSRMLS_DC);
typedef MYSQLND_RES_METADATA *	(*func_mysqlnd_res_meta__clone_metadata)(const MYSQLND_RES_METADATA * const meta, zend_bool persistent TSRMLS_DC);
typedef void					(*func_mysqlnd_res_meta__free_metadata)(MYSQLND_RES_METADATA * meta TSRMLS_DC);

struct st_mysqlnd_res_meta_methods
{
	func_mysqlnd_res_meta__fetch_field fetch_field;
	func_mysqlnd_res_meta__fetch_field_direct fetch_field_direct;
	func_mysqlnd_res_meta__fetch_fields fetch_fields;
	func_mysqlnd_res_meta__field_tell field_tell;
	func_mysqlnd_res_meta__read_metadata read_metadata;
	func_mysqlnd_res_meta__clone_metadata clone_metadata;
	func_mysqlnd_res_meta__free_metadata free_metadata;
};


typedef enum_func_status	(*func_mysqlnd_stmt__prepare)(MYSQLND_STMT * const stmt, const char * const query, unsigned int query_len TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__execute)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__use_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__store_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__get_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef zend_bool			(*func_mysqlnd_stmt__more_results)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__next_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__free_result)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__seek_data)(const MYSQLND_STMT * const stmt, uint64_t row TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__reset)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__net_close)(MYSQLND_STMT * const stmt, zend_bool implicit TSRMLS_DC); /* private */
typedef enum_func_status	(*func_mysqlnd_stmt__dtor)(MYSQLND_STMT * const stmt, zend_bool implicit TSRMLS_DC); /* use this for mysqlnd_stmt_close */
typedef enum_func_status	(*func_mysqlnd_stmt__fetch)(MYSQLND_STMT * const stmt, zend_bool * const fetched_anything TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_parameters)(MYSQLND_STMT * const stmt, MYSQLND_PARAM_BIND * const param_bind TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_one_parameter)(MYSQLND_STMT * const stmt, unsigned int param_no, zval * const zv, zend_uchar	type TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__refresh_bind_param)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_result)(MYSQLND_STMT * const stmt, MYSQLND_RESULT_BIND * const result_bind TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__bind_one_result)(MYSQLND_STMT * const stmt, unsigned int param_no TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__send_long_data)(MYSQLND_STMT * const stmt, unsigned int param_num, const char * const data, unsigned long length TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__get_parameter_metadata)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RES *		(*func_mysqlnd_stmt__get_result_metadata)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_stmt__get_last_insert_id)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_stmt__get_affected_rows)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef uint64_t			(*func_mysqlnd_stmt__get_num_rows)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_param_count)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_field_count)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_warning_count)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__get_error_no)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef const char *		(*func_mysqlnd_stmt__get_error_str)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef const char *		(*func_mysqlnd_stmt__get_sqlstate)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__get_attribute)(const MYSQLND_STMT * const stmt, enum mysqlnd_stmt_attr attr_type, void * const value TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__set_attribute)(MYSQLND_STMT * const stmt, enum mysqlnd_stmt_attr attr_type, const void * const value TSRMLS_DC);
typedef MYSQLND_PARAM_BIND *(*func_mysqlnd_stmt__alloc_param_bind)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef MYSQLND_RESULT_BIND*(*func_mysqlnd_stmt__alloc_result_bind)(MYSQLND_STMT * const stmt TSRMLS_DC);
typedef	void 				(*func_mysqlnd_stmt__free_parameter_bind)(MYSQLND_STMT * const stmt, MYSQLND_PARAM_BIND * TSRMLS_DC);
typedef	void 				(*func_mysqlnd_stmt__free_result_bind)(MYSQLND_STMT * const stmt, MYSQLND_RESULT_BIND * TSRMLS_DC);
typedef unsigned int		(*func_mysqlnd_stmt__server_status)(const MYSQLND_STMT * const stmt TSRMLS_DC);
typedef enum_func_status 	(*func_mysqlnd_stmt__generate_execute_request)(MYSQLND_STMT * const s, zend_uchar ** request, size_t *request_len, zend_bool * free_buffer TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__parse_execute_response)(MYSQLND_STMT * const s TSRMLS_DC);
typedef void 				(*func_mysqlnd_stmt__free_stmt_content)(MYSQLND_STMT * const s TSRMLS_DC);
typedef enum_func_status	(*func_mysqlnd_stmt__flush)(MYSQLND_STMT * const stmt TSRMLS_DC);

struct st_mysqlnd_stmt_methods
{
	func_mysqlnd_stmt__prepare prepare;
	func_mysqlnd_stmt__execute execute;
	func_mysqlnd_stmt__use_result use_result;
	func_mysqlnd_stmt__store_result store_result;
	func_mysqlnd_stmt__get_result get_result;
	func_mysqlnd_stmt__more_results more_results;
	func_mysqlnd_stmt__next_result next_result;
	func_mysqlnd_stmt__free_result free_result;
	func_mysqlnd_stmt__seek_data seek_data;
	func_mysqlnd_stmt__reset reset;
	func_mysqlnd_stmt__net_close net_close;
	func_mysqlnd_stmt__dtor dtor;
	func_mysqlnd_stmt__fetch fetch;

	func_mysqlnd_stmt__bind_parameters bind_parameters;
	func_mysqlnd_stmt__bind_one_parameter bind_one_parameter;
	func_mysqlnd_stmt__refresh_bind_param refresh_bind_param;
	func_mysqlnd_stmt__bind_result bind_result;
	func_mysqlnd_stmt__bind_one_result bind_one_result;
	func_mysqlnd_stmt__send_long_data send_long_data;
	func_mysqlnd_stmt__get_parameter_metadata get_parameter_metadata;
	func_mysqlnd_stmt__get_result_metadata get_result_metadata;

	func_mysqlnd_stmt__get_last_insert_id get_last_insert_id;
	func_mysqlnd_stmt__get_affected_rows get_affected_rows;
	func_mysqlnd_stmt__get_num_rows get_num_rows;

	func_mysqlnd_stmt__get_param_count get_param_count;
	func_mysqlnd_stmt__get_field_count get_field_count;
	func_mysqlnd_stmt__get_warning_count get_warning_count;

	func_mysqlnd_stmt__get_error_no get_error_no;
	func_mysqlnd_stmt__get_error_str get_error_str;
	func_mysqlnd_stmt__get_sqlstate get_sqlstate;

	func_mysqlnd_stmt__get_attribute get_attribute;
	func_mysqlnd_stmt__set_attribute set_attribute;

	func_mysqlnd_stmt__alloc_param_bind alloc_parameter_bind;
	func_mysqlnd_stmt__alloc_result_bind alloc_result_bind;

	func_mysqlnd_stmt__free_parameter_bind free_parameter_bind;
	func_mysqlnd_stmt__free_result_bind free_result_bind;

	func_mysqlnd_stmt__server_status get_server_status;

	func_mysqlnd_stmt__generate_execute_request generate_execute_request;
	func_mysqlnd_stmt__parse_execute_response parse_execute_response;

	func_mysqlnd_stmt__free_stmt_content free_stmt_content;

	func_mysqlnd_stmt__flush flush;
};


struct st_mysqlnd_net_data
{
	php_stream			*stream;
	zend_bool			compressed;
#ifdef MYSQLND_DO_WIRE_CHECK_BEFORE_COMMAND
	zend_uchar			last_command;
#else
	zend_uchar			unused_pad1;
#endif
	MYSQLND_NET_OPTIONS	options;

	unsigned int		refcount;

	zend_bool			persistent;

	struct st_mysqlnd_net_methods m;
};


struct st_mysqlnd_net
{
	struct st_mysqlnd_net_data * data;

	/* sequence for simple checking of correct packets */
	zend_uchar			packet_no;
	zend_uchar			compressed_envelope_packet_no;

#ifdef MYSQLND_COMPRESSION_ENABLED
	MYSQLND_READ_BUFFER	* uncompressed_data;
#else
	void * 				unused_pad1;
#endif

	/* cmd buffer */
	MYSQLND_CMD_BUFFER	cmd_buffer;

	zend_bool persistent;
};


struct st_mysqlnd_protocol
{
	zend_bool persistent;
	struct st_mysqlnd_protocol_methods m;
};


struct st_mysqlnd_connection_data
{
/* Operation related */
	MYSQLND_NET		* net;
	MYSQLND_PROTOCOL * protocol;

/* Information related */
	char			*host;
	unsigned int	host_len;
	char			*unix_socket;
	unsigned int	unix_socket_len;
	char			*user;
	unsigned int	user_len;
	char			*passwd;
	unsigned int	passwd_len;
	char			*scheme;
	unsigned int	scheme_len;
	uint64_t		thread_id;
	char			*server_version;
	char			*host_info;
	zend_uchar		*auth_plugin_data;
	size_t			auth_plugin_data_len;
	const MYSQLND_CHARSET *charset;
	const MYSQLND_CHARSET *greet_charset;
	char			*connect_or_select_db;
	unsigned int	connect_or_select_db_len;
	MYSQLND_INFILE	infile;
	unsigned int	protocol_version;
	unsigned long	max_packet_size;
	unsigned int	port;
	unsigned long	client_flag;
	unsigned long	server_capabilities;

	/* For UPSERT queries */
	MYSQLND_UPSERT_STATUS * upsert_status;
	MYSQLND_UPSERT_STATUS upsert_status_impl;
	char			*last_message;
	unsigned int	last_message_len;

	/* If error packet, we use these */
	MYSQLND_ERROR_INFO	* error_info;
	MYSQLND_ERROR_INFO	error_info_impl;

	/*
	  To prevent queries during unbuffered fetches. Also to
	  mark the connection as destroyed for garbage collection.
	*/
	enum mysqlnd_connection_state	state;
	enum_mysqlnd_query_type			last_query_type;
	/* Temporary storage between query and (use|store)_result() call */
	MYSQLND_RES						*current_result;

	/*
	  How many result sets reference this connection.
	  It won't be freed until this number reaches 0.
	  The last one, please close the door! :-)
	  The result set objects can determine by inspecting
	  'quit_sent' whether the connection is still valid.
	*/
	unsigned int	refcount;

	/* Temporal storage for mysql_query */
	unsigned int	field_count;

	/* persistent connection */
	zend_bool		persistent;

	/* options */
	MYSQLND_OPTIONS	* options;
	MYSQLND_OPTIONS	options_impl;

	/* stats */
	MYSQLND_STATS	* stats;

	struct st_mysqlnd_conn_data_methods * m;
};


struct st_mysqlnd_connection
{
	MYSQLND_CONN_DATA * data;
	zend_bool persistent;
	struct st_mysqlnd_conn_methods * m;
};


struct mysqlnd_field_hash_key
{
	zend_bool		is_numeric;
	unsigned long	key;
#if MYSQLND_UNICODE
	zstr			ustr;
	unsigned int	ulen;
#endif
};


struct st_mysqlnd_result_metadata
{
	MYSQLND_FIELD					*fields;
	struct mysqlnd_field_hash_key	*zend_hash_keys;
	unsigned int					current_field;
	unsigned int					field_count;
	/* We need this to make fast allocs in rowp_read */
	unsigned int					bit_fields_count;
	size_t							bit_fields_total_len; /* trailing \0 not counted */
	zend_bool						persistent;

	struct st_mysqlnd_res_meta_methods *m;
};


struct st_mysqlnd_buffered_result
{
	zval				**data;
	zval				**data_cursor;
	MYSQLND_MEMORY_POOL_CHUNK **row_buffers;
	uint64_t			row_count;
	uint64_t			initialized_rows;

	unsigned int		references;

	MYSQLND_ERROR_INFO	error_info;
};


struct st_mysqlnd_unbuffered_result
{
	/* For unbuffered (both normal and PS) */
	zval				**last_row_data;
	MYSQLND_MEMORY_POOL_CHUNK *last_row_buffer;

	uint64_t			row_count;
	zend_bool			eof_reached;
};


struct st_mysqlnd_res
{
	MYSQLND_CONN_DATA		*conn;
	enum_mysqlnd_res_type	type;
	unsigned int			field_count;

	/* For metadata functions */
	MYSQLND_RES_METADATA	*meta;

	/* To be used with store_result() - both normal and PS */
	MYSQLND_RES_BUFFERED		*stored_data;
	MYSQLND_RES_UNBUFFERED		*unbuf;

	/*
	  Column lengths of current row - both buffered and unbuffered.
	  For buffered results it duplicates the data found in **data
	*/
	unsigned long			*lengths;

	struct st_mysqlnd_packet_row * row_packet;

	MYSQLND_MEMORY_POOL		* result_set_memory_pool;
	zend_bool				persistent;

	struct st_mysqlnd_res_methods m;
};


struct st_mysqlnd_param_bind
{
	zval		*zv;
	zend_uchar	type;
	enum_param_bind_flags	flags;
};

struct st_mysqlnd_result_bind
{
	zval		*zv;
	zend_bool	bound;
};


struct st_mysqlnd_stmt_data
{
	MYSQLND_CONN_DATA			*conn;
	unsigned long				stmt_id;
	unsigned long				flags;/* cursor is set here */
	enum_mysqlnd_stmt_state		state;
	unsigned int				warning_count;
	MYSQLND_RES					*result;
	unsigned int				field_count;
	unsigned int				param_count;
	unsigned char				send_types_to_server;
	MYSQLND_PARAM_BIND			*param_bind;
	MYSQLND_RESULT_BIND			*result_bind;
	zend_bool					result_zvals_separated_once;
	zend_bool					persistent;

	MYSQLND_UPSERT_STATUS * 	upsert_status;
	MYSQLND_UPSERT_STATUS 		upsert_status_impl;

	MYSQLND_ERROR_INFO *		error_info;
	MYSQLND_ERROR_INFO			error_info_impl;

	zend_bool					update_max_length;
	unsigned long				prefetch_rows;

	zend_bool					cursor_exists;
	mysqlnd_stmt_use_or_store_func default_rset_handler;

	MYSQLND_CMD_BUFFER			execute_cmd_buffer;
	unsigned int				execute_count;/* count how many times the stmt was executed */
};


struct st_mysqlnd_stmt
{
	MYSQLND_STMT_DATA * data;
	struct st_mysqlnd_stmt_methods	*m;
	zend_bool persistent;
};


typedef struct st_mysqlnd_string
{
	char	*s;
	size_t	l;
} MYSQLND_STRING;


struct st_mysqlnd_plugin_header
{
	unsigned int	plugin_api_version; 
	const char *	plugin_name;
	unsigned long	plugin_version;
	const char *	plugin_string_version;
	const char *	plugin_license;
	const char *	plugin_author;
	struct
	{
		MYSQLND_STATS *			values;
		const MYSQLND_STRING *	names;
	} plugin_stats;

	struct
	{
		enum_func_status (*plugin_shutdown)(void * plugin TSRMLS_DC);
	} m;
};


struct st_mysqlnd_plugin_core
{
	struct st_mysqlnd_plugin_header plugin_header;
};


struct st_mysqlnd_typeii_plugin_example
{
	struct st_mysqlnd_plugin_header plugin_header;
	void * methods;
	unsigned int counter;
};

struct st_mysqlnd_authentication_plugin;

typedef zend_uchar * (*func_auth_plugin__get_auth_data)(struct st_mysqlnd_authentication_plugin * self,
														size_t * auth_data_len,
														MYSQLND_CONN_DATA * conn, const char * const user, const char * const passwd,
														const size_t passwd_len, zend_uchar * auth_plugin_data, size_t auth_plugin_data_len,
														const MYSQLND_OPTIONS * const options, unsigned long mysql_flags
														TSRMLS_DC);

struct st_mysqlnd_authentication_plugin
{
	struct st_mysqlnd_plugin_header plugin_header;
	struct {
		func_auth_plugin__get_auth_data get_auth_data;
	} methods;
};


#endif /* MYSQLND_STRUCTS_H */