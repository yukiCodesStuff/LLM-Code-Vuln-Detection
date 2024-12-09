#ifndef PHP_STDINT_H
#define PHP_STDINT_H

#if defined(_MSC_VER)
/* Make sure the regular stdint.h wasn't included already and prevent it to be
   included afterwards. Though if some other library needs some stuff from
   stdint.h included afterwards and misses it, we'd have to extend ours. On
   the other hand, if stdint.h was included before, some conflicts might
   happen so we'd likewise have to fix ours. */
# if !defined(_STDINT)
#  define _STDINT
#  include "win32/php_stdint.h"
# endif
# define HAVE_INT8_T   1
# define HAVE_UINT8_T  1
# define HAVE_INT16_T  1
# define HAVE_UINT16_T 1