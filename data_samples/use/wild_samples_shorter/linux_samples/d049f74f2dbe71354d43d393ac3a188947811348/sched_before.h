extern void set_dumpable(struct mm_struct *mm, int value);
extern int get_dumpable(struct mm_struct *mm);

/* mm flags */
/* dumpable bits */
#define MMF_DUMPABLE      0  /* core dump is permitted */
#define MMF_DUMP_SECURELY 1  /* core file is readable only by root */