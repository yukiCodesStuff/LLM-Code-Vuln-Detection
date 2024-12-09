void do_exit(long error_code) __noreturn;
void complete_and_exit(struct completion *, long) __noreturn;

#ifdef CONFIG_ARCH_HAS_REFCOUNT
void refcount_error_report(struct pt_regs *regs, const char *err);
#else
static inline void refcount_error_report(struct pt_regs *regs, const char *err)
{ }
#endif

/* Internal, do not use. */
int __must_check _kstrtoul(const char *s, unsigned int base, unsigned long *res);
int __must_check _kstrtol(const char *s, unsigned int base, long *res);
