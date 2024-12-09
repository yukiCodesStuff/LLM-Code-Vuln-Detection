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
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#include <asm/ldt.h>
#include <asm/desc.h>
#include <asm/mmu_context.h>
#include <asm/syscalls.h>

#ifdef CONFIG_SMP
static void flush_ldt(void *current_mm)
{
	if (current->active_mm == current_mm)
		load_LDT(&current->active_mm->context);
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
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#include <asm/ldt.h>
#include <asm/desc.h>
#include <asm/mmu_context.h>
#include <asm/syscalls.h>

#ifdef CONFIG_SMP
static void flush_ldt(void *current_mm)
{
	if (current->active_mm == current_mm)
		load_LDT(&current->active_mm->context);
}
{
	int err = alloc_ldt(new, old->size, 0);
	int i;

	if (err < 0)
		return err;

	for (i = 0; i < old->size; i++)
		write_ldt_entry(new->ldt, i, old->ldt + i * LDT_ENTRY_SIZE);
	return 0;
}

/*
 * we do not have to muck with descriptors here, that is
 * done in switch_mm() as needed.
 */
int init_new_context(struct task_struct *tsk, struct mm_struct *mm)
{
	struct mm_struct *old_mm;
	int retval = 0;

	mutex_init(&mm->context.lock);
	mm->context.size = 0;
	old_mm = current->mm;
	if (old_mm && old_mm->context.size > 0) {
		mutex_lock(&old_mm->context.lock);
		retval = copy_ldt(&mm->context, &old_mm->context);
		mutex_unlock(&old_mm->context.lock);
	}
	return retval;
}
	if (old_mm && old_mm->context.size > 0) {
		mutex_lock(&old_mm->context.lock);
		retval = copy_ldt(&mm->context, &old_mm->context);
		mutex_unlock(&old_mm->context.lock);
	}
	return retval;
}

/*
 * No need to lock the MM as we are the last user
 *
 * 64bit: Don't touch the LDT register - we're already in the next thread.
 */
void destroy_context(struct mm_struct *mm)
{
	if (mm->context.size) {
#ifdef CONFIG_X86_32
		/* CHECKME: Can this ever happen ? */
		if (mm == current->active_mm)
			clear_LDT();
#endif
		paravirt_free_ldt(mm->context.ldt, mm->context.size);
		if (mm->context.size * LDT_ENTRY_SIZE > PAGE_SIZE)
			vfree(mm->context.ldt);
		else
			put_page(virt_to_page(mm->context.ldt));
		mm->context.size = 0;
	}
}
{
	struct mm_struct *mm = current->mm;
	struct desc_struct ldt;
	int error;
	struct user_desc ldt_info;

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

	mutex_lock(&mm->context.lock);
	if (ldt_info.entry_number >= mm->context.size) {
		error = alloc_ldt(&current->mm->context,
				  ldt_info.entry_number + 1, 1);
		if (error < 0)
			goto out_unlock;
	}

	/* Allow LDTs to be cleared by the user. */
	if (ldt_info.base_addr == 0 && ldt_info.limit == 0) {
		if (oldmode || LDT_empty(&ldt_info)) {
			memset(&ldt, 0, sizeof(ldt));
			goto install;
		}