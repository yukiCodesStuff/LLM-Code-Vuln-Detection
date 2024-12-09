#define gdPFree(ptr)		pefree(ptr, 1)
#define gdPEstrdup(ptr)		pestrdup(ptr, 1)

#ifdef ZTS
#define gdMutexDeclare(x) MUTEX_T x
#define gdMutexSetup(x) x = tsrm_mutex_alloc()
#define gdMutexShutdown(x) tsrm_mutex_free(x)