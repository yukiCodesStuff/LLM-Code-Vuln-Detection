#define gdPFree(ptr)		pefree(ptr, 1)
#define gdPEstrdup(ptr)		pestrdup(ptr, 1)

/* Returns nonzero if multiplying the two quantities will
	result in integer overflow. Also returns nonzero if 
	either quantity is negative. By Phil Knirsch based on
	netpbm fixes by Alan Cox. */

int overflow2(int a, int b);

#ifdef ZTS
#define gdMutexDeclare(x) MUTEX_T x
#define gdMutexSetup(x) x = tsrm_mutex_alloc()
#define gdMutexShutdown(x) tsrm_mutex_free(x)