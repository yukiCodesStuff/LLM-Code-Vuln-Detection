	{
		MYSQLND_DEBUG * (*trace_instance_init)(const char * skip_functions[] TSRMLS_DC);
		char * (*get_backtrace)(uint max_levels, size_t * length TSRMLS_DC);
	} methods;
};

void mysqlnd_debug_trace_plugin_register(TSRMLS_D);

PHPAPI MYSQLND_DEBUG * mysqlnd_debug_init(const char * skip_functions[] TSRMLS_DC);

PHPAPI char * mysqlnd_get_backtrace(uint max_levels, size_t * length TSRMLS_DC);

#if defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER >= 1400))
#ifdef PHP_WIN32
#include "win32/time.h"
#elif defined(NETWARE)
#include <sys/timeval.h>
#include <sys/time.h>
#else
#include <sys/time.h>
#endif

#ifndef MYSQLND_PROFILING_DISABLED
#define DBG_PROFILE_TIMEVAL_TO_DOUBLE(tp)	((tp.tv_sec * 1000000LL)+ tp.tv_usec)
#define DBG_PROFILE_START_TIME()		gettimeofday(&__dbg_prof_tp, NULL); __dbg_prof_start = DBG_PROFILE_TIMEVAL_TO_DOUBLE(__dbg_prof_tp);
#define DBG_PROFILE_END_TIME(duration)	gettimeofday(&__dbg_prof_tp, NULL); (duration) = (DBG_PROFILE_TIMEVAL_TO_DOUBLE(__dbg_prof_tp) - __dbg_prof_start);
#else
#define DBG_PROFILE_TIMEVAL_TO_DOUBLE(tp)
#define DBG_PROFILE_START_TIME()
#define DBG_PROFILE_END_TIME(duration)
#endif

#define DBG_INF_EX(dbg_obj, msg)		do { if (dbg_skip_trace == FALSE) (dbg_obj)->m->log((dbg_obj), __LINE__, __FILE__, -1, "info : ", (msg)); } while (0)
#define DBG_ERR_EX(dbg_obj, msg)		do { if (dbg_skip_trace == FALSE) (dbg_obj)->m->log((dbg_obj), __LINE__, __FILE__, -1, "error: ", (msg)); } while (0)
#define DBG_INF_FMT_EX(dbg_obj, ...)	do { if (dbg_skip_trace == FALSE) (dbg_obj)->m->log_va((dbg_obj), __LINE__, __FILE__, -1, "info : ", __VA_ARGS__); } while (0)
#define DBG_ERR_FMT_EX(dbg_obj, ...)	do { if (dbg_skip_trace == FALSE) (dbg_obj)->m->log_va((dbg_obj), __LINE__, __FILE__, -1, "error: ", __VA_ARGS__); } while (0)

#define DBG_BLOCK_ENTER_EX(dbg_obj, block_name) \
		{ \
			DBG_ENTER_EX(dbg_obj, (block_name));

#define DBG_BLOCK_LEAVE_EX(dbg_obj) \
			DBG_LEAVE_EX((dbg_obj), ;) \
		} \
	

#define DBG_ENTER_EX(dbg_obj, func_name) \
					struct timeval __dbg_prof_tp = {0}; \
					uint64_t __dbg_prof_start = 0; /* initialization is needed */ \
					zend_bool dbg_skip_trace = TRUE; \
					if ((dbg_obj)) { \
						dbg_skip_trace = !(dbg_obj)->m->func_enter((dbg_obj), __LINE__, __FILE__, func_name, strlen(func_name)); \
					} \
					if (dbg_skip_trace); /* shut compiler's mouth */\ 
					do { \
						if ((dbg_obj) && (dbg_obj)->flags & MYSQLND_DEBUG_PROFILE_CALLS) { \
							DBG_PROFILE_START_TIME(); \
						} \
					} while (0); 

#define DBG_LEAVE_EX(dbg_obj, leave)	\
			do {\
				if ((dbg_obj)) { \
					uint64_t this_call_duration = 0; \
					if ((dbg_obj)->flags & MYSQLND_DEBUG_PROFILE_CALLS) { \
						DBG_PROFILE_END_TIME(this_call_duration); \
					} \
					(dbg_obj)->m->func_leave((dbg_obj), __LINE__, __FILE__, this_call_duration); \
				} \
				leave \
			} while (0);

#define DBG_RETURN_EX(dbg_obj, value) DBG_LEAVE_EX(dbg_obj, return (value);)

#define DBG_VOID_RETURN_EX(dbg_obj) DBG_LEAVE_EX(dbg_obj, return;)



#else  /* defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER >= 1400)) */
static inline void DBG_INF_EX(MYSQLND_DEBUG * dbg_obj, const char * const msg) {}
static inline void DBG_ERR_EX(MYSQLND_DEBUG * dbg_obj, const char * const msg) {}
static inline void DBG_INF_FMT_EX(MYSQLND_DEBUG * dbg_obj, ...) {}
static inline void DBG_ERR_FMT_EX(MYSQLND_DEBUG * dbg_obj, ...) {}
static inline void DBG_ENTER_EX(MYSQLND_DEBUG * dbg_obj, const char * const func_name) {}
#define DBG_BLOCK_ENTER(bname)			{
#define DBG_RETURN_EX(dbg_obj, value)	return (value)
#define DBG_VOID_RETURN_EX(dbg_obj)		return
#define DBG_BLOCK_LEAVE_EX(dbg_obj)		}

#endif

#if MYSQLND_DBG_ENABLED == 1

#define DBG_INF(msg)		DBG_INF_EX(MYSQLND_G(dbg), (msg))
#define DBG_ERR(msg)		DBG_ERR_EX(MYSQLND_G(dbg), (msg))
#define DBG_INF_FMT(...)	DBG_INF_FMT_EX(MYSQLND_G(dbg), __VA_ARGS__)
#define DBG_ERR_FMT(...)	DBG_ERR_FMT_EX(MYSQLND_G(dbg), __VA_ARGS__)

#define DBG_ENTER(func_name)	DBG_ENTER_EX(MYSQLND_G(dbg), (func_name))
#define DBG_BLOCK_ENTER(bname)	DBG_BLOCK_ENTER_EX(MYSQLND_G(dbg), (bname))
#define DBG_RETURN(value)		DBG_RETURN_EX(MYSQLND_G(dbg), (value))
#define DBG_VOID_RETURN			DBG_VOID_RETURN_EX(MYSQLND_G(dbg))
#define DBG_BLOCK_LEAVE			DBG_BLOCK_LEAVE_EX(MYSQLND_G(dbg))

#elif MYSQLND_DBG_ENABLED == 0

static inline void DBG_INF(const char * const msg) {}
static inline void DBG_ERR(const char * const msg) {}
static inline void DBG_INF_FMT(const char * const format, ...) {}
static inline void DBG_ERR_FMT(const char * const format, ...) {}
static inline void DBG_ENTER(const char * const func_name) {}
#define DBG_BLOCK_ENTER(bname)	{
#define DBG_RETURN(value)		return (value)
#define DBG_VOID_RETURN			return
#define DBG_BLOCK_LEAVE			}

#endif

#endif /* MYSQLND_DEBUG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */