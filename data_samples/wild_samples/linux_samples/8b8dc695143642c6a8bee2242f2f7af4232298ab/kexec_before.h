{
	return crashing_cpu >= 0;
}

#ifdef CONFIG_KEXEC_FILE
extern const struct kexec_file_ops kexec_elf64_ops;

#ifdef CONFIG_IMA_KEXEC
#define ARCH_HAS_KIMAGE_ARCH

struct kimage_arch {
	phys_addr_t ima_buffer_addr;
	size_t ima_buffer_size;
};
#endif

int setup_purgatory(struct kimage *image, const void *slave_code,
		    const void *fdt, unsigned long kernel_load_addr,
		    unsigned long fdt_load_addr);
int setup_new_fdt(const struct kimage *image, void *fdt,
		  unsigned long initrd_load_addr, unsigned long initrd_len,
		  const char *cmdline);
int delete_fdt_mem_rsv(void *fdt, unsigned long start, unsigned long size);
#endif /* CONFIG_KEXEC_FILE */

#else /* !CONFIG_KEXEC_CORE */
static inline void crash_kexec_secondary(struct pt_regs *regs) { }

static inline int overlaps_crashkernel(unsigned long start, unsigned long size)
{
	return 0;
}

static inline void reserve_crashkernel(void) { ; }

static inline int crash_shutdown_register(crash_shutdown_t handler)
{
	return 0;
}

static inline int crash_shutdown_unregister(crash_shutdown_t handler)
{
	return 0;
}

static inline bool kdump_in_progress(void)
{
	return false;
}

static inline void crash_ipi_callback(struct pt_regs *regs) { }

static inline void crash_send_ipi(void (*crash_ipi_callback)(struct pt_regs *))
{
}

#endif /* CONFIG_KEXEC_CORE */
#endif /* ! __ASSEMBLY__ */
#endif /* __KERNEL__ */
#endif /* _ASM_POWERPC_KEXEC_H */