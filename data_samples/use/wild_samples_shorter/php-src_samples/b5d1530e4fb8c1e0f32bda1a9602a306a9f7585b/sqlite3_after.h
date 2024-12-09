

/*
** Provide the ability to override linkage features of the interface.
*/
#ifndef SQLITE_EXTERN
# define SQLITE_EXTERN extern
#endif
#ifndef SQLITE_API
# define SQLITE_API
#endif
#ifndef SQLITE_CDECL
# define SQLITE_CDECL
#endif
#ifndef SQLITE_STDCALL
# define SQLITE_STDCALL
#endif

/*
** These no-op macros are used in front of interfaces to mark those
** interfaces as either deprecated or experimental.  New applications
** [sqlite3_libversion_number()], [sqlite3_sourceid()],
** [sqlite_version()] and [sqlite_source_id()].
*/
#define SQLITE_VERSION        "3.8.10.2"
#define SQLITE_VERSION_NUMBER 3008010
#define SQLITE_SOURCE_ID      "2015-05-20 18:17:19 2ef4f3a5b1d1d0c4338f8243d40a2452cc1f7fe4"

/*
** CAPI3REF: Run-Time Library Version Numbers
** KEYWORDS: sqlite3_version, sqlite3_sourceid
** See also: [sqlite_version()] and [sqlite_source_id()].
*/
SQLITE_API SQLITE_EXTERN const char sqlite3_version[];
SQLITE_API const char *SQLITE_STDCALL sqlite3_libversion(void);
SQLITE_API const char *SQLITE_STDCALL sqlite3_sourceid(void);
SQLITE_API int SQLITE_STDCALL sqlite3_libversion_number(void);

/*
** CAPI3REF: Run-Time Library Compilation Options Diagnostics
**
** [sqlite_compileoption_get()] and the [compile_options pragma].
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
SQLITE_API int SQLITE_STDCALL sqlite3_compileoption_used(const char *zOptName);
SQLITE_API const char *SQLITE_STDCALL sqlite3_compileoption_get(int N);
#endif

/*
** CAPI3REF: Test To See If The Library Is Threadsafe
**
** See the [threading mode] documentation for additional information.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_threadsafe(void);

/*
** CAPI3REF: Database Connection Handle
** KEYWORDS: {database connection} {database connections}

/*
** CAPI3REF: Closing A Database Connection
** DESTRUCTOR: sqlite3
**
** ^The sqlite3_close() and sqlite3_close_v2() routines are destructors
** for the [sqlite3] object.
** ^Calls to sqlite3_close() and sqlite3_close_v2() return [SQLITE_OK] if
** ^Calling sqlite3_close() or sqlite3_close_v2() with a NULL pointer
** argument is a harmless no-op.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_close(sqlite3*);
SQLITE_API int SQLITE_STDCALL sqlite3_close_v2(sqlite3*);

/*
** The type for a callback function.
** This is legacy and deprecated.  It is included for historical

/*
** CAPI3REF: One-Step Query Execution Interface
** METHOD: sqlite3
**
** The sqlite3_exec() interface is a convenience wrapper around
** [sqlite3_prepare_v2()], [sqlite3_step()], and [sqlite3_finalize()],
** that allows an application to run multiple statements of SQL
**      the 2nd parameter of sqlite3_exec() while sqlite3_exec() is running.
** </ul>
*/
SQLITE_API int SQLITE_STDCALL sqlite3_exec(
  sqlite3*,                                  /* An open database */
  const char *sql,                           /* SQL to be evaluated */
  int (*callback)(void*,int,char**,char**),  /* Callback function */
  void *,                                    /* 1st argument to callback */
** of the [sqlite3_io_methods] object and for the [sqlite3_file_control()]
** interface.
**
** <ul>
** <li>[[SQLITE_FCNTL_LOCKSTATE]]
** The [SQLITE_FCNTL_LOCKSTATE] opcode is used for debugging.  This
** opcode causes the xFileControl method to write the current state of
** the lock (one of [SQLITE_LOCK_NONE], [SQLITE_LOCK_SHARED],
** [SQLITE_LOCK_RESERVED], [SQLITE_LOCK_PENDING], or [SQLITE_LOCK_EXCLUSIVE])
** into an integer that the pArg argument points to. This capability
** is used during testing and is only available when the SQLITE_TEST
** compile-time option is used.
**
** <li>[[SQLITE_FCNTL_SIZE_HINT]]
** The [SQLITE_FCNTL_SIZE_HINT] opcode is used by SQLite to give the VFS
** layer a hint of how large the database file will grow to be during the
** current transaction.  This hint is not guaranteed to be accurate but it
** [PRAGMA] processing continues.  ^If the [SQLITE_FCNTL_PRAGMA]
** file control returns [SQLITE_OK], then the parser assumes that the
** VFS has handled the PRAGMA itself and the parser generates a no-op
** prepared statement if result string is NULL, or that returns a copy
** of the result string if the string is non-NULL.
** ^If the [SQLITE_FCNTL_PRAGMA] file control returns
** any result code other than [SQLITE_OK] or [SQLITE_NOTFOUND], that means
** that the VFS encountered an error while handling the [PRAGMA] and the
** compilation of the PRAGMA fails with an error.  ^The [SQLITE_FCNTL_PRAGMA]
** file control occurs at the beginning of pragma statement analysis and so
** pointed to by the pArg argument.  This capability is used during testing
** and only needs to be supported when SQLITE_TEST is defined.
**
** <li>[[SQLITE_FCNTL_WAL_BLOCK]]
** The [SQLITE_FCNTL_WAL_BLOCK] is a signal to the VFS layer that it might
** be advantageous to block on the next WAL lock if the lock is not immediately
** available.  The WAL subsystem issues this signal during rare
** circumstances in order to fix a problem with priority inversion.
** Applications should <em>not</em> use this file-control.
**
** </ul>
*/
#define SQLITE_FCNTL_LOCKSTATE               1
#define SQLITE_FCNTL_GET_LOCKPROXYFILE       2
#define SQLITE_FCNTL_SET_LOCKPROXYFILE       3
#define SQLITE_FCNTL_LAST_ERRNO              4
#define SQLITE_FCNTL_SIZE_HINT               5
#define SQLITE_FCNTL_CHUNK_SIZE              6
#define SQLITE_FCNTL_FILE_POINTER            7
#define SQLITE_FCNTL_SYNC_OMITTED            8
#define SQLITE_FCNTL_SYNC                   21
#define SQLITE_FCNTL_COMMIT_PHASETWO        22
#define SQLITE_FCNTL_WIN32_SET_HANDLE       23
#define SQLITE_FCNTL_WAL_BLOCK              24

/* deprecated names */
#define SQLITE_GET_LOCKPROXYFILE      SQLITE_FCNTL_GET_LOCKPROXYFILE
#define SQLITE_SET_LOCKPROXYFILE      SQLITE_FCNTL_SET_LOCKPROXYFILE
#define SQLITE_LAST_ERRNO             SQLITE_FCNTL_LAST_ERRNO


/*
** CAPI3REF: Mutex Handle
**
** must return [SQLITE_OK] on success and some other [error code] upon
** failure.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_initialize(void);
SQLITE_API int SQLITE_STDCALL sqlite3_shutdown(void);
SQLITE_API int SQLITE_STDCALL sqlite3_os_init(void);
SQLITE_API int SQLITE_STDCALL sqlite3_os_end(void);

/*
** CAPI3REF: Configuring The SQLite Library
**
** ^If the option is unknown or SQLite is unable to set the option
** then this routine returns a non-zero [error code].
*/
SQLITE_API int SQLITE_CDECL sqlite3_config(int, ...);

/*
** CAPI3REF: Configure database connections
** METHOD: sqlite3
**
** The sqlite3_db_config() interface is used to make configuration
** changes to a [database connection].  The interface is similar to
** [sqlite3_config()] except that the changes apply to a single
** ^Calls to sqlite3_db_config() return SQLITE_OK if and only if
** the call is considered successful.
*/
SQLITE_API int SQLITE_CDECL sqlite3_db_config(sqlite3*, int op, ...);

/*
** CAPI3REF: Memory Allocation Routines
**
**   <li> [sqlite3_memory_used()]
**   <li> [sqlite3_memory_highwater()]
**   <li> [sqlite3_soft_heap_limit64()]
**   <li> [sqlite3_status64()]
**   </ul>)^
** ^Memory allocation statistics are enabled by default unless SQLite is
** compiled with [SQLITE_DEFAULT_MEMSTATUS]=0 in which case memory
** allocation statistics are disabled by default.
** compiled for Windows with the [SQLITE_WIN32_MALLOC] pre-processor macro
** defined. ^SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit unsigned integer value
** that specifies the maximum size of the created heap.
**
** [[SQLITE_CONFIG_PCACHE_HDRSZ]]
** <dt>SQLITE_CONFIG_PCACHE_HDRSZ
** <dd>^The SQLITE_CONFIG_PCACHE_HDRSZ option takes a single parameter which

/*
** CAPI3REF: Enable Or Disable Extended Result Codes
** METHOD: sqlite3
**
** ^The sqlite3_extended_result_codes() routine enables or disables the
** [extended result codes] feature of SQLite. ^The extended result
** codes are disabled by default for historical compatibility.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_extended_result_codes(sqlite3*, int onoff);

/*
** CAPI3REF: Last Insert Rowid
** METHOD: sqlite3
**
** ^Each entry in most SQLite tables (except for [WITHOUT ROWID] tables)
** has a unique 64-bit signed
** integer key called the [ROWID | "rowid"]. ^The rowid is always available
** unpredictable and might not equal either the old or the new
** last insert [rowid].
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_last_insert_rowid(sqlite3*);

/*
** CAPI3REF: Count The Number Of Rows Modified
** METHOD: sqlite3
**
** ^This function returns the number of rows modified, inserted or
** deleted by the most recently completed INSERT, UPDATE or DELETE
** statement on the database connection specified by the only parameter.
** while [sqlite3_changes()] is running then the value returned
** is unpredictable and not meaningful.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_changes(sqlite3*);

/*
** CAPI3REF: Total Number Of Rows Modified
** METHOD: sqlite3
**
** ^This function returns the total number of rows inserted, modified or
** deleted by all [INSERT], [UPDATE] or [DELETE] statements completed
** since the database connection was opened, including those executed as
** while [sqlite3_total_changes()] is running then the value
** returned is unpredictable and not meaningful.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_total_changes(sqlite3*);

/*
** CAPI3REF: Interrupt A Long-Running Query
** METHOD: sqlite3
**
** ^This function causes any pending database operation to abort and
** return at its earliest opportunity. This routine is typically
** called in response to a user action such as pressing "Cancel"
** If the database connection closes while [sqlite3_interrupt()]
** is running then bad things will likely happen.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_interrupt(sqlite3*);

/*
** CAPI3REF: Determine If An SQL Statement Is Complete
**
** The input to [sqlite3_complete16()] must be a zero-terminated
** UTF-16 string in native byte order.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_complete(const char *sql);
SQLITE_API int SQLITE_STDCALL sqlite3_complete16(const void *sql);

/*
** CAPI3REF: Register A Callback To Handle SQLITE_BUSY Errors
** KEYWORDS: {busy-handler callback} {busy handler}
** METHOD: sqlite3
**
** ^The sqlite3_busy_handler(D,X,P) routine sets a callback function X
** that might be invoked with argument P whenever
** an attempt is made to access a database table associated with
** A busy handler must not close the database connection
** or [prepared statement] that invoked the busy handler.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_busy_handler(sqlite3*, int(*)(void*,int), void*);

/*
** CAPI3REF: Set A Busy Timeout
** METHOD: sqlite3
**
** ^This routine sets a [sqlite3_busy_handler | busy handler] that sleeps
** for a specified amount of time when a table is locked.  ^The handler
** will sleep multiple times until at least "ms" milliseconds of sleeping
**
** See also:  [PRAGMA busy_timeout]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_busy_timeout(sqlite3*, int ms);

/*
** CAPI3REF: Convenience Routines For Running Queries
** METHOD: sqlite3
**
** This is a legacy interface that is preserved for backwards compatibility.
** Use of this interface is not recommended.
**
** reflected in subsequent calls to [sqlite3_errcode()] or
** [sqlite3_errmsg()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_get_table(
  sqlite3 *db,          /* An open database */
  const char *zSql,     /* SQL to be evaluated */
  char ***pazResult,    /* Results of the query */
  int *pnRow,           /* Number of result rows written here */
  int *pnColumn,        /* Number of result columns written here */
  char **pzErrmsg       /* Error msg written here */
);
SQLITE_API void SQLITE_STDCALL sqlite3_free_table(char **result);

