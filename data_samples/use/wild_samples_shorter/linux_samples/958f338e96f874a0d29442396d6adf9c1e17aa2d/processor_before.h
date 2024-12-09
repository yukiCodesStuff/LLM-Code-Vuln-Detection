
extern void cpu_detect(struct cpuinfo_x86 *c);

extern void early_cpu_init(void);
extern void identify_boot_cpu(void);
extern void identify_secondary_cpu(struct cpuinfo_x86 *);
extern void print_cpu_info(struct cpuinfo_x86 *);
void stop_this_cpu(void *dummy);
void df_debug(struct pt_regs *regs, long error_code);
void microcode_check(void);
#endif /* _ASM_X86_PROCESSOR_H */