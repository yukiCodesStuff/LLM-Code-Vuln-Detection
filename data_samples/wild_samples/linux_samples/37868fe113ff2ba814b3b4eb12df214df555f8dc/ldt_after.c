/*
 * Copyright (C) 1992 Krishna Balasubramanian and Linus Torvalds
 * Copyright (C) 1999 Ingo Molnar <mingo@redhat.com>
 * Copyright (C) 2002 Andi Kleen
 *
 * This handles calls from both 32bit and 64bit mode.
 */

#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#include <asm/ldt.h>
#include <asm/desc.h>
#include <asm/mmu_context.h>
#include <asm/syscalls.h>

/* context.lock is held for us, so we don't need any locking. */
static void flush_ldt(void *current_mm)
{
	mm_context_t *pc;

	if (current->active_mm != current_mm)
		return;

	pc = &current->active_mm->context;
	set_ldt(pc->ldt->entries, pc->ldt->size);
}
/*
 * Copyright (C) 1992 Krishna Balasubramanian and Linus Torvalds
 * Copyright (C) 1999 Ingo Molnar <mingo@redhat.com>
 * Copyright (C) 2002 Andi Kleen
 *
 * This handles calls from both 32bit and 64bit mode.
 */

#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#include <asm/ldt.h>
#include <asm/desc.h>
#include <asm/mmu_context.h>
#include <asm/syscalls.h>

/* context.lock is held for us, so we don't need any locking. */
static void flush_ldt(void *current_mm)
{
	mm_context_t *pc;

	if (current->active_mm != current_mm)
		return;

	pc = &current->active_mm->context;
	set_ldt(pc->ldt->entries, pc->ldt->size);
}
{
	if (likely(!ldt))
		return;

	paravirt_free_ldt(ldt->entries, ldt->size);
	if (ldt->size * LDT_ENTRY_SIZE > PAGE_SIZE)
		vfree(ldt->entries);
	else
		kfree(ldt->entries);
	kfree(ldt);
}

/*
 * we do not have to muck with descriptors here, that is
 * done in switch_mm() as needed.
 */
int init_new_context(struct task_struct *tsk, struct mm_struct *mm)
{
	struct ldt_struct *new_ldt;
	struct mm_struct *old_mm;
	int retval = 0;

	mutex_init(&mm->context.lock);
	old_mm = current->mm;
	if (!old_mm) {
		mm->context.ldt = NULL;
		return 0;
	}

	mutex_lock(&old_mm->context.lock);
	if (!old_mm->context.ldt) {
		mm->context.ldt = NULL;
		goto out_unlock;
	}

	new_ldt = alloc_ldt_struct(old_mm->context.ldt->size);
	if (!new_ldt) {
		retval = -ENOMEM;
		goto out_unlock;
	}

	memcpy(new_ldt->entries, old_mm->context.ldt->entries,
	       new_ldt->size * LDT_ENTRY_SIZE);
	finalize_ldt_struct(new_ldt);

	mm->context.ldt = new_ldt;

out_unlock:
	mutex_unlock(&old_mm->context.lock);
	return retval;
}
	if (!new_ldt) {
		retval = -ENOMEM;
		goto out_unlock;
	}

	memcpy(new_ldt->entries, old_mm->context.ldt->entries,
	       new_ldt->size * LDT_ENTRY_SIZE);
	finalize_ldt_struct(new_ldt);

	mm->context.ldt = new_ldt;

out_unlock:
	mutex_unlock(&old_mm->context.lock);
	return retval;
}

/*
 * No need to lock the MM as we are the last user
 *
 * 64bit: Don't touch the LDT register - we're already in the next thread.
 */
void destroy_context(struct mm_struct *mm)
{
	free_ldt_struct(mm->context.ldt);
	mm->context.ldt = NULL;
}
{
	struct mm_struct *mm = current->mm;
	struct desc_struct ldt;
	int error;
	struct user_desc ldt_info;
	int oldsize, newsize;
	struct ldt_struct *new_ldt, *old_ldt;

	error = -EINVAL;
	if (bytecount != sizeof(ldt_info))
		goto out;
	error = -EFAULT;
	if (copy_from_user(&ldt_info, ptr, sizeof(ldt_info)))
		goto out;

	error = -EINVAL;
	if (ldt_info.entry_number >= LDT_ENTRIES)
		goto out;
	if (ldt_info.contents == 3) {
		if (oldmode)
			goto out;
		if (ldt_info.seg_not_present == 0)
			goto out;
	}
	if (ldt_info.contents == 3) {
		if (oldmode)
			goto out;
		if (ldt_info.seg_not_present == 0)
			goto out;
	}

	if ((oldmode && !ldt_info.base_addr && !ldt_info.limit) ||
	    LDT_empty(&ldt_info)) {
		/* The user wants to clear the entry. */
		memset(&ldt, 0, sizeof(ldt));
	} else {
		if (!IS_ENABLED(CONFIG_X86_16BIT) && !ldt_info.seg_32bit) {
			error = -EINVAL;
			goto out;
		}

		fill_ldt(&ldt, &ldt_info);
		if (oldmode)
			ldt.avl = 0;
	}