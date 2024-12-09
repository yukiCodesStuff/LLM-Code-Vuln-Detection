#define H_TLB_INVALIDATE	0xF808
#define H_COPY_TOFROM_GUEST	0xF80C

/* Flags for H_SVM_PAGE_IN */
#define H_PAGE_IN_SHARED        0x1

/* Platform-specific hcalls used by the Ultravisor */
#define H_SVM_PAGE_IN		0xEF00
#define H_SVM_PAGE_OUT		0xEF04
#define H_SVM_INIT_START	0xEF08
#define H_SVM_INIT_DONE		0xEF0C

/* Values for 2nd argument to H_SET_MODE */
#define H_SET_MODE_RESOURCE_SET_CIABR		1
#define H_SET_MODE_RESOURCE_SET_DAWR		2
#define H_SET_MODE_RESOURCE_ADDR_TRANS_MODE	3