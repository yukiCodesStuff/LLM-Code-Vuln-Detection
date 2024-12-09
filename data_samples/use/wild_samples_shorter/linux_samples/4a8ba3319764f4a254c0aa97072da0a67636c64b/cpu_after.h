	u64		reg_id_aa64pfr0;
	u64		reg_id_aa64pfr1;

	u32		reg_id_dfr0;
	u32		reg_id_isar0;
	u32		reg_id_isar1;
	u32		reg_id_isar2;
	u32		reg_id_isar3;
	u32		reg_id_mmfr3;
	u32		reg_id_pfr0;
	u32		reg_id_pfr1;

	u32		reg_mvfr0;
	u32		reg_mvfr1;
	u32		reg_mvfr2;
};

DECLARE_PER_CPU(struct cpuinfo_arm64, cpu_data);
