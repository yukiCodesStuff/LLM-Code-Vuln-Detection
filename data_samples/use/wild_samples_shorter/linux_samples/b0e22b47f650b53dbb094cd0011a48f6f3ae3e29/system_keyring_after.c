#include <keys/asymmetric-type.h>
#include <keys/system_keyring.h>
#include <crypto/pkcs7.h>
#include "common.h"

static struct key *builtin_trusted_keys;
#ifdef CONFIG_SECONDARY_TRUSTED_KEYRING
static struct key *secondary_trusted_keys;
 */
static __init int load_system_certificate_list(void)
{
	pr_notice("Loading compiled-in X.509 certificates\n");

	return load_certificate_list(system_certificate_list, system_certificate_list_size,
				     builtin_trusted_keys);
}
late_initcall(load_system_certificate_list);

#ifdef CONFIG_SYSTEM_DATA_VERIFICATION
			pr_devel("PKCS#7 platform keyring is not available\n");
			goto error;
		}

		ret = is_key_on_revocation_list(pkcs7);
		if (ret != -ENOKEY) {
			pr_devel("PKCS#7 platform key is on revocation list\n");
			goto error;
		}
	}
	ret = pkcs7_validate_trust(pkcs7, trusted_keys);
	if (ret < 0) {
		if (ret == -ENOKEY)