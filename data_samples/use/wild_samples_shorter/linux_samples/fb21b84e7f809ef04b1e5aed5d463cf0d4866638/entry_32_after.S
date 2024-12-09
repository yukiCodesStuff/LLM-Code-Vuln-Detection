sysenter_badsys:
	movl $-ENOSYS,%eax
	jmp sysenter_after_call
END(sysenter_badsys)
	CFI_ENDPROC

.macro FIXUP_ESPFIX_STACK
/*