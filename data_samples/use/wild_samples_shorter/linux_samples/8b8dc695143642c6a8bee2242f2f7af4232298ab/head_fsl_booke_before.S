
	/* System Call Interrupt */
	START_EXCEPTION(SystemCall)
	SYSCALL_ENTRY   0xc00 SYSCALL

	/* Auxiliary Processor Unavailable Interrupt */
	EXCEPTION(0x2900, AP_UNAVAIL, AuxillaryProcessorUnavailable, \
		  unknown_exception, EXC_XFER_STD)