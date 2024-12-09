
#ifdef CONFIG_PM
/*
 * The following code is located into the .data section.  This is to
 * allow phys_l2x0_saved_regs to be accessed with a relative load
 * as we are running on physical address here.
 */
	.data
	.align

#ifdef CONFIG_CACHE_L2X0
	.macro	pl310_resume
	ldr	r2, phys_l2x0_saved_regs
	ldr	r0, [r2, #L2X0_R_PHY_BASE]	@ get physical base of l2x0
	ldr	r1, [r2, #L2X0_R_AUX_CTRL]	@ get aux_ctrl value
	str	r1, [r0, #L2X0_AUX_CTRL]	@ restore aux_ctrl
	mov	r1, #0x1
	str	r1, [r0, #L2X0_CTRL]		@ re-enable L2
	.endm

	.globl	phys_l2x0_saved_regs
phys_l2x0_saved_regs:
        .long   0
#else
	.macro	pl310_resume
	.endm
#endif