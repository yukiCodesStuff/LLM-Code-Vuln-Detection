#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/uaccess.h>


#define RNG_MODULE_NAME		"hw_random"
static LIST_HEAD(rng_list);
static DEFINE_MUTEX(rng_mutex);
static int data_avail;
static u8 rng_buffer[SMP_CACHE_BYTES < 32 ? 32 : SMP_CACHE_BYTES]
	__cacheline_aligned;

static inline int hwrng_init(struct hwrng *rng)
{
	if (!rng->init)

		if (!data_avail) {
			bytes_read = rng_get_data(current_rng, rng_buffer,
				sizeof(rng_buffer),
				!(filp->f_flags & O_NONBLOCK));
			if (bytes_read < 0) {
				err = bytes_read;
				goto out_unlock;

	mutex_lock(&rng_mutex);

	/* Must not register two RNGs with the same name. */
	err = -EEXIST;
	list_for_each_entry(tmp, &rng_list, list) {
		if (strcmp(tmp->name, rng->name) == 0)