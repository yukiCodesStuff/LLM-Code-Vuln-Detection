#include <linux/vmacache.h>
#include <linux/rcupdate.h>
#include <linux/irq.h>
#include <linux/security.h>

#include <asm/cacheflush.h>
#include <asm/byteorder.h>
#include <linux/atomic.h>
				continue;
			kgdb_connected = 0;
		} else {
			/*
			 * This is a brutal way to interfere with the debugger
			 * and prevent gdb being used to poke at kernel memory.
			 * This could cause trouble if lockdown is applied when
			 * there is already an active gdb session. For now the
			 * answer is simply "don't do that". Typically lockdown
			 * *will* be applied before the debug core gets started
			 * so only developers using kgdb for fairly advanced
			 * early kernel debug can be biten by this. Hopefully
			 * they are sophisticated enough to take care of
			 * themselves, especially with help from the lockdown
			 * message printed on the console!
			 */
			if (security_locked_down(LOCKDOWN_DBG_WRITE_KERNEL)) {
				if (IS_ENABLED(CONFIG_KGDB_KDB)) {
					/* Switch back to kdb if possible... */
					dbg_kdb_mode = 1;
					continue;
				} else {
					/* ... otherwise just bail */
					break;
				}
			}
			error = gdb_serial_stub(ks);
		}

		if (error == DBG_PASS_EVENT) {