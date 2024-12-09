#define __smp_mb__before_atomic()	__smp_mb__before_llsc()
#define __smp_mb__after_atomic()	smp_llsc_mb()

/*
 * Some Loongson 3 CPUs have a bug wherein execution of a memory access (load,
 * store or pref) in between an ll & sc can cause the sc instruction to
 * erroneously succeed, breaking atomicity. Whilst it's unusual to write code
 * containing such sequences, this bug bites harder than we might otherwise
 * expect due to reordering & speculation:
 *
 * 1) A memory access appearing prior to the ll in program order may actually
 *    be executed after the ll - this is the reordering case.
 *
 *    In order to avoid this we need to place a memory barrier (ie. a sync
 *    instruction) prior to every ll instruction, in between it & any earlier
 *    memory access instructions. Many of these cases are already covered by
 *    smp_mb__before_llsc() but for the remaining cases, typically ones in
 *    which multiple CPUs may operate on a memory location but ordering is not
 *    usually guaranteed, we use loongson_llsc_mb() below.
 *
 *    This reordering case is fixed by 3A R2 CPUs, ie. 3A2000 models and later.
 *
 * 2) If a conditional branch exists between an ll & sc with a target outside
 *    of the ll-sc loop, for example an exit upon value mismatch in cmpxchg()
 *    or similar, then misprediction of the branch may allow speculative
 *    execution of memory accesses from outside of the ll-sc loop.
 *
 *    In order to avoid this we need a memory barrier (ie. a sync instruction)
 *    at each affected branch target, for which we also use loongson_llsc_mb()
 *    defined below.
 *
 *    This case affects all current Loongson 3 CPUs.
 */
#ifdef CONFIG_CPU_LOONGSON3_WORKAROUNDS /* Loongson-3's LLSC workaround */
#define loongson_llsc_mb()	__asm__ __volatile__(__WEAK_LLSC_MB : : :"memory")
#else
#define loongson_llsc_mb()	do { } while (0)
#endif

#include <asm-generic/barrier.h>

#endif /* __ASM_BARRIER_H */