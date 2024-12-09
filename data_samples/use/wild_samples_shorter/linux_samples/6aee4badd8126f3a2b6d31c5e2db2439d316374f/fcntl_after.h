#define _UAPI_LINUX_FCNTL_H

#include <asm/fcntl.h>
#include <linux/openat2.h>

#define F_SETLEASE	(F_LINUX_SPECIFIC_BASE + 0)
#define F_GETLEASE	(F_LINUX_SPECIFIC_BASE + 1)


#define AT_RECURSIVE		0x8000	/* Apply to the entire subtree */

#endif /* _UAPI_LINUX_FCNTL_H */