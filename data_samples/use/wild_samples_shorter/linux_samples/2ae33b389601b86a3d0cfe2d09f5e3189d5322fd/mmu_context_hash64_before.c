static DEFINE_SPINLOCK(mmu_context_lock);
static DEFINE_IDA(mmu_context_ida);

/*
 * 256MB segment
 * The proto-VSID space has 2^(CONTEX_BITS + USER_ESID_BITS) - 1 segments
 * available for user mappings. Each segment contains 2^28 bytes. Each
 * context maps 2^46 bytes (64TB) so we can support 2^19-1 contexts
 * (19 == 37 + 28 - 46).
 */
#define MAX_CONTEXT	((1UL << CONTEXT_BITS) - 1)

int __init_new_context(void)
{
	int index;
	int err;
	else if (err)
		return err;

	if (index > MAX_CONTEXT) {
		spin_lock(&mmu_context_lock);
		ida_remove(&mmu_context_ida, index);
		spin_unlock(&mmu_context_lock);
		return -ENOMEM;