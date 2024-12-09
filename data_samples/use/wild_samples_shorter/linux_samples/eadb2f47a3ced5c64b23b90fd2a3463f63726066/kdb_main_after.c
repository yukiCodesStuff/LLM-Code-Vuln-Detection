#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/security.h>
#include "kdb_private.h"

#undef	MODULE_PARAM_PREFIX
#define	MODULE_PARAM_PREFIX "kdb."
}

/*
 * Update the permissions flags (kdb_cmd_enabled) to match the
 * current lockdown state.
 *
 * Within this function the calls to security_locked_down() are "lazy". We
 * avoid calling them if the current value of kdb_cmd_enabled already excludes
 * flags that might be subject to lockdown. Additionally we deliberately check
 * the lockdown flags independently (even though read lockdown implies write
 * lockdown) since that results in both simpler code and clearer messages to
 * the user on first-time debugger entry.
 *
 * The permission masks during a read+write lockdown permits the following
 * flags: INSPECT, SIGNAL, REBOOT (and ALWAYS_SAFE).
 *
 * The INSPECT commands are not blocked during lockdown because they are
 * not arbitrary memory reads. INSPECT covers the backtrace family (sometimes
 * forcing them to have no arguments) and lsmod. These commands do expose
 * some kernel state but do not allow the developer seated at the console to
 * choose what state is reported. SIGNAL and REBOOT should not be controversial,
 * given these are allowed for root during lockdown already.
 */
static void kdb_check_for_lockdown(void)
{
	const int write_flags = KDB_ENABLE_MEM_WRITE |
				KDB_ENABLE_REG_WRITE |
				KDB_ENABLE_FLOW_CTRL;
	const int read_flags = KDB_ENABLE_MEM_READ |
			       KDB_ENABLE_REG_READ;

	bool need_to_lockdown_write = false;
	bool need_to_lockdown_read = false;

	if (kdb_cmd_enabled & (KDB_ENABLE_ALL | write_flags))
		need_to_lockdown_write =
			security_locked_down(LOCKDOWN_DBG_WRITE_KERNEL);

	if (kdb_cmd_enabled & (KDB_ENABLE_ALL | read_flags))
		need_to_lockdown_read =
			security_locked_down(LOCKDOWN_DBG_READ_KERNEL);

	/* De-compose KDB_ENABLE_ALL if required */
	if (need_to_lockdown_write || need_to_lockdown_read)
		if (kdb_cmd_enabled & KDB_ENABLE_ALL)
			kdb_cmd_enabled = KDB_ENABLE_MASK & ~KDB_ENABLE_ALL;

	if (need_to_lockdown_write)
		kdb_cmd_enabled &= ~write_flags;

	if (need_to_lockdown_read)
		kdb_cmd_enabled &= ~read_flags;
}

/*
 * Check whether the flags of the current command, the permissions of the kdb
 * console and the lockdown state allow a command to be run.
 */
static bool kdb_check_flags(kdb_cmdflags_t flags, int permissions,
				   bool no_args)
{
	/* permissions comes from userspace so needs massaging slightly */
	permissions &= KDB_ENABLE_MASK;
		kdb_curr_task(raw_smp_processor_id());

	KDB_DEBUG_STATE("kdb_local 1", reason);

	kdb_check_for_lockdown();

	kdb_go_count = 0;
	if (reason == KDB_REASON_DEBUG) {
		/* special case below */
	} else {