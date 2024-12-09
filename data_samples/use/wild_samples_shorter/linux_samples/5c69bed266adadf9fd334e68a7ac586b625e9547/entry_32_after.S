	lea 16(%esp),%esp
	CFI_ADJUST_CFA_OFFSET -16
	jz 5f
	jmp iret_exc
5:	pushl_cfi $-1 /* orig_ax = -1 => not a system call */
	SAVE_ALL
	jmp ret_from_exception