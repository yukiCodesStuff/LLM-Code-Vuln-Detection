	/* Save LR into r9 */
	mflr	r9

	.rept 64
	bl	.+4
	.endr
	b	1f
	.balign 32
	/* Restore LR */
1:	mtlr	r9
	li	r9,0x7fff
	mtctr	r9

	BCCTR_FLUSH