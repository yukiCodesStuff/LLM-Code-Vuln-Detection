 *-----------------------------------------------------------------------------
 */

	.align	5
#ifdef CONFIG_HARDEN_BRANCH_HISTORY
ENTRY(vector_bhb_loop8_swi)
	sub	sp, sp, #PT_REGS_SIZE
	stmia	sp, {r0 - r12}
	mov	r8, #8
1:	b	2f
2:	subs	r8, r8, #1
	bne	1b
	dsb
	isb
	b	3f
ENDPROC(vector_bhb_loop8_swi)

	.align	5
ENTRY(vector_bhb_bpiall_swi)
	sub	sp, sp, #PT_REGS_SIZE
	stmia	sp, {r0 - r12}
	mcr	p15, 0, r8, c7, c5, 6	@ BPIALL
	isb
	b	3f
ENDPROC(vector_bhb_bpiall_swi)
#endif
	.align	5
ENTRY(vector_swi)
#ifdef CONFIG_CPU_V7M
	v7m_exception_entry
#else
	sub	sp, sp, #PT_REGS_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0 - r12
3:
 ARM(	add	r8, sp, #S_PC		)
 ARM(	stmdb	r8, {sp, lr}^		)	@ Calling sp, lr
 THUMB(	mov	r8, sp			)
 THUMB(	store_user_sp_lr r8, r10, S_SP	)	@ calling sp, lr