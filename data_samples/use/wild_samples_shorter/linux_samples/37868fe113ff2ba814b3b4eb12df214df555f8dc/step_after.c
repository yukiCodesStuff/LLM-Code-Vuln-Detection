#include <linux/mm.h>
#include <linux/ptrace.h>
#include <asm/desc.h>
#include <asm/mmu_context.h>

unsigned long convert_ip_to_linear(struct task_struct *child, struct pt_regs *regs)
{
	unsigned long addr, seg;
		seg &= ~7UL;

		mutex_lock(&child->mm->context.lock);
		if (unlikely(!child->mm->context.ldt ||
			     (seg >> 3) >= child->mm->context.ldt->size))
			addr = -1L; /* bogus selector, access would fault */
		else {
			desc = &child->mm->context.ldt->entries[seg];
			base = get_desc_base(desc);

			/* 16-bit code segment? */
			if (!desc->d)