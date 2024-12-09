#define KUAP_CURRENT_WRITE	8
#define KUAP_CURRENT		(KUAP_CURRENT_READ | KUAP_CURRENT_WRITE)

#ifdef CONFIG_PPC64
#include <asm/book3s/64/kup-radix.h>
#endif
#ifdef CONFIG_PPC_8xx
#include <asm/nohash/32/kup-8xx.h>
.macro kuap_check	current, gpr
.endm

#endif

#else /* !__ASSEMBLY__ */

void setup_kuap(bool disabled);
#else
static inline void setup_kuap(bool disabled) { }
static inline void allow_user_access(void __user *to, const void __user *from,
				     unsigned long size, unsigned long dir) { }
static inline void prevent_user_access(void __user *to, const void __user *from,
				       unsigned long size, unsigned long dir) { }
static inline unsigned long prevent_user_access_return(void) { return 0UL; }
static inline void restore_user_access(unsigned long flags) { }
static inline bool
bad_kuap_fault(struct pt_regs *regs, unsigned long address, bool is_write)
{
	return false;
}
#endif /* CONFIG_PPC_KUAP */

static inline void allow_read_from_user(const void __user *from, unsigned long size)
{