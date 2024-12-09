	UPDATE_VTIME %r14,__LC_MCCK_ENTER_TIMER
	LAST_BREAK %r14
mcck_skip:
	lghi	%r14,__LC_GPREGS_SAVE_AREA
	mvc	__PT_R0(128,%r11),0(%r14)
	stmg	%r8,%r9,__PT_PSW(%r11)
	xc	__SF_BACKCHAIN(8,%r15),__SF_BACKCHAIN(%r15)
	lgr	%r2,%r11		# pass pointer to pt_regs
	brasl	%r14,s390_do_machine_check