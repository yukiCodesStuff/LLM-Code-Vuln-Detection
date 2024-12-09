	sub	lr, lr, #\correction
	.endif

	@ Save r0, lr_<exception> (parent PC)
	stmia	sp, {r0, lr}		@ save r0, lr

	@ Save spsr_<exception> (parent CPSR)
2:	mrs	lr, spsr
	str	lr, [sp, #8]		@ save spsr

	@
	@ Prepare for SVC32 mode.  IRQs remain disabled.
	movs	pc, lr			@ branch to handler in SVC mode
ENDPROC(vector_\name)

#ifdef CONFIG_HARDEN_BRANCH_HISTORY
	.subsection 1
	.align 5
vector_bhb_loop8_\name:
	.if \correction
	sub	lr, lr, #\correction
	.endif

	@ Save r0, lr_<exception> (parent PC)
	stmia	sp, {r0, lr}

	@ bhb workaround
	mov	r0, #8
1:	b	. + 4
	subs	r0, r0, #1
	bne	1b
	dsb
	isb
	b	2b
ENDPROC(vector_bhb_loop8_\name)

vector_bhb_bpiall_\name:
	.if \correction
	sub	lr, lr, #\correction
	.endif

	@ Save r0, lr_<exception> (parent PC)
	stmia	sp, {r0, lr}

	@ bhb workaround
	mcr	p15, 0, r0, c7, c5, 6	@ BPIALL
	@ isb not needed due to "movs pc, lr" in the vector stub
	@ which gives a "context synchronisation".
	b	2b
ENDPROC(vector_bhb_bpiall_\name)
	.previous
#endif

	.align	2
	@ handler addresses follow this label
1:
	.endm
	.section .stubs, "ax", %progbits
	@ This must be the first word
	.word	vector_swi
#ifdef CONFIG_HARDEN_BRANCH_HISTORY
	.word	vector_bhb_loop8_swi
	.word	vector_bhb_bpiall_swi
#endif

vector_rst:
 ARM(	swi	SYS_ERROR0	)
 THUMB(	svc	#0		)
 * FIQ "NMI" handler
 *-----------------------------------------------------------------------------
 * Handle a FIQ using the SVC stack allowing FIQ act like NMI on x86
 * systems. This must be the last vector stub, so lets place it in its own
 * subsection.
 */
	.subsection 2
	vector_stub	fiq, FIQ_MODE, 4

	.long	__fiq_usr			@  0  (USR_26 / USR_32)
	.long	__fiq_svc			@  1  (FIQ_26 / FIQ_32)
	W(b)	vector_irq
	W(b)	vector_fiq

#ifdef CONFIG_HARDEN_BRANCH_HISTORY
	.section .vectors.bhb.loop8, "ax", %progbits
.L__vectors_bhb_loop8_start:
	W(b)	vector_rst
	W(b)	vector_bhb_loop8_und
	W(ldr)	pc, .L__vectors_bhb_loop8_start + 0x1004
	W(b)	vector_bhb_loop8_pabt
	W(b)	vector_bhb_loop8_dabt
	W(b)	vector_addrexcptn
	W(b)	vector_bhb_loop8_irq
	W(b)	vector_bhb_loop8_fiq

	.section .vectors.bhb.bpiall, "ax", %progbits
.L__vectors_bhb_bpiall_start:
	W(b)	vector_rst
	W(b)	vector_bhb_bpiall_und
	W(ldr)	pc, .L__vectors_bhb_bpiall_start + 0x1008
	W(b)	vector_bhb_bpiall_pabt
	W(b)	vector_bhb_bpiall_dabt
	W(b)	vector_addrexcptn
	W(b)	vector_bhb_bpiall_irq
	W(b)	vector_bhb_bpiall_fiq
#endif

	.data
	.align	2

	.globl	cr_alignment