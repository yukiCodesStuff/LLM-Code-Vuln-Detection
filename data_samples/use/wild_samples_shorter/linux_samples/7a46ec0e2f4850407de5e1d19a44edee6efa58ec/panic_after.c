#include <linux/nmi.h>
#include <linux/console.h>
#include <linux/bug.h>
#include <linux/ratelimit.h>

#define PANIC_TIMER_STEP 100
#define PANIC_BLINK_SPD 18


#endif

#ifdef CONFIG_ARCH_HAS_REFCOUNT
void refcount_error_report(struct pt_regs *regs, const char *err)
{
	WARN_RATELIMIT(1, "refcount_t %s at %pB in %s[%d], uid/euid: %u/%u\n",
		err, (void *)instruction_pointer(regs),
		current->comm, task_pid_nr(current),
		from_kuid_munged(&init_user_ns, current_uid()),
		from_kuid_munged(&init_user_ns, current_euid()));
}
#endif

core_param(panic, panic_timeout, int, 0644);
core_param(pause_on_oops, pause_on_oops, int, 0644);
core_param(panic_on_warn, panic_on_warn, int, 0644);
core_param(crash_kexec_post_notifiers, crash_kexec_post_notifiers, bool, 0644);