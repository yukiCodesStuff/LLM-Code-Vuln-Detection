#include <linux/integrity.h>
#include <linux/evm.h>
#include <crypto/hash.h>
#include "evm.h"

int evm_initialized;

				   xattr_value_len, calc.digest);
		if (rc)
			break;
		rc = memcmp(xattr_data->digest, calc.digest,
			    sizeof(calc.digest));
		if (rc)
			rc = -EINVAL;
		break;