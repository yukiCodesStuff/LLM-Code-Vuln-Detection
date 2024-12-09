	.endr
	blr

TRAMP_REAL_BEGIN(entry_flush_fallback)
	std	r9,PACA_EXRFI+EX_R9(r13)
	std	r10,PACA_EXRFI+EX_R10(r13)
	std	r11,PACA_EXRFI+EX_R11(r13)
	mfctr	r9
	ld	r10,PACA_RFI_FLUSH_FALLBACK_AREA(r13)
	ld	r11,PACA_L1D_FLUSH_SIZE(r13)
	srdi	r11,r11,(7 + 3) /* 128 byte lines, unrolled 8x */
	mtctr	r11
	DCBT_BOOK3S_STOP_ALL_STREAM_IDS(r11) /* Stop prefetch streams */

	/* order ld/st prior to dcbt stop all streams with flushing */
	sync

	/*
	 * The load addresses are at staggered offsets within cachelines,
	 * which suits some pipelines better (on others it should not
	 * hurt).
	 */
1:
	ld	r11,(0x80 + 8)*0(r10)
	ld	r11,(0x80 + 8)*1(r10)
	ld	r11,(0x80 + 8)*2(r10)
	ld	r11,(0x80 + 8)*3(r10)
	ld	r11,(0x80 + 8)*4(r10)
	ld	r11,(0x80 + 8)*5(r10)
	ld	r11,(0x80 + 8)*6(r10)
	ld	r11,(0x80 + 8)*7(r10)
	addi	r10,r10,0x80*8
	bdnz	1b

	mtctr	r9
	ld	r9,PACA_EXRFI+EX_R9(r13)
	ld	r10,PACA_EXRFI+EX_R10(r13)
	ld	r11,PACA_EXRFI+EX_R11(r13)
	blr

TRAMP_REAL_BEGIN(rfi_flush_fallback)
	SET_SCRATCH0(r13);
	GET_PACA(r13);
	std	r1,PACA_EXRFI+EX_R12(r13)