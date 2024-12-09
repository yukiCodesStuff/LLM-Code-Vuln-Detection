#include <linux/atomic.h>
#include <asm/cacheflush.h>
#include <asm/exception.h>
#include <asm/spectre.h>
#include <asm/unistd.h>
#include <asm/traps.h>
#include <asm/ptrace.h>
#include <asm/unwind.h>
}
#endif

#ifndef CONFIG_CPU_V7M
static void copy_from_lma(void *vma, void *lma_start, void *lma_end)
{
	memcpy(vma, lma_start, lma_end - lma_start);
}

static void flush_vectors(void *vma, size_t offset, size_t size)
{
	unsigned long start = (unsigned long)vma + offset;
	unsigned long end = start + size;

	flush_icache_range(start, end);
}

#ifdef CONFIG_HARDEN_BRANCH_HISTORY
int spectre_bhb_update_vectors(unsigned int method)
{
	extern char __vectors_bhb_bpiall_start[], __vectors_bhb_bpiall_end[];
	extern char __vectors_bhb_loop8_start[], __vectors_bhb_loop8_end[];
	void *vec_start, *vec_end;

	if (system_state >= SYSTEM_FREEING_INITMEM) {
		pr_err("CPU%u: Spectre BHB workaround too late - system vulnerable\n",
		       smp_processor_id());
		return SPECTRE_VULNERABLE;
	}

	switch (method) {
	case SPECTRE_V2_METHOD_LOOP8:
		vec_start = __vectors_bhb_loop8_start;
		vec_end = __vectors_bhb_loop8_end;
		break;

	case SPECTRE_V2_METHOD_BPIALL:
		vec_start = __vectors_bhb_bpiall_start;
		vec_end = __vectors_bhb_bpiall_end;
		break;

	default:
		pr_err("CPU%u: unknown Spectre BHB state %d\n",
		       smp_processor_id(), method);
		return SPECTRE_VULNERABLE;
	}

	copy_from_lma(vectors_page, vec_start, vec_end);
	flush_vectors(vectors_page, 0, vec_end - vec_start);

	return SPECTRE_MITIGATED;
}
#endif

void __init early_trap_init(void *vectors_base)
{
	extern char __stubs_start[], __stubs_end[];
	extern char __vectors_start[], __vectors_end[];
	unsigned i;

	 * into the vector page, mapped at 0xffff0000, and ensure these
	 * are visible to the instruction stream.
	 */
	copy_from_lma(vectors_base, __vectors_start, __vectors_end);
	copy_from_lma(vectors_base + 0x1000, __stubs_start, __stubs_end);

	kuser_init(vectors_base);

	flush_vectors(vectors_base, 0, PAGE_SIZE * 2);
}
#else /* ifndef CONFIG_CPU_V7M */
void __init early_trap_init(void *vectors_base)
{
	/*
	 * on V7-M there is no need to copy the vector table to a dedicated
	 * memory area. The address is configurable and so a table in the kernel
	 * image can be used.
	 */
}
#endif