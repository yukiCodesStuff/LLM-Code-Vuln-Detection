{
	/* ax gets execve's return value. */
	/*regs->ax = */ regs->bx = regs->cx = regs->dx = 0;
	regs->si = regs->di = regs->bp = 0;
	regs->r8 = regs->r9 = regs->r10 = regs->r11 = 0;
	regs->r12 = regs->r13 = regs->r14 = regs->r15 = 0;
	t->fsbase = t->gsbase = 0;
	t->fsindex = t->gsindex = 0;
	t->ds = t->es = ds;
}

#define ELF_PLAT_INIT(_r, load_addr)			\
	elf_common_init(&current->thread, _r, 0)

#define	COMPAT_ELF_PLAT_INIT(regs, load_addr)		\
	elf_common_init(&current->thread, regs, __USER_DS)

void compat_start_thread(struct pt_regs *regs, u32 new_ip, u32 new_sp);
#define compat_start_thread compat_start_thread

void set_personality_ia32(bool);
#define COMPAT_SET_PERSONALITY(ex)			\
	set_personality_ia32((ex).e_machine == EM_X86_64)

#define COMPAT_ELF_PLATFORM			("i686")

/*
 * regs is struct pt_regs, pr_reg is elf_gregset_t (which is
 * now struct_user_regs, they are different). Assumes current is the process
 * getting dumped.
 */

#define ELF_CORE_COPY_REGS(pr_reg, regs)			\
do {								\
	unsigned v;						\
	(pr_reg)[0] = (regs)->r15;				\
	(pr_reg)[1] = (regs)->r14;				\
	(pr_reg)[2] = (regs)->r13;				\
	(pr_reg)[3] = (regs)->r12;				\
	(pr_reg)[4] = (regs)->bp;				\
	(pr_reg)[5] = (regs)->bx;				\
	(pr_reg)[6] = (regs)->r11;				\
	(pr_reg)[7] = (regs)->r10;				\
	(pr_reg)[8] = (regs)->r9;				\
	(pr_reg)[9] = (regs)->r8;				\
	(pr_reg)[10] = (regs)->ax;				\
	(pr_reg)[11] = (regs)->cx;				\
	(pr_reg)[12] = (regs)->dx;				\
	(pr_reg)[13] = (regs)->si;				\
	(pr_reg)[14] = (regs)->di;				\
	(pr_reg)[15] = (regs)->orig_ax;				\
	(pr_reg)[16] = (regs)->ip;				\
	(pr_reg)[17] = (regs)->cs;				\
	(pr_reg)[18] = (regs)->flags;				\
	(pr_reg)[19] = (regs)->sp;				\
	(pr_reg)[20] = (regs)->ss;				\
	(pr_reg)[21] = current->thread.fsbase;			\
	(pr_reg)[22] = current->thread.gsbase;			\
	asm("movl %%ds,%0" : "=r" (v)); (pr_reg)[23] = v;	\
	asm("movl %%es,%0" : "=r" (v)); (pr_reg)[24] = v;	\
	asm("movl %%fs,%0" : "=r" (v)); (pr_reg)[25] = v;	\
	asm("movl %%gs,%0" : "=r" (v)); (pr_reg)[26] = v;	\
} while (0);

/* I'm not sure if we can use '-' here */
#define ELF_PLATFORM       ("x86_64")
extern void set_personality_64bit(void);
extern unsigned int sysctl_vsyscall32;
extern int force_personality32;

#endif /* !CONFIG_X86_32 */

#define CORE_DUMP_USE_REGSET
#define ELF_EXEC_PAGESIZE	4096

/*
 * This is the base location for PIE (ET_DYN with INTERP) loads. On
 * 64-bit, this is raised to 4GB to leave the entire 32-bit address
 * space open for things that want to use the area for 32-bit pointers.
 */
#define ELF_ET_DYN_BASE		(mmap_is_ia32() ? 0x000400000UL : \
						  0x100000000UL)

/* This yields a mask that user programs can use to figure out what
   instruction set this CPU supports.  This could be done in user space,
   but it's not easy, and we've already done it here.  */

#define ELF_HWCAP		(boot_cpu_data.x86_capability[CPUID_1_EDX])

extern u32 elf_hwcap2;

/*
 * HWCAP2 supplies mask with kernel enabled CPU features, so that
 * the application can discover that it can safely use them.
 * The bits are defined in uapi/asm/hwcap2.h.
 */
#define ELF_HWCAP2		(elf_hwcap2)

/* This yields a string that ld.so will use to load implementation
   specific libraries for optimization.  This is more specific in
   intent than poking at uname or /proc/cpuinfo.

   For the moment, we have only optimizations for the Intel generations,
   but that could change... */

#define SET_PERSONALITY(ex) set_personality_64bit()

/*
 * An executable for which elf_read_implies_exec() returns TRUE will
 * have the READ_IMPLIES_EXEC personality flag set automatically.
 */
#define elf_read_implies_exec(ex, executable_stack)	\
	(executable_stack != EXSTACK_DISABLE_X)

struct task_struct;

#define	ARCH_DLINFO_IA32						\
do {									\
	if (VDSO_CURRENT_BASE) {					\
		NEW_AUX_ENT(AT_SYSINFO,	VDSO_ENTRY);			\
		NEW_AUX_ENT(AT_SYSINFO_EHDR, VDSO_CURRENT_BASE);	\
	}								\
} while (0)

/*
 * True on X86_32 or when emulating IA32 on X86_64
 */
static inline int mmap_is_ia32(void)
{
	return IS_ENABLED(CONFIG_X86_32) ||
	       (IS_ENABLED(CONFIG_COMPAT) &&
		test_thread_flag(TIF_ADDR32));
}