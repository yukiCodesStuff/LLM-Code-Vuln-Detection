	.endr
	blr

/* Clobbers r10, r11, ctr */
.macro L1D_DISPLACEMENT_FLUSH
	ld	r10,PACA_RFI_FLUSH_FALLBACK_AREA(r13)
	ld	r11,PACA_L1D_FLUSH_SIZE(r13)
	srdi	r11,r11,(7 + 3) /* 128 byte lines, unrolled 8x */
	mtctr	r11
	ld	r11,(0x80 + 8)*7(r10)
	addi	r10,r10,0x80*8
	bdnz	1b
.endm

TRAMP_REAL_BEGIN(entry_flush_fallback)
	std	r9,PACA_EXRFI+EX_R9(r13)
	std	r10,PACA_EXRFI+EX_R10(r13)
	std	r11,PACA_EXRFI+EX_R11(r13)
	mfctr	r9
	L1D_DISPLACEMENT_FLUSH
	mtctr	r9
	ld	r9,PACA_EXRFI+EX_R9(r13)
	ld	r10,PACA_EXRFI+EX_R10(r13)
	ld	r11,PACA_EXRFI+EX_R11(r13)
	std	r10,PACA_EXRFI+EX_R10(r13)
	std	r11,PACA_EXRFI+EX_R11(r13)
	mfctr	r9
	L1D_DISPLACEMENT_FLUSH
	mtctr	r9
	ld	r9,PACA_EXRFI+EX_R9(r13)
	ld	r10,PACA_EXRFI+EX_R10(r13)
	ld	r11,PACA_EXRFI+EX_R11(r13)
	std	r10,PACA_EXRFI+EX_R10(r13)
	std	r11,PACA_EXRFI+EX_R11(r13)
	mfctr	r9
	L1D_DISPLACEMENT_FLUSH
	mtctr	r9
	ld	r9,PACA_EXRFI+EX_R9(r13)
	ld	r10,PACA_EXRFI+EX_R10(r13)
	ld	r11,PACA_EXRFI+EX_R11(r13)
	RFSCV

USE_TEXT_SECTION()

_GLOBAL(do_uaccess_flush)
	UACCESS_FLUSH_FIXUP_SECTION
	nop
	nop
	nop
	blr
	L1D_DISPLACEMENT_FLUSH
	blr
_ASM_NOKPROBE_SYMBOL(do_uaccess_flush)
EXPORT_SYMBOL(do_uaccess_flush)


MASKED_INTERRUPT
MASKED_INTERRUPT hsrr=1

#ifdef CONFIG_KVM_BOOK3S_64_HANDLER
kvmppc_skip_interrupt:
	/*