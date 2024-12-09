
	bool perm_ok; /* do not check permissions if true */
	bool ud;	/* inject an #UD if host doesn't support insn */
	bool tf;	/* TF value before instruction (after for syscall/sysret) */

	bool have_exception;
	struct x86_exception exception;
