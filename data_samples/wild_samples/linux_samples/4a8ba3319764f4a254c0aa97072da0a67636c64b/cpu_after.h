struct cpuinfo_arm64 {
	struct cpu	cpu;
	u32		reg_ctr;
	u32		reg_cntfrq;
	u32		reg_dczid;
	u32		reg_midr;

	u64		reg_id_aa64dfr0;
	u64		reg_id_aa64dfr1;
	u64		reg_id_aa64isar0;
	u64		reg_id_aa64isar1;
	u64		reg_id_aa64mmfr0;
	u64		reg_id_aa64mmfr1;
	u64		reg_id_aa64pfr0;
	u64		reg_id_aa64pfr1;

	u32		reg_id_dfr0;
	u32		reg_id_isar0;
	u32		reg_id_isar1;
	u32		reg_id_isar2;
	u32		reg_id_isar3;
	u32		reg_id_isar4;
	u32		reg_id_isar5;
	u32		reg_id_mmfr0;
	u32		reg_id_mmfr1;
	u32		reg_id_mmfr2;
	u32		reg_id_mmfr3;
	u32		reg_id_pfr0;
	u32		reg_id_pfr1;

	u32		reg_mvfr0;
	u32		reg_mvfr1;
	u32		reg_mvfr2;
};

DECLARE_PER_CPU(struct cpuinfo_arm64, cpu_data);

void cpuinfo_store_cpu(void);
void __init cpuinfo_store_boot_cpu(void);

#endif /* __ASM_CPU_H */
struct cpuinfo_arm64 {
	struct cpu	cpu;
	u32		reg_ctr;
	u32		reg_cntfrq;
	u32		reg_dczid;
	u32		reg_midr;

	u64		reg_id_aa64dfr0;
	u64		reg_id_aa64dfr1;
	u64		reg_id_aa64isar0;
	u64		reg_id_aa64isar1;
	u64		reg_id_aa64mmfr0;
	u64		reg_id_aa64mmfr1;
	u64		reg_id_aa64pfr0;
	u64		reg_id_aa64pfr1;

	u32		reg_id_dfr0;
	u32		reg_id_isar0;
	u32		reg_id_isar1;
	u32		reg_id_isar2;
	u32		reg_id_isar3;
	u32		reg_id_isar4;
	u32		reg_id_isar5;
	u32		reg_id_mmfr0;
	u32		reg_id_mmfr1;
	u32		reg_id_mmfr2;
	u32		reg_id_mmfr3;
	u32		reg_id_pfr0;
	u32		reg_id_pfr1;

	u32		reg_mvfr0;
	u32		reg_mvfr1;
	u32		reg_mvfr2;
};

DECLARE_PER_CPU(struct cpuinfo_arm64, cpu_data);

void cpuinfo_store_cpu(void);
void __init cpuinfo_store_boot_cpu(void);

#endif /* __ASM_CPU_H */