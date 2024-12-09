#define CORE_DUMP_USE_REGSET
#define ELF_EXEC_PAGESIZE	4096

/*
 * This is the base location for PIE (ET_DYN with INTERP) loads. On
 * 64-bit, this is raised to 4GB to leave the entire 32-bit address
 * space open for things that want to use the area for 32-bit pointers.
 */
#define ELF_ET_DYN_BASE		(mmap_is_ia32() ? 0x000400000UL : \
						  0x100000000UL)

/* This yields a mask that user programs can use to figure out what
   instruction set this CPU supports.  This could be done in user space,
   but it's not easy, and we've already done it here.  */