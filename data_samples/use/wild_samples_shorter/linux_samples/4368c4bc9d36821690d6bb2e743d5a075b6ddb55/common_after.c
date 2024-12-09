#define NO_L1TF		BIT(3)
#define NO_MDS		BIT(4)
#define MSBDS_ONLY	BIT(5)
#define NO_SWAPGS	BIT(6)

#define VULNWL(_vendor, _family, _model, _whitelist)	\
	{ X86_VENDOR_##_vendor, _family, _model, X86_FEATURE_ANY, _whitelist }

	VULNWL_INTEL(ATOM_BONNELL,		NO_SPECULATION),
	VULNWL_INTEL(ATOM_BONNELL_MID,		NO_SPECULATION),

	VULNWL_INTEL(ATOM_SILVERMONT,		NO_SSB | NO_L1TF | MSBDS_ONLY | NO_SWAPGS),
	VULNWL_INTEL(ATOM_SILVERMONT_X,		NO_SSB | NO_L1TF | MSBDS_ONLY | NO_SWAPGS),
	VULNWL_INTEL(ATOM_SILVERMONT_MID,	NO_SSB | NO_L1TF | MSBDS_ONLY | NO_SWAPGS),
	VULNWL_INTEL(ATOM_AIRMONT,		NO_SSB | NO_L1TF | MSBDS_ONLY | NO_SWAPGS),
	VULNWL_INTEL(XEON_PHI_KNL,		NO_SSB | NO_L1TF | MSBDS_ONLY | NO_SWAPGS),
	VULNWL_INTEL(XEON_PHI_KNM,		NO_SSB | NO_L1TF | MSBDS_ONLY | NO_SWAPGS),

	VULNWL_INTEL(CORE_YONAH,		NO_SSB),

	VULNWL_INTEL(ATOM_AIRMONT_MID,		NO_L1TF | MSBDS_ONLY | NO_SWAPGS),

	VULNWL_INTEL(ATOM_GOLDMONT,		NO_MDS | NO_L1TF | NO_SWAPGS),
	VULNWL_INTEL(ATOM_GOLDMONT_X,		NO_MDS | NO_L1TF | NO_SWAPGS),
	VULNWL_INTEL(ATOM_GOLDMONT_PLUS,	NO_MDS | NO_L1TF | NO_SWAPGS),

	/*
	 * Technically, swapgs isn't serializing on AMD (despite it previously
	 * being documented as such in the APM).  But according to AMD, %gs is
	 * updated non-speculatively, and the issuing of %gs-relative memory
	 * operands will be blocked until the %gs update completes, which is
	 * good enough for our purposes.
	 */

	/* AMD Family 0xf - 0x12 */
	VULNWL_AMD(0x0f,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS | NO_SWAPGS),
	VULNWL_AMD(0x10,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS | NO_SWAPGS),
	VULNWL_AMD(0x11,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS | NO_SWAPGS),
	VULNWL_AMD(0x12,	NO_MELTDOWN | NO_SSB | NO_L1TF | NO_MDS | NO_SWAPGS),

	/* FAMILY_ANY must be last, otherwise 0x0f - 0x12 matches won't work */
	VULNWL_AMD(X86_FAMILY_ANY,	NO_MELTDOWN | NO_L1TF | NO_MDS | NO_SWAPGS),
	VULNWL_HYGON(X86_FAMILY_ANY,	NO_MELTDOWN | NO_L1TF | NO_MDS | NO_SWAPGS),
	{}
};

static bool __init cpu_matches(unsigned long which)
			setup_force_cpu_bug(X86_BUG_MSBDS_ONLY);
	}

	if (!cpu_matches(NO_SWAPGS))
		setup_force_cpu_bug(X86_BUG_SWAPGS);

	if (cpu_matches(NO_MELTDOWN))
		return;

	/* Rogue Data Cache Load? No! */