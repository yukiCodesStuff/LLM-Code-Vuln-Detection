#include <asm/debug-monitors.h>
#include <asm/pgtable.h>
#include <asm/memory.h>
#include <asm/smp_plat.h>
#include <asm/suspend.h>
#include <asm/tlbflush.h>

	 */
	ret = __cpu_suspend_enter(arg, fn);
	if (ret == 0) {
		cpu_switch_mm(mm->pgd, mm);
		flush_tlb_all();

		/*
		 * Restore per-cpu offset before any kernel