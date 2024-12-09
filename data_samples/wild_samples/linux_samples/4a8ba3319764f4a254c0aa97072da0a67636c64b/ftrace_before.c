{
	return module_alloc(size);
}
static inline void tramp_free(void *tramp)
{
	module_free(NULL, tramp);
}