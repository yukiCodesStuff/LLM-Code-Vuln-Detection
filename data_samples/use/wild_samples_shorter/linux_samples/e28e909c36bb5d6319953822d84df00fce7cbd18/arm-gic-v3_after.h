#define ICH_LR_ACTIVE_BIT		(1ULL << 63)
#define ICH_LR_PHYS_ID_SHIFT		32
#define ICH_LR_PHYS_ID_MASK		(0x3ffULL << ICH_LR_PHYS_ID_SHIFT)
#define ICH_LR_PRIORITY_SHIFT		48

/* These are for GICv2 emulation only */
#define GICH_LR_VIRTUALID		(0x3ffUL << 0)
#define GICH_LR_PHYSID_CPUID_SHIFT	(10)
#define GICH_LR_PHYSID_CPUID		(7UL << GICH_LR_PHYSID_CPUID_SHIFT)

#define ICH_MISR_EOI			(1 << 0)
#define ICH_MISR_U			(1 << 1)
