#include <linux/string.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#include <asm/ldt.h>
#include <asm/mmu_context.h>
#include <asm/syscalls.h>

#ifdef CONFIG_SMP
static void flush_ldt(void *current_mm)
{
	if (current->active_mm == current_mm)
		load_LDT(&current->active_mm->context);
}
#endif

static int alloc_ldt(mm_context_t *pc, int mincount, int reload)
{
	void *oldldt, *newldt;
	int oldsize;

	if (mincount <= pc->size)
		return 0;
	oldsize = pc->size;
	mincount = (mincount + (PAGE_SIZE / LDT_ENTRY_SIZE - 1)) &
			(~(PAGE_SIZE / LDT_ENTRY_SIZE - 1));
	if (mincount * LDT_ENTRY_SIZE > PAGE_SIZE)
		newldt = vmalloc(mincount * LDT_ENTRY_SIZE);
	else
		newldt = (void *)__get_free_page(GFP_KERNEL);

	if (!newldt)
		return -ENOMEM;

	if (oldsize)
		memcpy(newldt, pc->ldt, oldsize * LDT_ENTRY_SIZE);
	oldldt = pc->ldt;
	memset(newldt + oldsize * LDT_ENTRY_SIZE, 0,
	       (mincount - oldsize) * LDT_ENTRY_SIZE);

	paravirt_alloc_ldt(newldt, mincount);

#ifdef CONFIG_X86_64
	/* CHECKME: Do we really need this ? */
	wmb();
#endif
	pc->ldt = newldt;
	wmb();
	pc->size = mincount;
	wmb();

	if (reload) {
#ifdef CONFIG_SMP
		preempt_disable();
		load_LDT(pc);
		if (!cpumask_equal(mm_cpumask(current->mm),
				   cpumask_of(smp_processor_id())))
			smp_call_function(flush_ldt, current->mm, 1);
		preempt_enable();
#else
		load_LDT(pc);
#endif
	}
	if (oldsize) {
		paravirt_free_ldt(oldldt, oldsize);
		if (oldsize * LDT_ENTRY_SIZE > PAGE_SIZE)
			vfree(oldldt);
		else
			put_page(virt_to_page(oldldt));
	}
	return 0;
}

static inline int copy_ldt(mm_context_t *new, mm_context_t *old)
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

/*
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

static int read_ldt(void __user *ptr, unsigned long bytecount)
{
	int err;
	unsigned long size;
	struct mm_struct *mm = current->mm;

	if (!mm->context.size)
		return 0;
	if (bytecount > LDT_ENTRY_SIZE * LDT_ENTRIES)
		bytecount = LDT_ENTRY_SIZE * LDT_ENTRIES;

	mutex_lock(&mm->context.lock);
	size = mm->context.size * LDT_ENTRY_SIZE;
	if (size > bytecount)
		size = bytecount;

	err = 0;
	if (copy_to_user(ptr, mm->context.ldt, size))
		err = -EFAULT;
	mutex_unlock(&mm->context.lock);
	if (err < 0)
		goto error_return;
	if (size != bytecount) {
		/* zero-fill the rest */
		if (clear_user(ptr + size, bytecount - size) != 0) {
			err = -EFAULT;
			goto error_return;
		}
	}
	return bytecount;
error_return:
	return err;
}

static int read_default_ldt(void __user *ptr, unsigned long bytecount)
{
	struct desc_struct ldt;
	int error;
	struct user_desc ldt_info;

	error = -EINVAL;
	if (bytecount != sizeof(ldt_info))
		goto out;
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
	}

	if (!IS_ENABLED(CONFIG_X86_16BIT) && !ldt_info.seg_32bit) {
		error = -EINVAL;
		goto out_unlock;
	}

	fill_ldt(&ldt, &ldt_info);
	if (oldmode)
		ldt.avl = 0;

	/* Install the new entry ...  */
install:
	write_ldt_entry(mm->context.ldt, ldt_info.entry_number, &ldt);
	error = 0;

out_unlock:
	mutex_unlock(&mm->context.lock);