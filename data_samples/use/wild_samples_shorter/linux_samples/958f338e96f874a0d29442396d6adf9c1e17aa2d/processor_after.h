
extern void cpu_detect(struct cpuinfo_x86 *c);

static inline unsigned long l1tf_pfn_limit(void)
{
	return BIT(boot_cpu_data.x86_phys_bits - 1 - PAGE_SHIFT) - 1;
}

extern void early_cpu_init(void);
extern void identify_boot_cpu(void);
extern void identify_secondary_cpu(struct cpuinfo_x86 *);
extern void print_cpu_info(struct cpuinfo_x86 *);
void stop_this_cpu(void *dummy);
void df_debug(struct pt_regs *regs, long error_code);
void microcode_check(void);

enum l1tf_mitigations {
	L1TF_MITIGATION_OFF,
	L1TF_MITIGATION_FLUSH_NOWARN,
	L1TF_MITIGATION_FLUSH,
	L1TF_MITIGATION_FLUSH_NOSMT,
	L1TF_MITIGATION_FULL,
	L1TF_MITIGATION_FULL_FORCE
};

extern enum l1tf_mitigations l1tf_mitigation;

#endif /* _ASM_X86_PROCESSOR_H */