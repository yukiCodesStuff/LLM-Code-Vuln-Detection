	/* Save LR into r9 */
	mflr	r9

	// Flush the link stack
	.rept 64
	bl	.+4
	.endr
	b	1f
	.balign 32
	/* Restore LR */
1:	mtlr	r9

	// If we're just flushing the link stack, return here
3:	nop
	patch_site 3b patch__flush_link_stack_return

	li	r9,0x7fff
	mtctr	r9

	BCCTR_FLUSH