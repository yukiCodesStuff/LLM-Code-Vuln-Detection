#define FOLL_MLOCK	0x1000	/* lock present pages */
#define FOLL_REMOTE	0x2000	/* we are working on non-current tsk/mm */
#define FOLL_COW	0x4000	/* internal GUP flag */

static inline int vm_fault_to_errno(int vm_fault, int foll_flags)
{
	if (vm_fault & VM_FAULT_OOM)