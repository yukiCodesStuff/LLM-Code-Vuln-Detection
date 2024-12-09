{
	u64 val = PRED_CMD_IBPB;

	alternative_msr_write(MSR_IA32_PRED_CMD, val, X86_FEATURE_USE_IBPB);
}

/* The Intel SPEC CTRL MSR base value cache */
extern u64 x86_spec_ctrl_base;

/*
 * With retpoline, we must use IBRS to restrict branch prediction
 * before calling into firmware.
 *
 * (Implemented as CPP macros due to header hell.)
 */
#define firmware_restrict_branch_speculation_start()			\
do {									\
	u64 val = x86_spec_ctrl_base | SPEC_CTRL_IBRS;			\
									\
	preempt_disable();						\
	alternative_msr_write(MSR_IA32_SPEC_CTRL, val,			\
			      X86_FEATURE_USE_IBRS_FW);			\
} while (0)

#define firmware_restrict_branch_speculation_end()			\
do {									\
	u64 val = x86_spec_ctrl_base;					\
									\
	alternative_msr_write(MSR_IA32_SPEC_CTRL, val,			\
			      X86_FEATURE_USE_IBRS_FW);			\
	preempt_enable();						\
} while (0)

DECLARE_STATIC_KEY_FALSE(switch_to_cond_stibp);
DECLARE_STATIC_KEY_FALSE(switch_mm_cond_ibpb);
DECLARE_STATIC_KEY_FALSE(switch_mm_always_ibpb);

#endif /* __ASSEMBLY__ */

/*
 * Below is used in the eBPF JIT compiler and emits the byte sequence
 * for the following assembly:
 *
 * With retpolines configured:
 *
 *    callq do_rop
 *  spec_trap:
 *    pause
 *    lfence
 *    jmp spec_trap
 *  do_rop:
 *    mov %rax,(%rsp) for x86_64
 *    mov %edx,(%esp) for x86_32
 *    retq
 *
 * Without retpolines configured:
 *
 *    jmp *%rax for x86_64
 *    jmp *%edx for x86_32
 */
#ifdef CONFIG_RETPOLINE
# ifdef CONFIG_X86_64
#  define RETPOLINE_RAX_BPF_JIT_SIZE	17
#  define RETPOLINE_RAX_BPF_JIT()				\
do {								\
	EMIT1_off32(0xE8, 7);	 /* callq do_rop */		\
	/* spec_trap: */					\
	EMIT2(0xF3, 0x90);       /* pause */			\
	EMIT3(0x0F, 0xAE, 0xE8); /* lfence */			\
	EMIT2(0xEB, 0xF9);       /* jmp spec_trap */		\
	/* do_rop: */						\
	EMIT4(0x48, 0x89, 0x04, 0x24); /* mov %rax,(%rsp) */	\
	EMIT1(0xC3);             /* retq */			\
} while (0)
# else /* !CONFIG_X86_64 */
#  define RETPOLINE_EDX_BPF_JIT()				\
do {								\
	EMIT1_off32(0xE8, 7);	 /* call do_rop */		\
	/* spec_trap: */					\
	EMIT2(0xF3, 0x90);       /* pause */			\
	EMIT3(0x0F, 0xAE, 0xE8); /* lfence */			\
	EMIT2(0xEB, 0xF9);       /* jmp spec_trap */		\
	/* do_rop: */						\
	EMIT3(0x89, 0x14, 0x24); /* mov %edx,(%esp) */		\
	EMIT1(0xC3);             /* ret */			\
} while (0)
# endif
#else /* !CONFIG_RETPOLINE */
# ifdef CONFIG_X86_64
#  define RETPOLINE_RAX_BPF_JIT_SIZE	2
#  define RETPOLINE_RAX_BPF_JIT()				\
	EMIT2(0xFF, 0xE0);       /* jmp *%rax */
# else /* !CONFIG_X86_64 */
#  define RETPOLINE_EDX_BPF_JIT()				\
	EMIT2(0xFF, 0xE2)        /* jmp *%edx */
# endif
#endif

#endif /* _ASM_X86_NOSPEC_BRANCH_H_ */