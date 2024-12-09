/* Data type definition of ares_ssize_t. */
#ifdef _WIN32
#  ifdef _WIN64
     typedef __int64 ares_ssize_t;
#  else
     typedef long ares_ssize_t;
#  endif
#else
#  ifdef CARES_TYPEOF_ARES_SSIZE_T
     typedef CARES_TYPEOF_ARES_SSIZE_T ares_ssize_t;
#  else
     typedef ssize_t ares_ssize_t;
#  endif
#endif

#endif /* __CARES_BUILD_H */