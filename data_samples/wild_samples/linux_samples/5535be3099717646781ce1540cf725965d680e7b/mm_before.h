{
	if (err == -ENOMEM)
		return VM_FAULT_OOM;
	return VM_FAULT_SIGBUS;
}

struct page *follow_page(struct vm_area_struct *vma, unsigned long address,
			 unsigned int foll_flags);

#define FOLL_WRITE	0x01	/* check pte is writable */
#define FOLL_TOUCH	0x02	/* mark page accessed */
#define FOLL_GET	0x04	/* do get_page on page */
#define FOLL_DUMP	0x08	/* give error on hole if it would be zero */
#define FOLL_FORCE	0x10	/* get_user_pages read/write w/o permission */
#define FOLL_NOWAIT	0x20	/* if a disk transfer is needed, start the IO
				 * and return without waiting upon it */
#define FOLL_NOFAULT	0x80	/* do not fault in pages */
#define FOLL_HWPOISON	0x100	/* check page is hwpoisoned */
#define FOLL_NUMA	0x200	/* force NUMA hinting page fault */
#define FOLL_MIGRATION	0x400	/* wait for page to replace migration entry */
#define FOLL_TRIED	0x800	/* a retry, previous pass started an IO */
#define FOLL_REMOTE	0x2000	/* we are working on non-current tsk/mm */
#define FOLL_COW	0x4000	/* internal GUP flag */
#define FOLL_ANON	0x8000	/* don't do file mappings */
#define FOLL_LONGTERM	0x10000	/* mapping lifetime is indefinite: see below */
#define FOLL_SPLIT_PMD	0x20000	/* split huge pmd before returning */
#define FOLL_PIN	0x40000	/* pages must be released via unpin_user_page */
#define FOLL_FAST_ONLY	0x80000	/* gup_fast: prevent fall-back to slow gup */

/*
 * FOLL_PIN and FOLL_LONGTERM may be used in various combinations with each
 * other. Here is what they mean, and how to use them:
 *
 * FOLL_LONGTERM indicates that the page will be held for an indefinite time
 * period _often_ under userspace control.  This is in contrast to
 * iov_iter_get_pages(), whose usages are transient.
 *
 * FIXME: For pages which are part of a filesystem, mappings are subject to the
 * lifetime enforced by the filesystem and we need guarantees that longterm
 * users like RDMA and V4L2 only establish mappings which coordinate usage with
 * the filesystem.  Ideas for this coordination include revoking the longterm
 * pin, delaying writeback, bounce buffer page writeback, etc.  As FS DAX was
 * added after the problem with filesystems was found FS DAX VMAs are
 * specifically failed.  Filesystem pages are still subject to bugs and use of
 * FOLL_LONGTERM should be avoided on those pages.
 *
 * FIXME: Also NOTE that FOLL_LONGTERM is not supported in every GUP call.
 * Currently only get_user_pages() and get_user_pages_fast() support this flag
 * and calls to get_user_pages_[un]locked are specifically not allowed.  This
 * is due to an incompatibility with the FS DAX check and
 * FAULT_FLAG_ALLOW_RETRY.
 *
 * In the CMA case: long term pins in a CMA region would unnecessarily fragment
 * that region.  And so, CMA attempts to migrate the page before pinning, when
 * FOLL_LONGTERM is specified.
 *
 * FOLL_PIN indicates that a special kind of tracking (not just page->_refcount,
 * but an additional pin counting system) will be invoked. This is intended for
 * anything that gets a page reference and then touches page data (for example,
 * Direct IO). This lets the filesystem know that some non-file-system entity is
 * potentially changing the pages' data. In contrast to FOLL_GET (whose pages
 * are released via put_page()), FOLL_PIN pages must be released, ultimately, by
 * a call to unpin_user_page().
 *
 * FOLL_PIN is similar to FOLL_GET: both of these pin pages. They use different
 * and separate refcounting mechanisms, however, and that means that each has
 * its own acquire and release mechanisms:
 *
 *     FOLL_GET: get_user_pages*() to acquire, and put_page() to release.
 *
 *     FOLL_PIN: pin_user_pages*() to acquire, and unpin_user_pages to release.
 *
 * FOLL_PIN and FOLL_GET are mutually exclusive for a given function call.
 * (The underlying pages may experience both FOLL_GET-based and FOLL_PIN-based
 * calls applied to them, and that's perfectly OK. This is a constraint on the
 * callers, not on the pages.)
 *
 * FOLL_PIN should be set internally by the pin_user_pages*() APIs, never
 * directly by the caller. That's in order to help avoid mismatches when
 * releasing pages: get_user_pages*() pages must be released via put_page(),
 * while pin_user_pages*() pages must be released via unpin_user_page().
 *
 * Please see Documentation/core-api/pin_user_pages.rst for more information.
 */

static inline int vm_fault_to_errno(vm_fault_t vm_fault, int foll_flags)
{
	if (vm_fault & VM_FAULT_OOM)
		return -ENOMEM;
	if (vm_fault & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE))
		return (foll_flags & FOLL_HWPOISON) ? -EHWPOISON : -EFAULT;
	if (vm_fault & (VM_FAULT_SIGBUS | VM_FAULT_SIGSEGV))
		return -EFAULT;
	return 0;
}