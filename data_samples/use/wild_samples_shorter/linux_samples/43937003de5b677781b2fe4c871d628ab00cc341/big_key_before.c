 * 2 of the Licence, or (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/file.h>
#include <linux/shmem_fs.h>
 */
static int __init big_key_init(void)
{
	return register_key_type(&key_type_big_key);
}

/*
 * Initialize big_key crypto and RNG algorithms
 */
static int __init big_key_crypto_init(void)
{
	int ret = -EINVAL;

	/* init RNG */
	big_key_rng = crypto_alloc_rng(big_key_rng_name, 0, 0);
	if (IS_ERR(big_key_rng)) {
		big_key_rng = NULL;
		return -EFAULT;
	}

	/* seed RNG */
	ret = crypto_rng_reset(big_key_rng, NULL, crypto_rng_seedsize(big_key_rng));
	if (ret)
		goto error;

	/* init block cipher */
	big_key_skcipher = crypto_alloc_skcipher(big_key_alg_name,
						 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(big_key_skcipher)) {
		big_key_skcipher = NULL;
		ret = -EFAULT;
		goto error;
	}

	return 0;

error:
	crypto_free_rng(big_key_rng);
	big_key_rng = NULL;
	return ret;
}

device_initcall(big_key_init);
late_initcall(big_key_crypto_init);