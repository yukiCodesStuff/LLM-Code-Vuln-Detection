static DEFINE_SPINLOCK(mmu_context_lock);
static DEFINE_IDA(mmu_context_ida);

int __init_new_context(void)
{
	int index;
	int err;
	else if (err)
		return err;

	if (index > MAX_USER_CONTEXT) {
		spin_lock(&mmu_context_lock);
		ida_remove(&mmu_context_ida, index);
		spin_unlock(&mmu_context_lock);
		return -ENOMEM;