#include "ext/standard/info.h"

#include "tidy.h"

#if HAVE_TIDYBUFFIO_H
#include "tidybuffio.h"
#else
#include "buffio.h"
#endif

/* compatibility with older versions of libtidy */
#ifndef TIDY_CALL
#define TIDY_CALL