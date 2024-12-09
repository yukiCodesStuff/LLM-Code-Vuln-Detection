#define X86_FEATURE_CQM_OCCUP_LLC	(11*32+ 1) /* LLC occupancy monitoring */
#define X86_FEATURE_CQM_MBM_TOTAL	(11*32+ 2) /* LLC Total MBM monitoring */
#define X86_FEATURE_CQM_MBM_LOCAL	(11*32+ 3) /* LLC Local MBM monitoring */
#define X86_FEATURE_FENCE_SWAPGS_USER	(11*32+ 4) /* "" LFENCE in user entry SWAPGS path */
#define X86_FEATURE_FENCE_SWAPGS_KERNEL	(11*32+ 5) /* "" LFENCE in kernel entry SWAPGS path */

/* Intel-defined CPU features, CPUID level 0x00000007:1 (EAX), word 12 */
#define X86_FEATURE_AVX512_BF16		(12*32+ 5) /* AVX512 BFLOAT16 instructions */

#define X86_BUG_L1TF			X86_BUG(18) /* CPU is affected by L1 Terminal Fault */
#define X86_BUG_MDS			X86_BUG(19) /* CPU is affected by Microarchitectural data sampling */
#define X86_BUG_MSBDS_ONLY		X86_BUG(20) /* CPU is only affected by the  MSDBS variant of BUG_MDS */
#define X86_BUG_SWAPGS			X86_BUG(21) /* CPU is affected by speculation through SWAPGS */

#endif /* _ASM_X86_CPUFEATURES_H */