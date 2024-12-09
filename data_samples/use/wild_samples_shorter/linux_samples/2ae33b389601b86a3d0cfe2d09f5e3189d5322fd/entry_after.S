	UPDATE_VTIME %r14,%r15,__LC_MCCK_ENTER_TIMER
mcck_skip:
	SWITCH_ASYNC __LC_GPREGS_SAVE_AREA+32,__LC_PANIC_STACK,PAGE_SHIFT
	stm	%r0,%r7,__PT_R0(%r11)
	mvc	__PT_R8(32,%r11),__LC_GPREGS_SAVE_AREA+32
	stm	%r8,%r9,__PT_PSW(%r11)
	xc	__SF_BACKCHAIN(4,%r15),__SF_BACKCHAIN(%r15)
	l	%r1,BASED(.Ldo_machine_check)
	lr	%r2,%r11		# pass pointer to pt_regs