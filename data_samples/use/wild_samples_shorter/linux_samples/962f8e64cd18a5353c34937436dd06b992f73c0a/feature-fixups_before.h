	FTR_ENTRY_OFFSET 955b-956b;			\
	.popsection;

#define RFI_FLUSH_FIXUP_SECTION				\
951:							\
	.pushsection __rfi_flush_fixup,"a";		\
	.align 2;					\
#include <linux/types.h>

extern long stf_barrier_fallback;
extern long __start___stf_entry_barrier_fixup, __stop___stf_entry_barrier_fixup;
extern long __start___stf_exit_barrier_fixup, __stop___stf_exit_barrier_fixup;
extern long __start___rfi_flush_fixup, __stop___rfi_flush_fixup;
extern long __start___barrier_nospec_fixup, __stop___barrier_nospec_fixup;
extern long __start__btb_flush_fixup, __stop__btb_flush_fixup;
