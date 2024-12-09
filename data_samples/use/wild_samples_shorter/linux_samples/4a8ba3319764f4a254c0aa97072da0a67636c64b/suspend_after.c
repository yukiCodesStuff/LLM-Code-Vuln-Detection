#include <asm/debug-monitors.h>
#include <asm/pgtable.h>
#include <asm/memory.h>
#include <asm/mmu_context.h>
#include <asm/smp_plat.h>
#include <asm/suspend.h>
#include <asm/tlbflush.h>

	 */
	ret = __cpu_suspend_enter(arg, fn);
	if (ret == 0) {
		/*
		 * We are resuming from reset with TTBR0_EL1 set to the
		 * idmap to enable the MMU; restore the active_mm mappings in
		 * TTBR0_EL1 unless the active_mm == &init_mm, in which case
		 * the thread entered __cpu_suspend with TTBR0_EL1 set to
		 * reserved TTBR0 page tables and should be restored as such.
		 */
		if (mm == &init_mm)
			cpu_set_reserved_ttbr0();
		else
			cpu_switch_mm(mm->pgd, mm);

		flush_tlb_all();

		/*
		 * Restore per-cpu offset before any kernel