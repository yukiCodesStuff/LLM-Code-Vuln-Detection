
	/* System Call Interrupt */
	START_EXCEPTION(SystemCall)
	SYSCALL_ENTRY   0xc00 BOOKE_INTERRUPT_SYSCALL SPRN_SRR1

	/* Auxiliary Processor Unavailable Interrupt */
	EXCEPTION(0x2900, AP_UNAVAIL, AuxillaryProcessorUnavailable, \
		  unknown_exception, EXC_XFER_STD)