#endif

#ifdef CONFIG_PPC_STD_MMU_64
#if TASK_SIZE_USER64 > (1UL << (ESID_BITS + SID_SHIFT))
#error TASK_SIZE_USER64 exceeds user VSID range
#endif
#endif
