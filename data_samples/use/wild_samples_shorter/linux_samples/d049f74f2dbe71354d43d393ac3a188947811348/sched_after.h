extern void set_dumpable(struct mm_struct *mm, int value);
extern int get_dumpable(struct mm_struct *mm);

#define SUID_DUMP_DISABLE	0	/* No setuid dumping */
#define SUID_DUMP_USER		1	/* Dump as user of process */
#define SUID_DUMP_ROOT		2	/* Dump as root */

/* mm flags */
/* dumpable bits */
#define MMF_DUMPABLE      0  /* core dump is permitted */
#define MMF_DUMP_SECURELY 1  /* core file is readable only by root */