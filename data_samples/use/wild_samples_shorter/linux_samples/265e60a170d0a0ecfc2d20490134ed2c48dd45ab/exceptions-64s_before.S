EXC_VIRT(program_check, 0x4700, 0x100, 0x700)
TRAMP_KVM(PACA_EXGEN, 0x700)
EXC_COMMON_BEGIN(program_check_common)
	EXCEPTION_PROLOG_COMMON(0x700, PACA_EXGEN)
	bl	save_nvgprs
	RECONCILE_IRQ_STATE(r10, r11)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	program_check_exception