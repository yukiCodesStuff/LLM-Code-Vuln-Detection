 */

#include <asm/ppc_asm.h>
#include <asm/code-patching-asm.h>
#include <asm/kvm_asm.h>
#include <asm/reg.h>
#include <asm/mmu.h>
#include <asm/page.h>
1:
#endif /* CONFIG_KVM_XICS */

	/*
	 * Possibly flush the link stack here, before we do a blr in
	 * guest_exit_short_path.
	 */
1:	nop
	patch_site 1b patch__call_kvm_flush_link_stack

	/* If we came in through the P9 short path, go back out to C now */
	lwz	r0, STACK_SLOT_SHORT_PATH(r1)
	cmpwi	r0, 0
	bne	guest_exit_short_path
	mtlr	r0
	blr

.balign 32
.global kvm_flush_link_stack
kvm_flush_link_stack:
	/* Save LR into r0 */
	mflr	r0

	/* Flush the link stack. On Power8 it's up to 32 entries in size. */
	.rept 32
	bl	.+4
	.endr

	/* And on Power9 it's up to 64. */
BEGIN_FTR_SECTION
	.rept 32
	bl	.+4
	.endr
END_FTR_SECTION_IFSET(CPU_FTR_ARCH_300)

	/* Restore LR */
	mtlr	r0
	blr

kvmppc_guest_external:
	/* External interrupt, first check for host_ipi. If this is
	 * set, we know the host wants us out so let's do it now
	 */