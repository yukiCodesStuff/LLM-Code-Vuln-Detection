#include <asm/fpu/internal.h>
#include <asm/debugreg.h>
#include <asm/cpu.h>

#ifdef CONFIG_X86_32
__visible unsigned long saved_context_ebx;
__visible unsigned long saved_context_esp, saved_context_ebp;
	syscall_init();				/* This sets MSR_*STAR and related */
#endif
	load_TR_desc();				/* This does ltr */
	load_LDT(&current->active_mm->context);	/* This does lldt */

	fpu__resume_cpu();
}
