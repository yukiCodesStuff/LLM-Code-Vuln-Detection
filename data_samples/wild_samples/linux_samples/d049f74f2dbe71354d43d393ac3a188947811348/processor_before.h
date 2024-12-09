struct thread_struct {
	__u32 flags;			/* various thread flags (see IA64_THREAD_*) */
	/* writing on_ustack is performance-critical, so it's worth spending 8 bits on it... */
	__u8 on_ustack;			/* executing on user-stacks? */
	__u8 pad[3];
	__u64 ksp;			/* kernel stack pointer */
	__u64 map_base;			/* base address for get_unmapped_area() */
	__u64 rbs_bot;			/* the base address for the RBS */
	int last_fph_cpu;		/* CPU that may hold the contents of f32-f127 */

#ifdef CONFIG_PERFMON
	void *pfm_context;		     /* pointer to detailed PMU context */
	unsigned long pfm_needs_checking;    /* when >0, pending perfmon work on kernel exit */
# define INIT_THREAD_PM		.pfm_context =		NULL,     \
				.pfm_needs_checking =	0UL,
#else
# define INIT_THREAD_PM
#endif
	unsigned long dbr[IA64_NUM_DBG_REGS];
	unsigned long ibr[IA64_NUM_DBG_REGS];
	struct ia64_fpreg fph[96];	/* saved/loaded on demand */
};

#define INIT_THREAD {						\
	.flags =	0,					\
	.on_ustack =	0,					\
	.ksp =		0,					\
	.map_base =	DEFAULT_MAP_BASE,			\
	.rbs_bot =	STACK_TOP - DEFAULT_USER_STACK_SIZE,	\
	.last_fph_cpu =  -1,					\
	INIT_THREAD_PM						\
	.dbr =		{0, },					\
	.ibr =		{0, },					\
	.fph =		{{{{0}}}, }				\
}

#define start_thread(regs,new_ip,new_sp) do {							\
	regs->cr_ipsr = ((regs->cr_ipsr | (IA64_PSR_BITS_TO_SET | IA64_PSR_CPL))		\
			 & ~(IA64_PSR_BITS_TO_CLEAR | IA64_PSR_RI | IA64_PSR_IS));		\
	regs->cr_iip = new_ip;									\
	regs->ar_rsc = 0xf;		/* eager mode, privilege level 3 */			\
	regs->ar_rnat = 0;									\
	regs->ar_bspstore = current->thread.rbs_bot;						\
	regs->ar_fpsr = FPSR_DEFAULT;								\
	regs->loadrs = 0;									\
	regs->r8 = get_dumpable(current->mm);	/* set "don't zap registers" flag */		\
	regs->r12 = new_sp - 16;	/* allocate 16 byte scratch area */			\
	if (unlikely(!get_dumpable(current->mm))) {							\
		/*										\
		 * Zap scratch regs to avoid leaking bits between processes with different	\
		 * uid/privileges.								\
		 */										\
		regs->ar_pfs = 0; regs->b0 = 0; regs->pr = 0;					\
		regs->r1 = 0; regs->r9  = 0; regs->r11 = 0; regs->r13 = 0; regs->r15 = 0;	\
	}											\
} while (0)

/* Forward declarations, a strange C thing... */
struct mm_struct;
struct task_struct;

/*
 * Free all resources held by a thread. This is called after the
 * parent of DEAD_TASK has collected the exit status of the task via
 * wait().
 */
#define release_thread(dead_task)

/* Get wait channel for task P.  */
extern unsigned long get_wchan (struct task_struct *p);

/* Return instruction pointer of blocked task TSK.  */
#define KSTK_EIP(tsk)					\
  ({							\
	struct pt_regs *_regs = task_pt_regs(tsk);	\
	_regs->cr_iip + ia64_psr(_regs)->ri;		\
  })

/* Return stack pointer of blocked task TSK.  */
#define KSTK_ESP(tsk)  ((tsk)->thread.ksp)

