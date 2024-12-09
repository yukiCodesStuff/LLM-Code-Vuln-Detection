
#endif /*!CONFIG_PA20*/

/* LDCW, the only atomic read-write operation PA-RISC has. *sigh*.
   We don't explicitly expose that "*a" may be written as reload
   fails to find a register in class R1_REGS when "a" needs to be
   reloaded when generating 64-bit PIC code.  Instead, we clobber
   memory to indicate to the compiler that the assembly code reads
   or writes to items other than those listed in the input and output
   operands.  This may pessimize the code somewhat but __ldcw is
   usually used within code blocks surrounded by memory barriors.  */
#define __ldcw(a) ({						\
	unsigned __ret;						\
	__asm__ __volatile__(__LDCW " 0(%1),%0"			\
		: "=r" (__ret) : "r" (a) : "memory");		\
	__ret;							\
})

#ifdef CONFIG_SMP