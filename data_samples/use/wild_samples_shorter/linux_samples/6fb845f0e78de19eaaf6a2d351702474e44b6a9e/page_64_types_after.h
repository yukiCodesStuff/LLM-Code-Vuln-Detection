#endif

#ifdef CONFIG_KASAN
#ifdef CONFIG_KASAN_EXTRA
#define KASAN_STACK_ORDER 2
#else
#define KASAN_STACK_ORDER 1
#endif
#else
#define KASAN_STACK_ORDER 0
#endif
