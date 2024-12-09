#ifndef PHP_STDINT_H
#define PHP_STDINT_H

#if PHP_WIN32
# include "win32/php_stdint.h"
# define HAVE_INT8_T   1
# define HAVE_UINT8_T  1
# define HAVE_INT16_T  1
# define HAVE_UINT16_T 1