
#endif /*!CONFIG_PA20*/

/* LDCW, the only atomic read-write operation PA-RISC has. *sigh*.  */
#define __ldcw(a) ({						\
	unsigned __ret;						\
	__asm__ __volatile__(__LDCW " 0(%2),%0"			\
		: "=r" (__ret), "+m" (*(a)) : "r" (a));		\
	__ret;							\
})

#ifdef CONFIG_SMP