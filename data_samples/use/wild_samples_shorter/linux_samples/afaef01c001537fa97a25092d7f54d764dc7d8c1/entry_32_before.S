#include <asm/frame.h>
#include <asm/nospec-branch.h>

	.section .entry.text, "ax"

/*
 * We use macros for low-level operations which need to be overridden
	/* When we fork, we trace the syscall return in the child, too. */
	movl    %esp, %eax
	call    syscall_return_slowpath
	jmp     restore_all

	/* kernel thread */
1:	movl	%edi, %eax
	ALTERNATIVE "testl %eax, %eax; jz .Lsyscall_32_done", \
		    "jmp .Lsyscall_32_done", X86_FEATURE_XENPV

/* Opportunistic SYSEXIT */
	TRACE_IRQS_ON			/* User mode traces as IRQs on. */

	/*
	call	do_int80_syscall_32
.Lsyscall_32_done:

restore_all:
	TRACE_IRQS_IRET
	SWITCH_TO_ENTRY_STACK
.Lrestore_all_notrace: