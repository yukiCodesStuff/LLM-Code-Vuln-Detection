 */

#include <asm/ppc_asm.h>
#include <asm/kvm_asm.h>
#include <asm/reg.h>
#include <asm/mmu.h>
#include <asm/page.h>
1:
#endif /* CONFIG_KVM_XICS */

	/* If we came in through the P9 short path, go back out to C now */
	lwz	r0, STACK_SLOT_SHORT_PATH(r1)
	cmpwi	r0, 0
	bne	guest_exit_short_path
	mtlr	r0
	blr

kvmppc_guest_external:
	/* External interrupt, first check for host_ipi. If this is
	 * set, we know the host wants us out so let's do it now
	 */