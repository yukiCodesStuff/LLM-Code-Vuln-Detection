{
	unsigned int unused_page_mask;
	return follow_page_mask(vma, address, foll_flags, &unused_page_mask);
}

#define FOLL_WRITE	0x01	/* check pte is writable */
#define FOLL_TOUCH	0x02	/* mark page accessed */
#define FOLL_GET	0x04	/* do get_page on page */
#define FOLL_DUMP	0x08	/* give error on hole if it would be zero */
#define FOLL_FORCE	0x10	/* get_user_pages read/write w/o permission */
#define FOLL_NOWAIT	0x20	/* if a disk transfer is needed, start the IO
				 * and return without waiting upon it */
#define FOLL_POPULATE	0x40	/* fault in page */
#define FOLL_SPLIT	0x80	/* don't return transhuge pages, split them */
#define FOLL_HWPOISON	0x100	/* check page is hwpoisoned */
#define FOLL_NUMA	0x200	/* force NUMA hinting page fault */
#define FOLL_MIGRATION	0x400	/* wait for page to replace migration entry */
#define FOLL_TRIED	0x800	/* a retry, previous pass started an IO */
#define FOLL_MLOCK	0x1000	/* lock present pages */
#define FOLL_REMOTE	0x2000	/* we are working on non-current tsk/mm */
#define FOLL_COW	0x4000	/* internal GUP flag */
#define FOLL_ANON	0x8000	/* don't do file mappings */

static inline int vm_fault_to_errno(int vm_fault, int foll_flags)
{
	if (vm_fault & VM_FAULT_OOM)
		return -ENOMEM;
	if (vm_fault & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE))
		return (foll_flags & FOLL_HWPOISON) ? -EHWPOISON : -EFAULT;
	if (vm_fault & (VM_FAULT_SIGBUS | VM_FAULT_SIGSEGV))
		return -EFAULT;
	return 0;
}