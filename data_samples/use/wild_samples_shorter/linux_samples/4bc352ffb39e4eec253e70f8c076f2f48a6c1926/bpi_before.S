
	.popsection

ENTRY(__qcom_hyp_sanitize_link_stack_start)
	stp     x29, x30, [sp, #-16]!
	.rept	16
	bl	. + 4
	.endr
	ldp	x29, x30, [sp], #16
ENTRY(__qcom_hyp_sanitize_link_stack_end)

.macro smccc_workaround_1 inst
	sub	sp, sp, #(8 * 4)
	stp	x2, x3, [sp, #(8 * 0)]
	stp	x0, x1, [sp, #(8 * 2)]