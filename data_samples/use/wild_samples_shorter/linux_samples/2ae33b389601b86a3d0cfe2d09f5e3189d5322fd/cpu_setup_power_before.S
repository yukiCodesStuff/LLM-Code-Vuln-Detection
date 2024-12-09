
_GLOBAL(__setup_cpu_power8)
	mflr	r11
	bl	__init_hvmode_206
	mtlr	r11
	beqlr
	li	r0,0
	mfspr	r3,SPRN_LPCR
	oris	r3, r3, LPCR_AIL_3@h
	bl	__init_LPCR
	bl	__init_FSCR
	bl	__init_TLB
	mtlr	r11
	blr

_GLOBAL(__restore_cpu_power8)
	mflr	r11
	mfmsr	r3
	rldicl.	r0,r3,4,63
	beqlr
	li	r0,0

__init_FSCR:
	mfspr	r3,SPRN_FSCR
	ori	r3,r3,FSCR_TAR
	mtspr	SPRN_FSCR,r3
	blr

__init_TLB: