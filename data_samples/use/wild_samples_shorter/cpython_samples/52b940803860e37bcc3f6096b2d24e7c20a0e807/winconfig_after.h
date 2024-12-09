#include <memory.h>
#include <string.h>

#if defined(HAVE_EXPAT_CONFIG_H) /* e.g. MinGW */
#  include <expat_config.h>
#else /* !defined(HAVE_EXPAT_CONFIG_H) */

#  define XML_NS 1
#  define XML_DTD 1
#  define XML_CONTEXT_BYTES 1024

/* we will assume all Windows platforms are little endian */
#  define BYTEORDER 1234

#endif /* !defined(HAVE_EXPAT_CONFIG_H) */

#endif /* ndef WINCONFIG_H */