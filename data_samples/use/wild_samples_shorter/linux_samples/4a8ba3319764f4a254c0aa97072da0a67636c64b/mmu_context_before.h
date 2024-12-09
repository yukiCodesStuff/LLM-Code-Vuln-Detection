static inline void arch_unmap(struct mm_struct *mm, struct vm_area_struct *vma,
			      unsigned long start, unsigned long end)
{
	mpx_notify_unmap(mm, vma, start, end);
}

#endif /* _ASM_X86_MMU_CONTEXT_H */