/*
** CAPI3REF: Formatted String Printing Functions
**
** These routines are work-alikes of the "printf()" family of functions
** from the standard C library.
** These routines understand most of the common K&R formatting options,
** plus some additional non-standard formats, detailed below.
** Note that some of the more obscure formatting options from recent
** C-library standards are omitted from this implementation.
**
** ^The sqlite3_mprintf() and sqlite3_vmprintf() routines write their
** results into memory obtained from [sqlite3_malloc()].
** The strings returned by these two routines should be
** These routines all implement some additional formatting
** options that are useful for constructing SQL statements.
** All of the usual printf() formatting options apply.  In addition, there
** is are "%q", "%Q", "%w" and "%z" options.
**
** ^(The %q option works like %s in that it substitutes a nul-terminated
** string from the argument list.  But %q also doubles every '\'' character.
** %q is designed for use inside a string literal.)^  By doubling each '\''
** The code above will render a correct SQL statement in the zSQL
** variable even if the zText variable is a NULL pointer.
**
** ^(The "%w" formatting option is like "%q" except that it expects to
** be contained within double-quotes instead of single quotes, and it
** escapes the double-quote character instead of the single-quote
** character.)^  The "%w" formatting option is intended for safely inserting
** table and column names into a constructed SQL statement.
**
** ^(The "%z" formatting option works like "%s" but with the
** addition that after the string has been read and copied into
** the result, [sqlite3_free()] is called on the input string.)^
*/
SQLITE_API char *SQLITE_CDECL sqlite3_mprintf(const char*,...);
SQLITE_API char *SQLITE_STDCALL sqlite3_vmprintf(const char*, va_list);
SQLITE_API char *SQLITE_CDECL sqlite3_snprintf(int,char*,const char*, ...);
SQLITE_API char *SQLITE_STDCALL sqlite3_vsnprintf(int,char*,const char*, va_list);

