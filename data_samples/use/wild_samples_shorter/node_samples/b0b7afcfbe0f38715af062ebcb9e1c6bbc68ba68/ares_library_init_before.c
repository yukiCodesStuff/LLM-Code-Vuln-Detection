fpGetBestRoute2_t ares_fpGetBestRoute2 = ZERO_NULL;
#endif

/* library-private global vars with source visibility restricted to this file */

static unsigned int ares_initialized;
static int          ares_init_flags;
  if (ares_init_flags & ARES_LIB_INIT_WIN32)
    ares_win32_cleanup();

  ares_init_flags = ARES_LIB_INIT_NONE;
  ares_malloc = malloc;
  ares_realloc = realloc;
  ares_free = free;