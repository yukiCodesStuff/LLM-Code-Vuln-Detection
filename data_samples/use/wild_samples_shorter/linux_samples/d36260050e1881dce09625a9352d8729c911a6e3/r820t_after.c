//
//	RF Gain set/get is not implemented.

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/videodev2.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/bitrev.h>
err_no_gate:
	mutex_unlock(&r820t_list_mutex);

	pr_info("%s: failed=%d\n", __func__, rc);
	r820t_release(fe);
	return NULL;
}
EXPORT_SYMBOL_GPL(r820t_attach);