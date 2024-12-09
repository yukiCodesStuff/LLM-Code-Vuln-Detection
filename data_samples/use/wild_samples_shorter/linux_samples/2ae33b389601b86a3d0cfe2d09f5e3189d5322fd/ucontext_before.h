	stack_t		  uc_stack;
	sigset_t	  uc_sigmask;
	/* glibc uses a 1024-bit sigset_t */
	__u8		  __unused[(1024 - sizeof(sigset_t)) / 8];
	/* last for future expansion */
	struct sigcontext uc_mcontext;
};
