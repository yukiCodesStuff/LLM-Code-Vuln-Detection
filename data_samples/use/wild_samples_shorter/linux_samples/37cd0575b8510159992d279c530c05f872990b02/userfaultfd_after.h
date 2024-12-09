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