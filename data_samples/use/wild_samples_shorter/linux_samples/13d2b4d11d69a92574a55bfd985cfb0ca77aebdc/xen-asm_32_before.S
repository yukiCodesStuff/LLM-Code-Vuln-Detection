	 */
#ifdef CONFIG_SMP
	GET_THREAD_INFO(%eax)
	movl TI_cpu(%eax), %eax
	movl __per_cpu_offset(,%eax,4), %eax
	mov xen_vcpu(%eax), %eax
#else
	movl xen_vcpu, %eax
#endif

	/* check IF state we're restoring */
	testb $X86_EFLAGS_IF>>8, 8+1+ESP_OFFSET(%esp)
	 * resuming the code, so we don't have to be worried about
	 * being preempted to another CPU.
	 */
	setz XEN_vcpu_info_mask(%eax)
xen_iret_start_crit:

	/* check for unmasked and pending */
	cmpw $0x0001, XEN_vcpu_info_pending(%eax)

	/*
	 * If there's something pending, mask events again so we can
	 * jump back into xen_hypervisor_callback. Otherwise do not
	 * touch XEN_vcpu_info_mask.
	 */
	jne 1f
	movb $1, XEN_vcpu_info_mask(%eax)

1:	popl %eax

	/*