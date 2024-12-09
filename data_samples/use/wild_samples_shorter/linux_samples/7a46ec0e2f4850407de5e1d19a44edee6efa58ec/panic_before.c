#include <linux/nmi.h>
#include <linux/console.h>
#include <linux/bug.h>

#define PANIC_TIMER_STEP 100
#define PANIC_BLINK_SPD 18


#endif

core_param(panic, panic_timeout, int, 0644);
core_param(pause_on_oops, pause_on_oops, int, 0644);
core_param(panic_on_warn, panic_on_warn, int, 0644);
core_param(crash_kexec_post_notifiers, crash_kexec_post_notifiers, bool, 0644);