#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "kdb_private.h"

#undef	MODULE_PARAM_PREFIX
#define	MODULE_PARAM_PREFIX "kdb."
}

/*
 * Check whether the flags of the current command and the permissions
 * of the kdb console has allow a command to be run.
 */
static inline bool kdb_check_flags(kdb_cmdflags_t flags, int permissions,
				   bool no_args)
{
	/* permissions comes from userspace so needs massaging slightly */
	permissions &= KDB_ENABLE_MASK;
		kdb_curr_task(raw_smp_processor_id());

	KDB_DEBUG_STATE("kdb_local 1", reason);
	kdb_go_count = 0;
	if (reason == KDB_REASON_DEBUG) {
		/* special case below */
	} else {