/*
** CAPI3REF: Memory Allocation Subsystem
**
** a block of memory after it has been released using
** [sqlite3_free()] or [sqlite3_realloc()].
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_malloc(int);
SQLITE_API void *SQLITE_STDCALL sqlite3_malloc64(sqlite3_uint64);
SQLITE_API void *SQLITE_STDCALL sqlite3_realloc(void*, int);
SQLITE_API void *SQLITE_STDCALL sqlite3_realloc64(void*, sqlite3_uint64);
SQLITE_API void SQLITE_STDCALL sqlite3_free(void*);
SQLITE_API sqlite3_uint64 SQLITE_STDCALL sqlite3_msize(void*);

/*
** CAPI3REF: Memory Allocator Statistics
**
** by [sqlite3_memory_highwater(1)] is the high-water mark
** prior to the reset.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_memory_used(void);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_memory_highwater(int resetFlag);

/*
** CAPI3REF: Pseudo-Random Number Generator
**
** internally and without recourse to the [sqlite3_vfs] xRandomness
** method.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_randomness(int N, void *P);

/*
** CAPI3REF: Compile-Time Authorization Callbacks
** METHOD: sqlite3
**
** ^This routine registers an authorizer callback with a particular
** [database connection], supplied in the first argument.
** ^The authorizer callback is invoked as SQL statements are being compiled
** as stated in the previous paragraph, sqlite3_step() invokes
** sqlite3_prepare_v2() to reprepare a statement after a schema change.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_set_authorizer(
  sqlite3*,
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*),
  void *pUserData
);

/*
** CAPI3REF: Tracing And Profiling Functions
** METHOD: sqlite3
**
** These routines register callback functions that can be used for
** tracing and profiling the execution of SQL statements.
**
** sqlite3_profile() function is considered experimental and is
** subject to change in future versions of SQLite.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_trace(sqlite3*, void(*xTrace)(void*,const char*), void*);
SQLITE_API SQLITE_EXPERIMENTAL void *SQLITE_STDCALL sqlite3_profile(sqlite3*,
   void(*xProfile)(void*,const char*,sqlite3_uint64), void*);

/*
** CAPI3REF: Query Progress Callbacks
** METHOD: sqlite3
**
** ^The sqlite3_progress_handler(D,N,X,P) interface causes the callback
** function X to be invoked periodically during long running calls to
** [sqlite3_exec()], [sqlite3_step()] and [sqlite3_get_table()] for
** database connections for the meaning of "modify" in this paragraph.
**
*/
SQLITE_API void SQLITE_STDCALL sqlite3_progress_handler(sqlite3*, int, int(*)(void*), void*);

/*
** CAPI3REF: Opening A New Database Connection
** CONSTRUCTOR: sqlite3
**
** ^These routines open an SQLite database file as specified by the 
** filename argument. ^The filename argument is interpreted as UTF-8 for
** sqlite3_open() and sqlite3_open_v2() and as UTF-16 in the native byte
**
** See also: [sqlite3_temp_directory]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_open(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
);
SQLITE_API int SQLITE_STDCALL sqlite3_open16(
  const void *filename,   /* Database filename (UTF-16) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
);
SQLITE_API int SQLITE_STDCALL sqlite3_open_v2(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
** VFS method, then the behavior of this routine is undefined and probably
** undesirable.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_uri_parameter(const char *zFilename, const char *zParam);
SQLITE_API int SQLITE_STDCALL sqlite3_uri_boolean(const char *zFile, const char *zParam, int bDefault);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_uri_int64(const char*, const char*, sqlite3_int64);


/*
** CAPI3REF: Error Codes And Messages
** METHOD: sqlite3
**
** ^If the most recent sqlite3_* API call associated with 
** [database connection] D failed, then the sqlite3_errcode(D) interface
** returns the numeric [result code] or [extended result code] for that
** API call.
** If the most recent API call was successful,
** then the return value from sqlite3_errcode() is undefined.
** ^The sqlite3_extended_errcode()
** interface is the same except that it always returns the 
** [extended result code] even when extended result codes are
** disabled.
**
** was invoked incorrectly by the application.  In that case, the
** error code and message may or may not be set.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_errcode(sqlite3 *db);
SQLITE_API int SQLITE_STDCALL sqlite3_extended_errcode(sqlite3 *db);
SQLITE_API const char *SQLITE_STDCALL sqlite3_errmsg(sqlite3*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_errmsg16(sqlite3*);
SQLITE_API const char *SQLITE_STDCALL sqlite3_errstr(int);

/*
** CAPI3REF: Prepared Statement Object
** KEYWORDS: {prepared statement} {prepared statements}
**
** An instance of this object represents a single SQL statement that
** has been compiled into binary form and is ready to be evaluated.
**
** Think of each SQL statement as a separate computer program.  The
** original SQL text is source code.  A prepared statement object 
** is the compiled object code.  All SQL must be converted into a
** prepared statement before it can be run.
**
** The life-cycle of a prepared statement object usually goes like this:
**
** <ol>
** <li> Create the prepared statement object using [sqlite3_prepare_v2()].
** <li> Bind values to [parameters] using the sqlite3_bind_*()
**      interfaces.
** <li> Run the SQL by calling [sqlite3_step()] one or more times.
** <li> Reset the prepared statement using [sqlite3_reset()] then go back
**      to step 2.  Do this zero or more times.
** <li> Destroy the object using [sqlite3_finalize()].
** </ol>
*/
typedef struct sqlite3_stmt sqlite3_stmt;

