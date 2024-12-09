#  include <netinet/in.h>
#endif

#if defined(ANDROID) || defined(__ANDROID__)
#include <jni.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

                                       void (*afree)(void *ptr),
                                       void *(*arealloc)(void *ptr, size_t size));

#if defined(ANDROID) || defined(__ANDROID__)
CARES_EXTERN void ares_library_init_jvm(JavaVM *jvm);
CARES_EXTERN int ares_library_init_android(jobject connectivity_manager);
CARES_EXTERN int ares_library_android_initialized(void);
#endif

CARES_EXTERN int ares_library_initialized(void);

CARES_EXTERN void ares_library_cleanup(void);
