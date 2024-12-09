
#ifdef CONFIG_PM
/*
 * The following code must assume it is running from physical address
 * where absolute virtual addresses to the data section have to be
 * turned into relative ones.
 */

#ifdef CONFIG_CACHE_L2X0
	.macro	pl310_resume
	adr	r0, l2x0_saved_regs_offset
	ldr	r2, [r0]
	add	r2, r2, r0
	ldr	r0, [r2, #L2X0_R_PHY_BASE]	@ get physical base of l2x0
	ldr	r1, [r2, #L2X0_R_AUX_CTRL]	@ get aux_ctrl value
	str	r1, [r0, #L2X0_AUX_CTRL]	@ restore aux_ctrl
	mov	r1, #0x1
	str	r1, [r0, #L2X0_CTRL]		@ re-enable L2
	.endm

l2x0_saved_regs_offset:
	.word	l2x0_saved_regs - .

#else
	.macro	pl310_resume
	.endm
#endif