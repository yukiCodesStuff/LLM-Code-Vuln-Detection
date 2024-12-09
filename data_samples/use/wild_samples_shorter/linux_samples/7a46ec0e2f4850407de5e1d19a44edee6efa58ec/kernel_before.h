void do_exit(long error_code) __noreturn;
void complete_and_exit(struct completion *, long) __noreturn;

/* Internal, do not use. */
int __must_check _kstrtoul(const char *s, unsigned int base, unsigned long *res);
int __must_check _kstrtol(const char *s, unsigned int base, long *res);
