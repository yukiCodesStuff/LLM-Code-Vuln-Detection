#include <linux/slab.h>
#include <linux/security.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

/* init to 2 - one for init_task, one to ensure it is never freed */
struct group_info init_groups = { .usage = ATOMIC_INIT(2) };
{
	struct user_namespace *user_ns = current_user_ns();

	return ns_capable(user_ns, CAP_SETGID);
}

/*
 *	SMP: Our groups are copy-on-write. We can set them safely