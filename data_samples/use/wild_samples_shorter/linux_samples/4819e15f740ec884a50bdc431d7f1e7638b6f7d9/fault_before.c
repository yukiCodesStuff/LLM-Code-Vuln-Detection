	return pmd_k;
}

void arch_sync_kernel_mappings(unsigned long start, unsigned long end)
{
	unsigned long addr;

	 */
	WARN_ON_ONCE(hw_error_code & X86_PF_PK);

	/* Was the fault spurious, caused by lazy TLB invalidation? */
	if (spurious_kernel_fault(hw_error_code, address))
		return;
