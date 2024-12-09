extern bool __read_mostly enable_unrestricted_guest;
extern bool __read_mostly enable_ept_ad_bits;
extern bool __read_mostly enable_pml;
extern int __read_mostly pt_mode;

#define PT_MODE_SYSTEM		0
#define PT_MODE_HOST_GUEST	1