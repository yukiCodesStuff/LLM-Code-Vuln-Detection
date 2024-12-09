#endif
.endm

.macro kuap_check_amr gpr1, gpr2
#ifdef CONFIG_PPC_KUAP_DEBUG
	BEGIN_MMU_FTR_SECTION_NESTED(67)
	mfspr	\gpr1, SPRN_AMR
	END_MMU_FTR_SECTION_NESTED_IFSET(MMU_FTR_RADIX_KUAP, 67)
#endif
.endm

.macro kuap_save_amr_and_lock gpr1, gpr2, use_cr, msr_pr_cr
#ifdef CONFIG_PPC_KUAP
	BEGIN_MMU_FTR_SECTION_NESTED(67)

#else /* !__ASSEMBLY__ */

#ifdef CONFIG_PPC_KUAP

#include <asm/mmu.h>
#include <asm/ptrace.h>

static inline unsigned long get_kuap(void)
{
	if (!early_mmu_has_feature(MMU_FTR_RADIX_KUAP))
		return 0;

	return mfspr(SPRN_AMR);
}

	isync();
}

static __always_inline void allow_user_access(void __user *to, const void __user *from,
					      unsigned long size, unsigned long dir)
{
	// This is written so we can resolve to a single case at build time
				       unsigned long size, unsigned long dir)
{
	set_kuap(AMR_KUAP_BLOCKED);
}

static inline unsigned long prevent_user_access_return(void)
{
	unsigned long flags = get_kuap();

	set_kuap(AMR_KUAP_BLOCKED);

	return flags;
}

static inline void restore_user_access(unsigned long flags)
{
	set_kuap(flags);
}

static inline bool
bad_kuap_fault(struct pt_regs *regs, unsigned long address, bool is_write)
{
	return WARN(mmu_has_feature(MMU_FTR_RADIX_KUAP) &&
		    (regs->kuap & (is_write ? AMR_KUAP_BLOCK_WRITE : AMR_KUAP_BLOCK_READ)),
		    "Bug: %s fault blocked by AMR!", is_write ? "Write" : "Read");
}
#else /* CONFIG_PPC_KUAP */
static inline void kuap_restore_amr(struct pt_regs *regs, unsigned long amr)
{
}

static inline void kuap_check_amr(void)
{
}

static inline unsigned long kuap_get_and_check_amr(void)
{
	return 0;
}
#endif /* CONFIG_PPC_KUAP */

#endif /* __ASSEMBLY__ */

#endif /* _ASM_POWERPC_BOOK3S_64_KUP_RADIX_H */