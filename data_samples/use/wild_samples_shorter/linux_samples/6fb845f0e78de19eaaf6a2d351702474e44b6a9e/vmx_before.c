#include <linux/mod_devicetable.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/tboot.h>
#include <linux/trace_events.h>

			 * Warn upon starting the first VM in a potentially
			 * insecure environment.
			 */
			if (cpu_smt_control == CPU_SMT_ENABLED)
				pr_warn_once(L1TF_MSG_SMT);
			if (l1tf_vmx_mitigation == VMENTER_L1D_FLUSH_NEVER)
				pr_warn_once(L1TF_MSG_L1D);
			break;