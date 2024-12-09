#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


#define RNG_MODULE_NAME		"hw_random"
static LIST_HEAD(rng_list);
static DEFINE_MUTEX(rng_mutex);
static int data_avail;
static u8 *rng_buffer;

static size_t rng_buffer_size(void)
{
	return SMP_CACHE_BYTES < 32 ? 32 : SMP_CACHE_BYTES;
}

static inline int hwrng_init(struct hwrng *rng)
{
	if (!rng->init)

		if (!data_avail) {
			bytes_read = rng_get_data(current_rng, rng_buffer,
				rng_buffer_size(),
				!(filp->f_flags & O_NONBLOCK));
			if (bytes_read < 0) {
				err = bytes_read;
				goto out_unlock;

	mutex_lock(&rng_mutex);

	/* kmalloc makes this safe for virt_to_page() in virtio_rng.c */
	err = -ENOMEM;
	if (!rng_buffer) {
		rng_buffer = kmalloc(rng_buffer_size(), GFP_KERNEL);
		if (!rng_buffer)
			goto out_unlock;
	}

	/* Must not register two RNGs with the same name. */
	err = -EEXIST;
	list_for_each_entry(tmp, &rng_list, list) {
		if (strcmp(tmp->name, rng->name) == 0)