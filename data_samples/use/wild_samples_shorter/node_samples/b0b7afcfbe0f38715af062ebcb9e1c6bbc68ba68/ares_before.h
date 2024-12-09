#  include <netinet/in.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

                                       void (*afree)(void *ptr),
                                       void *(*arealloc)(void *ptr, size_t size));

CARES_EXTERN int ares_library_initialized(void);

CARES_EXTERN void ares_library_cleanup(void);
