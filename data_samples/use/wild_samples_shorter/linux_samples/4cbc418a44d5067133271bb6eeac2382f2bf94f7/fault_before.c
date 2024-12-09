	const struct fault_info *inf;
	struct mm_struct *mm = current->mm;
	vm_fault_t fault, major = 0;
	unsigned long vm_flags = VM_READ | VM_WRITE;
	unsigned int mm_flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;

	if (kprobe_page_fault(regs, esr))
		return 0;