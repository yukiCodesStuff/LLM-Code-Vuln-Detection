}
static inline void tramp_free(void *tramp)
{
	module_free(NULL, tramp);
}
#else
/* Trampolines can only be created if modules are supported */
static inline void *alloc_tramp(unsigned long size)