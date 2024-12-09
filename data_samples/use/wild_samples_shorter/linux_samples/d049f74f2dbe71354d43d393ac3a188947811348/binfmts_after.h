extern void would_dump(struct linux_binprm *, struct file *);

extern int suid_dumpable;

/* Stack area protections */
#define EXSTACK_DEFAULT   0	/* Whatever the arch defaults to */
#define EXSTACK_DISABLE_X 1	/* Disable executable stacks */