
static inline void __tlb_flush_mm(struct mm_struct * mm)
{
	/*
	 * If the machine has IDTE we prefer to do a per mm flush
	 * on all cpus instead of doing a local flush if the mm
	 * only ran on the local cpu.