	jnz sysenter_audit
sysenter_do_call:
	cmpl $(NR_syscalls), %eax
	jae sysenter_badsys
	call *sys_call_table(,%eax,4)
	movl %eax,PT_EAX(%esp)
sysenter_after_call:
	LOCKDEP_SYS_EXIT
	DISABLE_INTERRUPTS(CLBR_ANY)
	TRACE_IRQS_OFF
	movl TI_flags(%ebp), %ecx

syscall_badsys:
	movl $-ENOSYS,PT_EAX(%esp)
	jmp syscall_exit
END(syscall_badsys)

sysenter_badsys:
	movl $-ENOSYS,PT_EAX(%esp)
	jmp sysenter_after_call
END(syscall_badsys)
	CFI_ENDPROC

.macro FIXUP_ESPFIX_STACK