extern void ia64_getreg_unknown_kr (void);
extern void ia64_setreg_unknown_kr (void);

#define ia64_get_kr(regnum)					\
({								\
	unsigned long r = 0;					\
								\
	switch (regnum) {					\
	    case 0: r = ia64_getreg(_IA64_REG_AR_KR0); break;	\
	    case 1: r = ia64_getreg(_IA64_REG_AR_KR1); break;	\
	    case 2: r = ia64_getreg(_IA64_REG_AR_KR2); break;	\
	    case 3: r = ia64_getreg(_IA64_REG_AR_KR3); break;	\
	    case 4: r = ia64_getreg(_IA64_REG_AR_KR4); break;	\
	    case 5: r = ia64_getreg(_IA64_REG_AR_KR5); break;	\
	    case 6: r = ia64_getreg(_IA64_REG_AR_KR6); break;	\
	    case 7: r = ia64_getreg(_IA64_REG_AR_KR7); break;	\
	    default: ia64_getreg_unknown_kr(); break;		\
	}							\
	r;							\
})

#define ia64_set_kr(regnum, r) 					\
({								\
	switch (regnum) {					\
	    case 0: ia64_setreg(_IA64_REG_AR_KR0, r); break;	\
	    case 1: ia64_setreg(_IA64_REG_AR_KR1, r); break;	\
	    case 2: ia64_setreg(_IA64_REG_AR_KR2, r); break;	\
	    case 3: ia64_setreg(_IA64_REG_AR_KR3, r); break;	\
	    case 4: ia64_setreg(_IA64_REG_AR_KR4, r); break;	\
	    case 5: ia64_setreg(_IA64_REG_AR_KR5, r); break;	\
	    case 6: ia64_setreg(_IA64_REG_AR_KR6, r); break;	\
	    case 7: ia64_setreg(_IA64_REG_AR_KR7, r); break;	\
	    default: ia64_setreg_unknown_kr(); break;		\
	}							\
})

/*
 * The following three macros can't be inline functions because we don't have struct
 * task_struct at this point.
 */

/*
 * Return TRUE if task T owns the fph partition of the CPU we're running on.
 * Must be called from code that has preemption disabled.
 */
#define ia64_is_local_fpu_owner(t)								\
({												\
	struct task_struct *__ia64_islfo_task = (t);						\
	(__ia64_islfo_task->thread.last_fph_cpu == smp_processor_id()				\
	 && __ia64_islfo_task == (struct task_struct *) ia64_get_kr(IA64_KR_FPU_OWNER));	\
})

/*
 * Mark task T as owning the fph partition of the CPU we're running on.
 * Must be called from code that has preemption disabled.
 */
#define ia64_set_local_fpu_owner(t) do {						\
	struct task_struct *__ia64_slfo_task = (t);					\
	__ia64_slfo_task->thread.last_fph_cpu = smp_processor_id();			\
	ia64_set_kr(IA64_KR_FPU_OWNER, (unsigned long) __ia64_slfo_task);		\
} while (0)

/* Mark the fph partition of task T as being invalid on all CPUs.  */
#define ia64_drop_fpu(t)	((t)->thread.last_fph_cpu = -1)

extern void __ia64_init_fpu (void);
extern void __ia64_save_fpu (struct ia64_fpreg *fph);
extern void __ia64_load_fpu (struct ia64_fpreg *fph);
extern void ia64_save_debug_regs (unsigned long *save_area);
extern void ia64_load_debug_regs (unsigned long *save_area);

#define ia64_fph_enable()	do { ia64_rsm(IA64_PSR_DFH); ia64_srlz_d(); } while (0)
#define ia64_fph_disable()	do { ia64_ssm(IA64_PSR_DFH); ia64_srlz_d(); } while (0)

/* load fp 0.0 into fph */
static inline void
ia64_init_fpu (void) {
	ia64_fph_enable();
	__ia64_init_fpu();
	ia64_fph_disable();
}