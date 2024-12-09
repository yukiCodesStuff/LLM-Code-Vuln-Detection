    return TRUE;
}

#elif defined(__GNUC__) && !defined(_AIX)
# undef DEP_INIT_ATTRIBUTE
# undef DEP_FINI_ATTRIBUTE
# define DEP_INIT_ATTRIBUTE static __attribute__((constructor))
# define DEP_FINI_ATTRIBUTE static __attribute__((destructor))
# pragma init(init)
# pragma fini(cleanup)

#elif defined(_AIX) && !defined(__GNUC__)
void _init(void);
void _cleanup(void);
# pragma init(_init)
# pragma fini(_cleanup)