// Software required to flush count cache on context switch
#define SEC_FTR_FLUSH_COUNT_CACHE	0x0000000000000400ull

// Software required to flush link stack on context switch
#define SEC_FTR_FLUSH_LINK_STACK	0x0000000000001000ull


// Features enabled by default
#define SEC_FTR_DEFAULT \
	(SEC_FTR_L1D_FLUSH_HV | \