/*
** CAPI3REF: Run-time Limits
** METHOD: sqlite3
**
** ^(This interface allows the size of various constructs to be limited
** on a connection by connection basis.  The first parameter is the
** [database connection] whose limit is to be set or queried.  The
**
** New run-time limit categories may be added in future releases.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_limit(sqlite3*, int id, int newVal);

/*
** CAPI3REF: Run-Time Limit Categories
** KEYWORDS: {limit category} {*limit categories}
/*
** CAPI3REF: Compiling An SQL Statement
** KEYWORDS: {SQL statement compiler}
** METHOD: sqlite3
** CONSTRUCTOR: sqlite3_stmt
**
** To execute an SQL query, it must first be compiled into a byte-code
** program using one of these routines.
**
** interfaces use UTF-8, and sqlite3_prepare16() and sqlite3_prepare16_v2()
** use UTF-16.
**
** ^If the nByte argument is negative, then zSql is read up to the
** first zero terminator. ^If nByte is positive, then it is the
** number of bytes read from zSql.  ^If nByte is zero, then no prepared
** statement is generated.
** If the caller knows that the supplied string is nul-terminated, then
** there is a small performance advantage to passing an nByte parameter that
** is the number of bytes in the input string <i>including</i>
** the nul-terminator.
**
** ^If pzTail is not NULL then *pzTail is made to point to the first byte
** past the end of the first SQL statement in zSql.  These routines only
** compile the first statement in zSql, so *pzTail is left pointing to
** </li>
** </ol>
*/
SQLITE_API int SQLITE_STDCALL sqlite3_prepare(
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);
SQLITE_API int SQLITE_STDCALL sqlite3_prepare_v2(
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);
SQLITE_API int SQLITE_STDCALL sqlite3_prepare16(
  sqlite3 *db,            /* Database handle */
  const void *zSql,       /* SQL statement, UTF-16 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const void **pzTail     /* OUT: Pointer to unused portion of zSql */
);
SQLITE_API int SQLITE_STDCALL sqlite3_prepare16_v2(
  sqlite3 *db,            /* Database handle */
  const void *zSql,       /* SQL statement, UTF-16 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */

/*
** CAPI3REF: Retrieving Statement SQL
** METHOD: sqlite3_stmt
**
** ^This interface can be used to retrieve a saved copy of the original
** SQL text used to create a [prepared statement] if that statement was
** compiled using either [sqlite3_prepare_v2()] or [sqlite3_prepare16_v2()].
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_sql(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Determine If An SQL Statement Writes The Database
** METHOD: sqlite3_stmt
**
** ^The sqlite3_stmt_readonly(X) interface returns true (non-zero) if
** and only if the [prepared statement] X makes no direct changes to
** the content of the database file.
** change the configuration of a database connection, they do not make 
** changes to the content of the database files on disk.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_readonly(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Determine If A Prepared Statement Has Been Reset
** METHOD: sqlite3_stmt
**
** ^The sqlite3_stmt_busy(S) interface returns true (non-zero) if the
** [prepared statement] S has been stepped at least once using 
** [sqlite3_step(S)] but has not run to completion and/or has not 
** for example, in diagnostic routines to search for prepared 
** statements that are holding a transaction open.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_busy(sqlite3_stmt*);

/*
** CAPI3REF: Dynamically Typed Value Object
** KEYWORDS: {protected sqlite3_value} {unprotected sqlite3_value}
** CAPI3REF: Binding Values To Prepared Statements
** KEYWORDS: {host parameter} {host parameters} {host parameter name}
** KEYWORDS: {SQL parameter} {SQL parameters} {parameter binding}
** METHOD: sqlite3_stmt
**
** ^(In the SQL statement text input to [sqlite3_prepare_v2()] and its variants,
** literals may be replaced by a [parameter] that matches one of following
** templates:
** See also: [sqlite3_bind_parameter_count()],
** [sqlite3_bind_parameter_name()], and [sqlite3_bind_parameter_index()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_blob64(sqlite3_stmt*, int, const void*, sqlite3_uint64,
                        void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_double(sqlite3_stmt*, int, double);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_int(sqlite3_stmt*, int, int);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_int64(sqlite3_stmt*, int, sqlite3_int64);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_null(sqlite3_stmt*, int);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_text(sqlite3_stmt*,int,const char*,int,void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int, void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_text64(sqlite3_stmt*, int, const char*, sqlite3_uint64,
                         void(*)(void*), unsigned char encoding);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_value(sqlite3_stmt*, int, const sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_zeroblob(sqlite3_stmt*, int, int n);

/*
** CAPI3REF: Number Of SQL Parameters
** METHOD: sqlite3_stmt
**
** ^This routine can be used to find the number of [SQL parameters]
** in a [prepared statement].  SQL parameters are tokens of the
** form "?", "?NNN", ":AAA", "$AAA", or "@AAA" that serve as
** [sqlite3_bind_parameter_name()], and
** [sqlite3_bind_parameter_index()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_bind_parameter_count(sqlite3_stmt*);

/*
** CAPI3REF: Name Of A Host Parameter
** METHOD: sqlite3_stmt
**
** ^The sqlite3_bind_parameter_name(P,N) interface returns
** the name of the N-th [SQL parameter] in the [prepared statement] P.
** ^(SQL parameters of the form "?NNN" or ":AAA" or "@AAA" or "$AAA"
** [sqlite3_bind_parameter_count()], and
** [sqlite3_bind_parameter_index()].
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_bind_parameter_name(sqlite3_stmt*, int);

/*
** CAPI3REF: Index Of A Parameter With A Given Name
** METHOD: sqlite3_stmt
**
** ^Return the index of an SQL parameter given its name.  ^The
** index value returned is suitable for use as the second
** parameter to [sqlite3_bind_blob|sqlite3_bind()].  ^A zero
** [sqlite3_bind_parameter_count()], and
** [sqlite3_bind_parameter_index()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_bind_parameter_index(sqlite3_stmt*, const char *zName);

/*
** CAPI3REF: Reset All Bindings On A Prepared Statement
** METHOD: sqlite3_stmt
**
** ^Contrary to the intuition of many, [sqlite3_reset()] does not reset
** the [sqlite3_bind_blob | bindings] on a [prepared statement].
** ^Use this routine to reset all host parameters to NULL.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_clear_bindings(sqlite3_stmt*);

/*
** CAPI3REF: Number Of Columns In A Result Set
** METHOD: sqlite3_stmt
**
** ^Return the number of columns in the result set returned by the
** [prepared statement]. ^This routine returns 0 if pStmt is an SQL
** statement that does not return data (for example an [UPDATE]).
**
** See also: [sqlite3_data_count()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_column_count(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Column Names In A Result Set
** METHOD: sqlite3_stmt
**
** ^These routines return the name assigned to a particular column
** in the result set of a [SELECT] statement.  ^The sqlite3_column_name()
** interface returns a pointer to a zero-terminated UTF-8 string
** then the name of the column is unspecified and may change from
** one release of SQLite to the next.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_name(sqlite3_stmt*, int N);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_name16(sqlite3_stmt*, int N);

/*
** CAPI3REF: Source Of Data In A Query Result
** METHOD: sqlite3_stmt
**
** ^These routines provide a means to determine the database, table, and
** table column that is the origin of a particular result column in
** [SELECT] statement.
** for the same [prepared statement] and result column
** at the same time then the results are undefined.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_database_name(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_database_name16(sqlite3_stmt*,int);
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_table_name(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_table_name16(sqlite3_stmt*,int);
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_origin_name(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_origin_name16(sqlite3_stmt*,int);

/*
** CAPI3REF: Declared Datatype Of A Query Result
** METHOD: sqlite3_stmt
**
** ^(The first parameter is a [prepared statement].
** If this statement is a [SELECT] statement and the Nth column of the
** returned result set of that [SELECT] is a table column (not an
** is associated with individual values, not with the containers
** used to hold those values.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_decltype(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_decltype16(sqlite3_stmt*,int);

/*
** CAPI3REF: Evaluate An SQL Statement
** METHOD: sqlite3_stmt
**
** After a [prepared statement] has been prepared using either
** [sqlite3_prepare_v2()] or [sqlite3_prepare16_v2()] or one of the legacy
** interfaces [sqlite3_prepare()] or [sqlite3_prepare16()], this function
** then the more specific [error codes] are returned directly
** by sqlite3_step().  The use of the "v2" interface is recommended.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_step(sqlite3_stmt*);

/*
** CAPI3REF: Number of columns in a result set
** METHOD: sqlite3_stmt
**
** ^The sqlite3_data_count(P) interface returns the number of columns in the
** current row of the result set of [prepared statement] P.
** ^If prepared statement P does not have results ready to return
**
** See also: [sqlite3_column_count()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_data_count(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Fundamental Datatypes
** KEYWORDS: SQLITE_TEXT
/*
** CAPI3REF: Result Values From A Query
** KEYWORDS: {column access functions}
** METHOD: sqlite3_stmt
**
** These routines form the "result set" interface.
**
** ^These routines return information about a single column of the current
** pointer.  Subsequent calls to [sqlite3_errcode()] will return
** [SQLITE_NOMEM].)^
*/
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_blob(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_bytes(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_bytes16(sqlite3_stmt*, int iCol);
SQLITE_API double SQLITE_STDCALL sqlite3_column_double(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_int(sqlite3_stmt*, int iCol);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_column_int64(sqlite3_stmt*, int iCol);
SQLITE_API const unsigned char *SQLITE_STDCALL sqlite3_column_text(sqlite3_stmt*, int iCol);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_text16(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_type(sqlite3_stmt*, int iCol);
SQLITE_API sqlite3_value *SQLITE_STDCALL sqlite3_column_value(sqlite3_stmt*, int iCol);

/*
** CAPI3REF: Destroy A Prepared Statement Object
** DESTRUCTOR: sqlite3_stmt
**
** ^The sqlite3_finalize() function is called to delete a [prepared statement].
** ^If the most recent evaluation of the statement encountered no errors
** or if the statement is never been evaluated, then sqlite3_finalize() returns
** statement after it has been finalized can result in undefined and
** undesirable behavior such as segfaults and heap corruption.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_finalize(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Reset A Prepared Statement Object
** METHOD: sqlite3_stmt
**
** The sqlite3_reset() function is called to reset a [prepared statement]
** object back to its initial state, ready to be re-executed.
** ^Any SQL statement variables that had values bound to them using
** ^The [sqlite3_reset(S)] interface does not change the values
** of any [sqlite3_bind_blob|bindings] on the [prepared statement] S.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_reset(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Create Or Redefine SQL Functions
** KEYWORDS: {function creation routines}
** KEYWORDS: {application-defined SQL function}
** KEYWORDS: {application-defined SQL functions}
** METHOD: sqlite3
**
** ^These functions (collectively known as "function creation routines")
** are used to add SQL functions or aggregates or to redefine the behavior
** of existing SQL functions or aggregates.  The only differences between
** close the database connection nor finalize or reset the prepared
** statement in which the function is running.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_function(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_function16(
  sqlite3 *db,
  const void *zFunctionName,
  int nArg,
  int eTextRep,
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_function_v2(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
** These functions are [deprecated].  In order to maintain
** backwards compatibility with older code, these functions continue 
** to be supported.  However, new applications should avoid
** the use of these functions.  To encourage programmers to avoid
** these functions, we will not explain what they do.
*/
#ifndef SQLITE_OMIT_DEPRECATED
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_aggregate_count(sqlite3_context*);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_expired(sqlite3_stmt*);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_transfer_bindings(sqlite3_stmt*, sqlite3_stmt*);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_global_recover(void);
SQLITE_API SQLITE_DEPRECATED void SQLITE_STDCALL sqlite3_thread_cleanup(void);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_memory_alarm(void(*)(void*,sqlite3_int64,int),
                      void*,sqlite3_int64);
#endif

/*
** CAPI3REF: Obtaining SQL Function Parameter Values
** METHOD: sqlite3_value
**
** The C-language implementation of SQL functions and aggregates uses
** this set of interface routines to access the parameter values on
** the function or aggregate.
** These routines must be called from the same thread as
** the SQL function that supplied the [sqlite3_value*] parameters.
*/
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_blob(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_bytes(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_bytes16(sqlite3_value*);
SQLITE_API double SQLITE_STDCALL sqlite3_value_double(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_int(sqlite3_value*);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_value_int64(sqlite3_value*);
SQLITE_API const unsigned char *SQLITE_STDCALL sqlite3_value_text(sqlite3_value*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_text16(sqlite3_value*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_text16le(sqlite3_value*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_text16be(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_type(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_numeric_type(sqlite3_value*);

/*
** CAPI3REF: Obtain Aggregate Function Context
** METHOD: sqlite3_context
**
** Implementations of aggregate SQL functions use this
** routine to allocate memory for storing their state.
**
** This routine must be called from the same thread in which
** the aggregate SQL function is running.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_aggregate_context(sqlite3_context*, int nBytes);

/*
** CAPI3REF: User Data For Functions
** METHOD: sqlite3_context
**
** ^The sqlite3_user_data() interface returns a copy of
** the pointer that was the pUserData parameter (the 5th parameter)
** of the [sqlite3_create_function()]
** This routine must be called from the same thread in which
** the application-defined function is running.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_user_data(sqlite3_context*);

/*
** CAPI3REF: Database Connection For Functions
** METHOD: sqlite3_context
**
** ^The sqlite3_context_db_handle() interface returns a copy of
** the pointer to the [database connection] (the 1st parameter)
** of the [sqlite3_create_function()]
** and [sqlite3_create_function16()] routines that originally
** registered the application defined function.
*/
SQLITE_API sqlite3 *SQLITE_STDCALL sqlite3_context_db_handle(sqlite3_context*);

/*
** CAPI3REF: Function Auxiliary Data
** METHOD: sqlite3_context
**
** These functions may be used by (non-aggregate) SQL functions to
** associate metadata with argument values. If the same value is passed to
** multiple invocations of the same SQL function during query execution, under
** These routines must be called from the same thread in which
** the SQL function is running.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_get_auxdata(sqlite3_context*, int N);
SQLITE_API void SQLITE_STDCALL sqlite3_set_auxdata(sqlite3_context*, int N, void*, void (*)(void*));


/*
** CAPI3REF: Constants Defining Special Destructor Behavior

/*
** CAPI3REF: Setting The Result Of An SQL Function
** METHOD: sqlite3_context
**
** These routines are used by the xFunc or xFinal callbacks that
** implement SQL functions and aggregates.  See
** [sqlite3_create_function()] and [sqlite3_create_function16()]
** than the one containing the application-defined function that received
** the [sqlite3_context] pointer, the results are undefined.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_result_blob(sqlite3_context*, const void*, int, void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_blob64(sqlite3_context*,const void*,
                           sqlite3_uint64,void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_double(sqlite3_context*, double);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error(sqlite3_context*, const char*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error16(sqlite3_context*, const void*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error_toobig(sqlite3_context*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error_nomem(sqlite3_context*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error_code(sqlite3_context*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_int(sqlite3_context*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_int64(sqlite3_context*, sqlite3_int64);
SQLITE_API void SQLITE_STDCALL sqlite3_result_null(sqlite3_context*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_text(sqlite3_context*, const char*, int, void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_text64(sqlite3_context*, const char*,sqlite3_uint64,
                           void(*)(void*), unsigned char encoding);
SQLITE_API void SQLITE_STDCALL sqlite3_result_text16(sqlite3_context*, const void*, int, void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_text16le(sqlite3_context*, const void*, int,void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_text16be(sqlite3_context*, const void*, int,void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_value(sqlite3_context*, sqlite3_value*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_zeroblob(sqlite3_context*, int n);

/*
** CAPI3REF: Define New Collating Sequences
** METHOD: sqlite3
**
** ^These functions add, remove, or modify a [collation] associated
** with the [database connection] specified as the first argument.
**
**
** See also:  [sqlite3_collation_needed()] and [sqlite3_collation_needed16()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation(
  sqlite3*, 
  const char *zName, 
  int eTextRep, 
  void *pArg,
  int(*xCompare)(void*,int,const void*,int,const void*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation_v2(
  sqlite3*, 
  const char *zName, 
  int eTextRep, 
  void *pArg,
  int(*xCompare)(void*,int,const void*,int,const void*),
  void(*xDestroy)(void*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation16(
  sqlite3*, 
  const void *zName,
  int eTextRep, 
  void *pArg,

/*
** CAPI3REF: Collation Needed Callbacks
** METHOD: sqlite3
**
** ^To avoid having to register all collation sequences before a database
** can be used, a single callback function may be registered with the
** [database connection] to be invoked whenever an undefined collation
** [sqlite3_create_collation()], [sqlite3_create_collation16()], or
** [sqlite3_create_collation_v2()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_collation_needed(
  sqlite3*, 
  void*, 
  void(*)(void*,sqlite3*,int eTextRep,const char*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_collation_needed16(
  sqlite3*, 
  void*,
  void(*)(void*,sqlite3*,int eTextRep,const void*)
);
** The code to implement this API is not available in the public release
** of SQLite.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_key(
  sqlite3 *db,                   /* Database to be rekeyed */
  const void *pKey, int nKey     /* The key */
);
SQLITE_API int SQLITE_STDCALL sqlite3_key_v2(
  sqlite3 *db,                   /* Database to be rekeyed */
  const char *zDbName,           /* Name of the database */
  const void *pKey, int nKey     /* The key */
);
** The code to implement this API is not available in the public release
** of SQLite.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rekey(
  sqlite3 *db,                   /* Database to be rekeyed */
  const void *pKey, int nKey     /* The new key */
);
SQLITE_API int SQLITE_STDCALL sqlite3_rekey_v2(
  sqlite3 *db,                   /* Database to be rekeyed */
  const char *zDbName,           /* Name of the database */
  const void *pKey, int nKey     /* The new key */
);
** Specify the activation key for a SEE database.  Unless 
** activated, none of the SEE routines will work.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_activate_see(
  const char *zPassPhrase        /* Activation phrase */
);
#endif

** Specify the activation key for a CEROD database.  Unless 
** activated, none of the CEROD routines will work.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_activate_cerod(
  const char *zPassPhrase        /* Activation phrase */
);
#endif

** all, then the behavior of sqlite3_sleep() may deviate from the description
** in the previous paragraphs.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_sleep(int);

/*
** CAPI3REF: Name Of The Folder Holding Temporary Files
**
/*
** CAPI3REF: Test For Auto-Commit Mode
** KEYWORDS: {autocommit mode}
** METHOD: sqlite3
**
** ^The sqlite3_get_autocommit() interface returns non-zero or
** zero if the given database connection is or is not in autocommit mode,
** respectively.  ^Autocommit mode is on by default.
** connection while this routine is running, then the return value
** is undefined.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_get_autocommit(sqlite3*);

/*
** CAPI3REF: Find The Database Handle Of A Prepared Statement
** METHOD: sqlite3_stmt
**
** ^The sqlite3_db_handle interface returns the [database connection] handle
** to which a [prepared statement] belongs.  ^The [database connection]
** returned by sqlite3_db_handle is the same [database connection]
** to the [sqlite3_prepare_v2()] call (or its variants) that was used to
** create the statement in the first place.
*/
SQLITE_API sqlite3 *SQLITE_STDCALL sqlite3_db_handle(sqlite3_stmt*);

/*
** CAPI3REF: Return The Filename For A Database Connection
** METHOD: sqlite3
**
** ^The sqlite3_db_filename(D,N) interface returns a pointer to a filename
** associated with database N of connection D.  ^The main database file
** has the name "main".  If there is no attached database N on the database
** will be an absolute pathname, even if the filename used
** to open the database originally was a URI or relative pathname.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_db_filename(sqlite3 *db, const char *zDbName);

/*
** CAPI3REF: Determine if a database is read-only
** METHOD: sqlite3
**
** ^The sqlite3_db_readonly(D,N) interface returns 1 if the database N
** of connection D is read-only, 0 if it is read/write, or -1 if N is not
** the name of a database on connection D.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_readonly(sqlite3 *db, const char *zDbName);

/*
** CAPI3REF: Find the next prepared statement
** METHOD: sqlite3
**
** ^This interface returns a pointer to the next [prepared statement] after
** pStmt associated with the [database connection] pDb.  ^If pStmt is NULL
** then this interface returns a pointer to the first prepared statement
** [sqlite3_next_stmt(D,S)] must refer to an open database
** connection and in particular must not be a NULL pointer.
*/
SQLITE_API sqlite3_stmt *SQLITE_STDCALL sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt);

/*
** CAPI3REF: Commit And Rollback Notification Callbacks
** METHOD: sqlite3
**
** ^The sqlite3_commit_hook() interface registers a callback
** function to be invoked whenever a transaction is [COMMIT | committed].
** ^Any callback set by a previous call to sqlite3_commit_hook()
**
** See also the [sqlite3_update_hook()] interface.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_commit_hook(sqlite3*, int(*)(void*), void*);
SQLITE_API void *SQLITE_STDCALL sqlite3_rollback_hook(sqlite3*, void(*)(void *), void*);

/*
** CAPI3REF: Data Change Notification Callbacks
** METHOD: sqlite3
**
** ^The sqlite3_update_hook() interface registers a callback function
** with the [database connection] identified by the first argument
** to be invoked whenever a row is updated, inserted or deleted in
** See also the [sqlite3_commit_hook()] and [sqlite3_rollback_hook()]
** interfaces.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_update_hook(
  sqlite3*, 
  void(*)(void *,int ,char const *,char const *,sqlite3_int64),
  void*
);
** future releases of SQLite.  Applications that care about shared
** cache setting should set it explicitly.
**
** Note: This method is disabled on MacOS X 10.7 and iOS version 5.0
** and will always return SQLITE_MISUSE. On those systems, 
** shared cache mode should be enabled per-database connection via 
** [sqlite3_open_v2()] with [SQLITE_OPEN_SHAREDCACHE].
**
** This interface is threadsafe on processors where writing a
** 32-bit integer is atomic.
**
** See Also:  [SQLite Shared-Cache Mode]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_enable_shared_cache(int);

/*
** CAPI3REF: Attempt To Free Heap Memory
**
**
** See also: [sqlite3_db_release_memory()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_release_memory(int);

/*
** CAPI3REF: Free Memory Used By A Database Connection
** METHOD: sqlite3
**
** ^The sqlite3_db_release_memory(D) interface attempts to free as much heap
** memory as possible from database connection D. Unlike the
** [sqlite3_release_memory()] interface, this interface is in effect even
**
** See also: [sqlite3_release_memory()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_release_memory(sqlite3*);

/*
** CAPI3REF: Impose A Limit On Heap Size
**
** The circumstances under which SQLite will enforce the soft heap limit may
** changes in future releases of SQLite.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_soft_heap_limit64(sqlite3_int64 N);

/*
** CAPI3REF: Deprecated Soft Heap Limit Interface
** DEPRECATED
** only.  All new applications should use the
** [sqlite3_soft_heap_limit64()] interface rather than this one.
*/
SQLITE_API SQLITE_DEPRECATED void SQLITE_STDCALL sqlite3_soft_heap_limit(int N);


/*
** CAPI3REF: Extract Metadata About A Column Of A Table
** METHOD: sqlite3
**
** ^(The sqlite3_table_column_metadata(X,D,T,C,....) routine returns
** information about column C of table T in database D
** on [database connection] X.)^  ^The sqlite3_table_column_metadata()
** parsed, if that has not already been done, and returns an error if
** any errors are encountered while loading the schema.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  const char *zDbName,        /* Database name or NULL */
  const char *zTableName,     /* Table name */
  const char *zColumnName,    /* Column name */

/*
** CAPI3REF: Load An Extension
** METHOD: sqlite3
**
** ^This interface loads an SQLite extension library from the named file.
**
** ^The sqlite3_load_extension() interface attempts to load an
**
** See also the [load_extension() SQL function].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_load_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  const char *zFile,    /* Name of the shared library containing extension */
  const char *zProc,    /* Entry point.  Derived from zFile if 0 */
  char **pzErrMsg       /* Put error message here if not 0 */

/*
** CAPI3REF: Enable Or Disable Extension Loading
** METHOD: sqlite3
**
** ^So as not to open security holes in older applications that are
** unprepared to deal with [extension loading], and as a means of disabling
** [extension loading] while evaluating user-entered SQL, the following API
** to turn extension loading on and call it with onoff==0 to turn
** it back off again.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_enable_load_extension(sqlite3 *db, int onoff);

/*
** CAPI3REF: Automatically Load Statically Linked Extensions
**
** See also: [sqlite3_reset_auto_extension()]
** and [sqlite3_cancel_auto_extension()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_auto_extension(void (*xEntryPoint)(void));

/*
** CAPI3REF: Cancel Automatic Extension Loading
**
** unregistered and it returns 0 if X was not on the list of initialization
** routines.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_cancel_auto_extension(void (*xEntryPoint)(void));

/*
** CAPI3REF: Reset Automatic Extension Loading
**
** ^This interface disables all automatic extensions previously
** registered using [sqlite3_auto_extension()].
*/
SQLITE_API void SQLITE_STDCALL sqlite3_reset_auto_extension(void);

/*
** The interface to the virtual-table mechanism is currently considered
** to be experimental.  The interface might change in incompatible ways.

/*
** CAPI3REF: Register A Virtual Table Implementation
** METHOD: sqlite3
**
** ^These routines are used to register a new [virtual table module] name.
** ^Module names must be registered before
** creating a new [virtual table] using the module and before using a
** interface is equivalent to sqlite3_create_module_v2() with a NULL
** destructor.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_module(
  sqlite3 *db,               /* SQLite connection to register module with */
  const char *zName,         /* Name of the module */
  const sqlite3_module *p,   /* Methods for the module */
  void *pClientData          /* Client data for xCreate/xConnect */
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_module_v2(
  sqlite3 *db,               /* SQLite connection to register module with */
  const char *zName,         /* Name of the module */
  const sqlite3_module *p,   /* Methods for the module */
  void *pClientData,         /* Client data for xCreate/xConnect */
*/
struct sqlite3_vtab {
  const sqlite3_module *pModule;  /* The module for this virtual table */
  int nRef;                       /* Number of open cursors */
  char *zErrMsg;                  /* Error message from sqlite3_mprintf() */
  /* Virtual table implementations will typically add additional fields */
};

** to declare the format (the names and datatypes of the columns) of
** the virtual tables they implement.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_declare_vtab(sqlite3*, const char *zSQL);

/*
** CAPI3REF: Overload A Function For A Virtual Table
** METHOD: sqlite3
**
** ^(Virtual tables can provide alternative implementations of functions
** using the [xFindFunction] method of the [virtual table module].  
** But global versions of those functions
** purpose is to be a placeholder function that can be overloaded
** by a [virtual table].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_overload_function(sqlite3*, const char *zFuncName, int nArg);

/*
** The interface to the virtual-table mechanism defined above (back up
** to a comment remarkably similar to this one) is currently considered

/*
** CAPI3REF: Open A BLOB For Incremental I/O
** METHOD: sqlite3
** CONSTRUCTOR: sqlite3_blob
**
** ^(This interfaces opens a [BLOB handle | handle] to the BLOB located
** in row iRow, column zColumn, table zTable in database zDb;
** in other words, the same BLOB that would be selected by:
** To avoid a resource leak, every open [BLOB handle] should eventually
** be released by a call to [sqlite3_blob_close()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_open(
  sqlite3*,
  const char *zDb,
  const char *zTable,
  const char *zColumn,

/*
** CAPI3REF: Move a BLOB Handle to a New Row
** METHOD: sqlite3_blob
**
** ^This function is used to move an existing blob handle so that it points
** to a different row of the same database table. ^The new row is identified
** by the rowid value passed as the second argument. Only the row can be
**
** ^This function sets the database handle error code and message.
*/
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_blob_reopen(sqlite3_blob *, sqlite3_int64);

/*
** CAPI3REF: Close A BLOB Handle
** DESTRUCTOR: sqlite3_blob
**
** ^This function closes an open [BLOB handle]. ^(The BLOB handle is closed
** unconditionally.  Even if this routine returns an error code, the 
** handle is still closed.)^
** is passed a valid open blob handle, the values returned by the 
** sqlite3_errcode() and sqlite3_errmsg() functions are set before returning.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_close(sqlite3_blob *);

/*
** CAPI3REF: Return The Size Of An Open BLOB
** METHOD: sqlite3_blob
**
** ^Returns the size in bytes of the BLOB accessible via the 
** successfully opened [BLOB handle] in its only argument.  ^The
** incremental blob I/O routines can only read or overwriting existing
** been closed by [sqlite3_blob_close()].  Passing any other pointer in
** to this routine results in undefined and probably undesirable behavior.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_bytes(sqlite3_blob *);

/*
** CAPI3REF: Read Data From A BLOB Incrementally
** METHOD: sqlite3_blob
**
** ^(This function is used to read data from an open [BLOB handle] into a
** caller-supplied buffer. N bytes of data are copied into buffer Z
** from the open BLOB, starting at offset iOffset.)^
**
** See also: [sqlite3_blob_write()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_read(sqlite3_blob *, void *Z, int N, int iOffset);

/*
** CAPI3REF: Write Data Into A BLOB Incrementally
** METHOD: sqlite3_blob
**
** ^(This function is used to write data into an open [BLOB handle] from a
** caller-supplied buffer. N bytes of data are copied from the buffer Z
** into the open BLOB, starting at offset iOffset.)^
**
** See also: [sqlite3_blob_read()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_write(sqlite3_blob *, const void *z, int n, int iOffset);

/*
** CAPI3REF: Virtual File System Objects
**
** ^(If the default VFS is unregistered, another VFS is chosen as
** the default.  The choice for the new VFS is arbitrary.)^
*/
SQLITE_API sqlite3_vfs *SQLITE_STDCALL sqlite3_vfs_find(const char *zVfsName);
SQLITE_API int SQLITE_STDCALL sqlite3_vfs_register(sqlite3_vfs*, int makeDflt);
SQLITE_API int SQLITE_STDCALL sqlite3_vfs_unregister(sqlite3_vfs*);

/*
** CAPI3REF: Mutexes
**
**
** See also: [sqlite3_mutex_held()] and [sqlite3_mutex_notheld()].
*/
SQLITE_API sqlite3_mutex *SQLITE_STDCALL sqlite3_mutex_alloc(int);
SQLITE_API void SQLITE_STDCALL sqlite3_mutex_free(sqlite3_mutex*);
SQLITE_API void SQLITE_STDCALL sqlite3_mutex_enter(sqlite3_mutex*);
SQLITE_API int SQLITE_STDCALL sqlite3_mutex_try(sqlite3_mutex*);
SQLITE_API void SQLITE_STDCALL sqlite3_mutex_leave(sqlite3_mutex*);

/*
** CAPI3REF: Mutex Methods Object
**
** interface should also return 1 when given a NULL pointer.
*/
#ifndef NDEBUG
SQLITE_API int SQLITE_STDCALL sqlite3_mutex_held(sqlite3_mutex*);
SQLITE_API int SQLITE_STDCALL sqlite3_mutex_notheld(sqlite3_mutex*);
#endif

/*
** CAPI3REF: Mutex Types

/*
** CAPI3REF: Retrieve the mutex for a database connection
** METHOD: sqlite3
**
** ^This interface returns a pointer the [sqlite3_mutex] object that 
** serializes access to the [database connection] given in the argument
** when the [threading mode] is Serialized.
** ^If the [threading mode] is Single-thread or Multi-thread then this
** routine returns a NULL pointer.
*/
SQLITE_API sqlite3_mutex *SQLITE_STDCALL sqlite3_db_mutex(sqlite3*);

/*
** CAPI3REF: Low-Level Control Of Database Files
** METHOD: sqlite3
**
** ^The [sqlite3_file_control()] interface makes a direct call to the
** xFileControl method for the [sqlite3_io_methods] object associated
** with a particular database identified by the second argument. ^The
**
** See also: [SQLITE_FCNTL_LOCKSTATE]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_file_control(sqlite3*, const char *zDbName, int op, void*);

/*
** CAPI3REF: Testing Interface
**
** Unlike most of the SQLite API, this function is not guaranteed to
** operate consistently from one release to the next.
*/
SQLITE_API int SQLITE_CDECL sqlite3_test_control(int op, ...);

/*
** CAPI3REF: Testing Interface Operation Codes
**
#define SQLITE_TESTCTRL_BYTEORDER               22
#define SQLITE_TESTCTRL_ISINIT                  23
#define SQLITE_TESTCTRL_SORTER_MMAP             24
#define SQLITE_TESTCTRL_IMPOSTER                25
#define SQLITE_TESTCTRL_LAST                    25

/*
** CAPI3REF: SQLite Runtime Status
**
** ^These interfaces are used to retrieve runtime status information
** about the performance of SQLite, and optionally to reset various
** highwater marks.  ^The first argument is an integer code for
** the specific parameter to measure.  ^(Recognized integer codes
** are of the form [status parameters | SQLITE_STATUS_...].)^
** ^(Other parameters record only the highwater mark and not the current
** value.  For these latter parameters nothing is written into *pCurrent.)^
**
** ^The sqlite3_status() and sqlite3_status64() routines return
** SQLITE_OK on success and a non-zero [error code] on failure.
**
** If either the current value or the highwater mark is too large to
** be represented by a 32-bit integer, then the values returned by
** sqlite3_status() are undefined.
**
** See also: [sqlite3_db_status()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_status(int op, int *pCurrent, int *pHighwater, int resetFlag);
SQLITE_API int SQLITE_STDCALL sqlite3_status64(
  int op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  int resetFlag
);


/*
** CAPI3REF: Status Parameters

/*
** CAPI3REF: Database Connection Status
** METHOD: sqlite3
**
** ^This interface is used to retrieve runtime status information 
** about a single [database connection].  ^The first argument is the
** database connection object to be interrogated.  ^The second argument
**
** See also: [sqlite3_status()] and [sqlite3_stmt_status()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_status(sqlite3*, int op, int *pCur, int *pHiwtr, int resetFlg);

/*
** CAPI3REF: Status Parameters for database connections
** KEYWORDS: {SQLITE_DBSTATUS options}

/*
** CAPI3REF: Prepared Statement Status
** METHOD: sqlite3_stmt
**
** ^(Each prepared statement maintains various
** [SQLITE_STMTSTATUS counters] that measure the number
** of times it has performed specific operations.)^  These counters can
**
** See also: [sqlite3_status()] and [sqlite3_db_status()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_status(sqlite3_stmt*, int op,int resetFlg);

/*
** CAPI3REF: Status Parameters for prepared statements
** KEYWORDS: {SQLITE_STMTSTATUS counter} {SQLITE_STMTSTATUS counters}
** is not a permanent error and does not affect the return value of
** sqlite3_backup_finish().
**
** [[sqlite3_backup_remaining()]] [[sqlite3_backup_pagecount()]]
** <b>sqlite3_backup_remaining() and sqlite3_backup_pagecount()</b>
**
** ^The sqlite3_backup_remaining() routine returns the number of pages still
** to be backed up at the conclusion of the most recent sqlite3_backup_step().
** ^The sqlite3_backup_pagecount() routine returns the total number of pages
** in the source database at the conclusion of the most recent
** sqlite3_backup_step().
** ^(The values returned by these functions are only updated by
** sqlite3_backup_step(). If the source database is modified in a way that
** changes the size of the source database or the number of pages remaining,
** those changes are not reflected in the output of sqlite3_backup_pagecount()
** and sqlite3_backup_remaining() until after the next
** sqlite3_backup_step().)^
**
** <b>Concurrent Usage of Database Handles</b>
**
** ^The source [database connection] may be used by the application for other
** same time as another thread is invoking sqlite3_backup_step() it is
** possible that they return invalid values.
*/
SQLITE_API sqlite3_backup *SQLITE_STDCALL sqlite3_backup_init(
  sqlite3 *pDest,                        /* Destination database handle */
  const char *zDestName,                 /* Destination database name */
  sqlite3 *pSource,                      /* Source database handle */
  const char *zSourceName                /* Source database name */
);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_step(sqlite3_backup *p, int nPage);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_finish(sqlite3_backup *p);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_remaining(sqlite3_backup *p);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_pagecount(sqlite3_backup *p);

/*
** CAPI3REF: Unlock Notification
** METHOD: sqlite3
**
** ^When running in shared-cache mode, a database operation may fail with
** an [SQLITE_LOCKED] error if the required locks on the shared-cache or
** individual tables within the shared-cache cannot be obtained. See
** the special "DROP TABLE/INDEX" case, the extended error code is just 
** SQLITE_LOCKED.)^
*/
SQLITE_API int SQLITE_STDCALL sqlite3_unlock_notify(
  sqlite3 *pBlocked,                          /* Waiting connection */
  void (*xNotify)(void **apArg, int nArg),    /* Callback function to invoke */
  void *pNotifyArg                            /* Argument to pass to xNotify */
);
** strings in a case-independent fashion, using the same definition of "case
** independence" that SQLite uses internally when comparing identifiers.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stricmp(const char *, const char *);
SQLITE_API int SQLITE_STDCALL sqlite3_strnicmp(const char *, const char *, int);

/*
** CAPI3REF: String Globbing
*
** Note that this routine returns zero on a match and non-zero if the strings
** do not match, the same as [sqlite3_stricmp()] and [sqlite3_strnicmp()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_strglob(const char *zGlob, const char *zStr);

/*
** CAPI3REF: Error Logging Interface
**
** a few hundred characters, it will be truncated to the length of the
** buffer.
*/
SQLITE_API void SQLITE_CDECL sqlite3_log(int iErrCode, const char *zFormat, ...);

/*
** CAPI3REF: Write-Ahead Log Commit Hook
** METHOD: sqlite3
**
** ^The [sqlite3_wal_hook()] function is used to register a callback that
** is invoked each time data is committed to a database in wal mode.
**
** [wal_autocheckpoint pragma] both invoke [sqlite3_wal_hook()] and will
** those overwrite any prior [sqlite3_wal_hook()] settings.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_wal_hook(
  sqlite3*, 
  int(*)(void *,sqlite3*,const char*,int),
  void*
);

/*
** CAPI3REF: Configure an auto-checkpoint
** METHOD: sqlite3
**
** ^The [sqlite3_wal_autocheckpoint(D,N)] is a wrapper around
** [sqlite3_wal_hook()] that causes any database on [database connection] D
** to automatically [checkpoint]
** is only necessary if the default setting is found to be suboptimal
** for a particular application.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_autocheckpoint(sqlite3 *db, int N);

/*
** CAPI3REF: Checkpoint a database
** METHOD: sqlite3
**
** ^(The sqlite3_wal_checkpoint(D,X) is equivalent to
** [sqlite3_wal_checkpoint_v2](D,X,[SQLITE_CHECKPOINT_PASSIVE],0,0).)^
**
** start a callback but which do not need the full power (and corresponding
** complication) of [sqlite3_wal_checkpoint_v2()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb);

/*
** CAPI3REF: Checkpoint a database
** METHOD: sqlite3
**
** ^(The sqlite3_wal_checkpoint_v2(D,X,M,L,C) interface runs a checkpoint
** operation on database X of [database connection] D in mode M.  Status
** information is written back into integers pointed to by L and C.)^
** ^The [PRAGMA wal_checkpoint] command can be used to invoke this interface
** from SQL.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of attached database (or NULL) */
  int eMode,                      /* SQLITE_CHECKPOINT_* value */
  int *pnLog,                     /* OUT: Size of WAL log in frames */
** this function. (See [SQLITE_VTAB_CONSTRAINT_SUPPORT].)  Further options
** may be added in the future.
*/
SQLITE_API int SQLITE_CDECL sqlite3_vtab_config(sqlite3*, int op, ...);

/*
** CAPI3REF: Virtual Table Configuration Options
**
** of the SQL statement that triggered the call to the [xUpdate] method of the
** [virtual table].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_vtab_on_conflict(sqlite3 *);

/*
** CAPI3REF: Conflict resolution modes
** KEYWORDS: {conflict resolution mode}

/*
** CAPI3REF: Prepared Statement Scan Status
** METHOD: sqlite3_stmt
**
** This interface returns information about the predicted and measured
** performance for pStmt.  Advanced applications can use this
** interface to compare the predicted and the measured performance and
**
** See also: [sqlite3_stmt_scanstatus_reset()]
*/
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,      /* Prepared statement for which info desired */
  int idx,                  /* Index of loop to report on */
  int iScanStatusOp,        /* Information desired.  SQLITE_SCANSTAT_* */
  void *pOut                /* Result written here */

/*
** CAPI3REF: Zero Scan-Status Counters
** METHOD: sqlite3_stmt
**
** ^Zero all [sqlite3_stmt_scanstatus()] related event counters.
**
** This API is only available if the library is built with pre-processor
** symbol [SQLITE_ENABLE_STMT_SCANSTATUS] defined.
*/
SQLITE_API SQLITE_EXPERIMENTAL void SQLITE_STDCALL sqlite3_stmt_scanstatus_reset(sqlite3_stmt*);


/*
** Undo the hack that converts floating point types to integer for
**
**   SELECT ... FROM <rtree> WHERE <rtree col> MATCH $zGeom(... params ...)
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_geometry_callback(
  sqlite3 *db,
  const char *zGeom,
  int (*xGeom)(sqlite3_rtree_geometry*, int, sqlite3_rtree_dbl*,int*),
  void *pContext
**
**   SELECT ... FROM <rtree> WHERE <rtree col> MATCH $zQueryFunc(... params ...)
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_query_callback(
  sqlite3 *db,
  const char *zQueryFunc,
  int (*xQueryFunc)(sqlite3_rtree_query_info*),
  void *pContext,