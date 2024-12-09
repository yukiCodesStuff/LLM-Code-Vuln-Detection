	FTR_ENTRY_OFFSET 955b-956b;			\
	.popsection;

#define UACCESS_FLUSH_FIXUP_SECTION			\
959:							\
	.pushsection __uaccess_flush_fixup,"a";		\
	.align 2;					\
960:							\
	FTR_ENTRY_OFFSET 959b-960b;			\
	.popsection;

#define ENTRY_FLUSH_FIXUP_SECTION			\
957:							\
	.pushsection __entry_flush_fixup,"a";		\
	.align 2;					\
958:							\
	FTR_ENTRY_OFFSET 957b-958b;			\
	.popsection;

#define RFI_FLUSH_FIXUP_SECTION				\
951:							\
	.pushsection __rfi_flush_fixup,"a";		\
	.align 2;					\
#include <linux/types.h>

extern long stf_barrier_fallback;
extern long entry_flush_fallback;
extern long __start___stf_entry_barrier_fixup, __stop___stf_entry_barrier_fixup;
extern long __start___stf_exit_barrier_fixup, __stop___stf_exit_barrier_fixup;
extern long __start___uaccess_flush_fixup, __stop___uaccess_flush_fixup;
extern long __start___entry_flush_fixup, __stop___entry_flush_fixup;
extern long __start___rfi_flush_fixup, __stop___rfi_flush_fixup;
extern long __start___barrier_nospec_fixup, __stop___barrier_nospec_fixup;
extern long __start__btb_flush_fixup, __stop__btb_flush_fixup;
