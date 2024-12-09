/*
        Added support for the AMD Geode LX RNG
	(c) Copyright 2004-2005 Advanced Micro Devices, Inc.

	derived from

 	Hardware driver for the Intel/AMD/VIA Random Number Generators (RNG)
	(c) Copyright 2003 Red Hat Inc <jgarzik@redhat.com>

 	derived from

        Hardware driver for the AMD 768 Random Number Generator (RNG)
        (c) Copyright 2001 Red Hat Inc <alan@redhat.com>

 	derived from

	Hardware driver for Intel i810 Random Number Generator (RNG)
	Copyright 2000,2001 Jeff Garzik <jgarzik@pobox.com>
	Copyright 2000,2001 Philipp Rumpf <prumpf@mandrakesoft.com>

	Added generic RNG API
	Copyright 2006 Michael Buesch <m@bues.ch>
	Copyright 2005 (c) MontaVista Software, Inc.

	Please read Documentation/hw_random.txt for details on use.

	----------------------------------------------------------
	This software may be used and distributed according to the terms
        of the GNU General Public License, incorporated herein by reference.

 */


#include <linux/device.h>
#include <linux/hw_random.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/uaccess.h>


#define RNG_MODULE_NAME		"hw_random"
#define PFX			RNG_MODULE_NAME ": "
#define RNG_MISCDEV_MINOR	183 /* official */


static struct hwrng *current_rng;
static LIST_HEAD(rng_list);
static DEFINE_MUTEX(rng_mutex);
static int data_avail;
static u8 rng_buffer[SMP_CACHE_BYTES < 32 ? 32 : SMP_CACHE_BYTES]
	__cacheline_aligned;

static inline int hwrng_init(struct hwrng *rng)
{
	if (!rng->init)
		return 0;
	return rng->init(rng);
}
/*
        Added support for the AMD Geode LX RNG
	(c) Copyright 2004-2005 Advanced Micro Devices, Inc.

	derived from

 	Hardware driver for the Intel/AMD/VIA Random Number Generators (RNG)
	(c) Copyright 2003 Red Hat Inc <jgarzik@redhat.com>

 	derived from

        Hardware driver for the AMD 768 Random Number Generator (RNG)
        (c) Copyright 2001 Red Hat Inc <alan@redhat.com>

 	derived from

	Hardware driver for Intel i810 Random Number Generator (RNG)
	Copyright 2000,2001 Jeff Garzik <jgarzik@pobox.com>
	Copyright 2000,2001 Philipp Rumpf <prumpf@mandrakesoft.com>

	Added generic RNG API
	Copyright 2006 Michael Buesch <m@bues.ch>
	Copyright 2005 (c) MontaVista Software, Inc.

	Please read Documentation/hw_random.txt for details on use.

	----------------------------------------------------------
	This software may be used and distributed according to the terms
        of the GNU General Public License, incorporated herein by reference.

 */


#include <linux/device.h>
#include <linux/hw_random.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/uaccess.h>


#define RNG_MODULE_NAME		"hw_random"
#define PFX			RNG_MODULE_NAME ": "
#define RNG_MISCDEV_MINOR	183 /* official */


static struct hwrng *current_rng;
static LIST_HEAD(rng_list);
static DEFINE_MUTEX(rng_mutex);
static int data_avail;
static u8 rng_buffer[SMP_CACHE_BYTES < 32 ? 32 : SMP_CACHE_BYTES]
	__cacheline_aligned;

static inline int hwrng_init(struct hwrng *rng)
{
	if (!rng->init)
		return 0;
	return rng->init(rng);
}
		if (!data_avail) {
			bytes_read = rng_get_data(current_rng, rng_buffer,
				sizeof(rng_buffer),
				!(filp->f_flags & O_NONBLOCK));
			if (bytes_read < 0) {
				err = bytes_read;
				goto out_unlock;
			}
			data_avail = bytes_read;
		}
{
	int must_register_misc;
	int err = -EINVAL;
	struct hwrng *old_rng, *tmp;

	if (rng->name == NULL ||
	    (rng->data_read == NULL && rng->read == NULL))
		goto out;

	mutex_lock(&rng_mutex);

	/* Must not register two RNGs with the same name. */
	err = -EEXIST;
	list_for_each_entry(tmp, &rng_list, list) {
		if (strcmp(tmp->name, rng->name) == 0)
			goto out_unlock;
	}