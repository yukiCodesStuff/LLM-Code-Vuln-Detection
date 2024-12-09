	testb	$3, CS-ORIG_RAX+8(%rsp)
	jz	1f
	SWAPGS

	/*
	 * Switch to the thread stack. The IRET frame and orig_ax are
	 * on the stack, as well as the return address. RDI..R12 are
	 * not (yet) on the stack and space has not (yet) been
	UNWIND_HINT_FUNC

	movq	(%rdi), %rdi
1:

	PUSH_AND_CLEAR_REGS save_ret=1
	ENCODE_FRAME_POINTER 8

	testb	$3, CS+8(%rsp)
	 */
	SAVE_AND_SWITCH_TO_KERNEL_CR3 scratch_reg=%rax save_reg=%r14

	ret
END(paranoid_entry)

/*
	 * from user mode due to an IRET fault.
	 */
	SWAPGS
	/* We have user CR3.  Change to kernel CR3. */
	SWITCH_TO_KERNEL_CR3 scratch_reg=%rax

.Lerror_entry_from_usermode_after_swapgs:
	pushq	%r12
	ret

.Lerror_entry_done:
	ret

	/*
	cmpq	%rax, RIP+8(%rsp)
	je	.Lbstep_iret
	cmpq	$.Lgs_change, RIP+8(%rsp)
	jne	.Lerror_entry_done

	/*
	 * hack: .Lgs_change can fail with user gsbase.  If this happens, fix up
	 * gsbase and proceed.  We'll fix up the exception and land in
	 * .Lgs_change's error handler with kernel gsbase.
	 */
	SWAPGS
	SWITCH_TO_KERNEL_CR3 scratch_reg=%rax
	jmp .Lerror_entry_done

.Lbstep_iret:
	 * gsbase and CR3.  Switch to kernel gsbase and CR3:
	 */
	SWAPGS
	SWITCH_TO_KERNEL_CR3 scratch_reg=%rax

	/*
	 * Pretend that the exception came from user mode: set up pt_regs

	swapgs
	cld
	SWITCH_TO_KERNEL_CR3 scratch_reg=%rdx
	movq	%rsp, %rdx
	movq	PER_CPU_VAR(cpu_current_top_of_stack), %rsp
	UNWIND_HINT_IRET_REGS base=%rdx offset=8