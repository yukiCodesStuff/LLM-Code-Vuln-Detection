	mfspr	r11,SPRN_DSISR		/* Save DSISR */
	std	r11,_DSISR(r1)
	std	r9,_CCR(r1)		/* Save CR in stackframe */
	/* We don't touch AMR here, we never go to virtual mode */
	/* Save r9 through r13 from EXMC save area to stack frame. */
	EXCEPTION_PROLOG_COMMON_2(PACA_EXMC)
	mfmsr	r11			/* get MSR value */
BEGIN_FTR_SECTION