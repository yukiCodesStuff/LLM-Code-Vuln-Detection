#include "cryptlib.h"
#include <openssl/buffer.h>

/* LIMIT_BEFORE_EXPANSION is the maximum n such that (n+3)/3*4 < 2**31. That
 * function is applied in several functions in this file and this limit ensures
 * that the result fits in an int. */
#define LIMIT_BEFORE_EXPANSION 0x5ffffffc

BUF_MEM *BUF_MEM_new(void)
	{
	BUF_MEM *ret;

		str->length=len;
		return(len);
		}
	/* This limit is sufficient to ensure (len+3)/3*4 < 2**31 */
	if (len > LIMIT_BEFORE_EXPANSION)
		{
		BUFerr(BUF_F_BUF_MEM_GROW,ERR_R_MALLOC_FAILURE);
		return 0;
		}
	n=(len+3)/3*4;
	if (str->data == NULL)
		ret=OPENSSL_malloc(n);
	else
		str->length=len;
		return(len);
		}
	/* This limit is sufficient to ensure (len+3)/3*4 < 2**31 */
	if (len > LIMIT_BEFORE_EXPANSION)
		{
		BUFerr(BUF_F_BUF_MEM_GROW_CLEAN,ERR_R_MALLOC_FAILURE);
		return 0;
		}
	n=(len+3)/3*4;
	if (str->data == NULL)
		ret=OPENSSL_malloc(n);
	else