
#define X2APIC_MSR(r) (APIC_BASE_MSR + ((r) >> 4))

#define NR_LOADSTORE_MSRS 8

struct vmx_msrs {
	unsigned int		nr;
	u32                   idt_vectoring_info;
	ulong                 rflags;

	struct shared_msr_entry *guest_msrs;
	int                   nmsrs;
	int                   save_nmsrs;
	bool                  guest_msrs_ready;
#ifdef CONFIG_X86_64