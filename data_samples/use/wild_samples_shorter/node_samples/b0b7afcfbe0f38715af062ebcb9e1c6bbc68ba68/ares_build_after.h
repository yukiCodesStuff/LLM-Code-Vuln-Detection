/* Data type definition of ares_ssize_t. */
#ifdef _WIN32
#  ifdef _WIN64
#    define CARES_TYPEOF_ARES_SSIZE_T __int64
#  else
#    define CARES_TYPEOF_ARES_SSIZE_T long
#  endif
#else
#  define CARES_TYPEOF_ARES_SSIZE_T ssize_t
#endif

typedef CARES_TYPEOF_ARES_SSIZE_T ares_ssize_t;

#endif /* __CARES_BUILD_H */