{
	if (dead_task->mm) {
		if (dead_task->mm->context.ldt) {
			pr_warn("WARNING: dead process %s still has LDT? <%p/%d>\n",
				dead_task->comm,
				dead_task->mm->context.ldt,
				dead_task->mm->context.ldt->size);
			BUG();
		}
	}
}