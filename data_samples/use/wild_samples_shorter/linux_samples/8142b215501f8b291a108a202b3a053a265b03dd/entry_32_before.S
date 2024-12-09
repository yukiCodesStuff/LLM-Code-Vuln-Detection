	cmpl $(NR_syscalls), %eax
	jae sysenter_badsys
	call *sys_call_table(,%eax,4)
	movl %eax,PT_EAX(%esp)
sysenter_after_call:
	LOCKDEP_SYS_EXIT
	DISABLE_INTERRUPTS(CLBR_ANY)
	TRACE_IRQS_OFF
	movl TI_flags(%ebp), %ecx
	jae syscall_badsys
syscall_call:
	call *sys_call_table(,%eax,4)
	movl %eax,PT_EAX(%esp)		# store the return value
syscall_exit:
	LOCKDEP_SYS_EXIT
	DISABLE_INTERRUPTS(CLBR_ANY)	# make sure we don't miss an interrupt
END(syscall_fault)

syscall_badsys:
	movl $-ENOSYS,PT_EAX(%esp)
	jmp syscall_exit
END(syscall_badsys)

sysenter_badsys:
	movl $-ENOSYS,PT_EAX(%esp)
	jmp sysenter_after_call
END(syscall_badsys)
	CFI_ENDPROC
