int CRYPTO_mem_debug_push(const char *info, const char *file, int line)
{
    (void)info; (void)file; (void)line;
    return 0;
}

int CRYPTO_mem_debug_pop(void)
{
    return 0;
}

void CRYPTO_mem_debug_malloc(void *addr, size_t num, int flag,
                             const char *file, int line)