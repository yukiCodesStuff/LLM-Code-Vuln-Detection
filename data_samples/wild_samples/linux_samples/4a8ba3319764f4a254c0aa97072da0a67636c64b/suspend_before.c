#include <linux/percpu.h>
#include <linux/slab.h>
#include <asm/cacheflush.h>
#include <asm/cpu_ops.h>
#include <asm/debug-monitors.h>
#include <asm/pgtable.h>
#include <asm/memory.h>
#include <asm/smp_plat.h>
#include <asm/suspend.h>
#include <asm/tlbflush.h>

extern int __cpu_suspend_enter(unsigned long arg, int (*fn)(unsigned long));
/*
 * This is called by __cpu_suspend_enter() to save the state, and do whatever
 * flushing is required to ensure that when the CPU goes to sleep we have
 * the necessary data available when the caches are not searched.
 *
 * ptr: CPU context virtual address
 * save_ptr: address of the location where the context physical address
 *           must be saved
 */
void notrace __cpu_suspend_save(struct cpu_suspend_ctx *ptr,
				phys_addr_t *save_ptr)
{
	*save_ptr = virt_to_phys(ptr);

	cpu_do_suspend(ptr);
	/*
	 * Only flush the context that must be retrieved with the MMU
	 * off. VA primitives ensure the flush is applied to all
	 * cache levels so context is pushed to DRAM.
	 */
	__flush_dcache_area(ptr, sizeof(*ptr));
	__flush_dcache_area(save_ptr, sizeof(*save_ptr));
}
{
	struct mm_struct *mm = current->active_mm;
	int ret;
	unsigned long flags;

	/*
	 * From this point debug exceptions are disabled to prevent
	 * updates to mdscr register (saved and restored along with
	 * general purpose registers) from kernel debuggers.
	 */
	local_dbg_save(flags);

	/*
	 * mm context saved on the stack, it will be restored when
	 * the cpu comes out of reset through the identity mapped
	 * page tables, so that the thread address space is properly
	 * set-up on function return.
	 */
	ret = __cpu_suspend_enter(arg, fn);
	if (ret == 0) {
		cpu_switch_mm(mm->pgd, mm);
		flush_tlb_all();

		/*
		 * Restore per-cpu offset before any kernel
		 * subsystem relying on it has a chance to run.
		 */
		set_my_cpu_offset(per_cpu_offset(smp_processor_id()));

		/*
		 * Restore HW breakpoint registers to sane values
		 * before debug exceptions are possibly reenabled
		 * through local_dbg_restore.
		 */
		if (hw_breakpoint_restore)
			hw_breakpoint_restore(NULL);
	}