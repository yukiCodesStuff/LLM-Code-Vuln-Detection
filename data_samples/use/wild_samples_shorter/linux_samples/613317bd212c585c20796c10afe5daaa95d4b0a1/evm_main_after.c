#include <linux/integrity.h>
#include <linux/evm.h>
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include "evm.h"

int evm_initialized;

				   xattr_value_len, calc.digest);
		if (rc)
			break;
		rc = crypto_memneq(xattr_data->digest, calc.digest,
			    sizeof(calc.digest));
		if (rc)
			rc = -EINVAL;
		break;