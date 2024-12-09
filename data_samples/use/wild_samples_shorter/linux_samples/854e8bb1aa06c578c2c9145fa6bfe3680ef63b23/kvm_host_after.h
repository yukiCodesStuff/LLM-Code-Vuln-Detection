	kvm_queue_exception_e(vcpu, GP_VECTOR, error_code);
}

static inline u64 get_canonical(u64 la)
{
	return ((int64_t)la << 16) >> 16;
}

static inline bool is_noncanonical_address(u64 la)
{
#ifdef CONFIG_X86_64
	return get_canonical(la) != la;
#else
	return false;
#endif
}

#define TSS_IOPB_BASE_OFFSET 0x66
#define TSS_BASE_SIZE 0x68
#define TSS_IOPB_SIZE (65536 / 8)
#define TSS_REDIRECTION_SIZE (256 / 8)