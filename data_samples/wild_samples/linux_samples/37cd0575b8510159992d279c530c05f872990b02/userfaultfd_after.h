struct uffdio_writeprotect {
	struct uffdio_range range;
/*
 * UFFDIO_WRITEPROTECT_MODE_WP: set the flag to write protect a range,
 * unset the flag to undo protection of a range which was previously
 * write protected.
 *
 * UFFDIO_WRITEPROTECT_MODE_DONTWAKE: set the flag to avoid waking up
 * any wait thread after the operation succeeds.
 *
 * NOTE: Write protecting a region (WP=1) is unrelated to page faults,
 * therefore DONTWAKE flag is meaningless with WP=1.  Removing write
 * protection (WP=0) in response to a page fault wakes the faulting
 * task unless DONTWAKE is set.
 */
#define UFFDIO_WRITEPROTECT_MODE_WP		((__u64)1<<0)
#define UFFDIO_WRITEPROTECT_MODE_DONTWAKE	((__u64)1<<1)
	__u64 mode;
};

/*
 * Flags for the userfaultfd(2) system call itself.
 */

/*
 * Create a userfaultfd that can handle page faults only in user mode.
 */
#define UFFD_USER_MODE_ONLY 1

#endif /* _LINUX_USERFAULTFD_H */