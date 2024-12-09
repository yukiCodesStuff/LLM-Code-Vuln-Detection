	return (struct kvm_mmu_page *)page_private(page);
}

static inline u16 kvm_read_fs(void)
{
	u16 seg;
	asm("mov %%fs, %0" : "=g"(seg));
	return seg;
}

static inline u16 kvm_read_gs(void)
{
	u16 seg;
	asm("mov %%gs, %0" : "=g"(seg));
	return seg;
}

static inline u16 kvm_read_ldt(void)
{
	u16 ldt;
	asm("sldt %0" : "=g"(ldt));
	return ldt;
}

static inline void kvm_load_fs(u16 sel)
{
	asm("mov %0, %%fs" : : "rm"(sel));
}

static inline void kvm_load_gs(u16 sel)
{
	asm("mov %0, %%gs" : : "rm"(sel));
}

static inline void kvm_load_ldt(u16 sel)
{
	asm("lldt %0" : : "rm"(sel));
}