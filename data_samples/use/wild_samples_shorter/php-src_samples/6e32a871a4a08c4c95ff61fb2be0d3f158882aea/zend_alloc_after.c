	size_t res = nmemb;
	unsigned long overflow = 0;

	__asm__ ("mull %3\n\taddl %4,%0\n\tadcl $0,%1"
	     : "=&a"(res), "=&d" (overflow)
	     : "%0"(res),
	       "rm"(size),
	       "rm"(offset));
        size_t res = nmemb;
        unsigned long overflow = 0;

        __asm__ ("mulq %3\n\taddq %4,%0\n\tadcq $0,%1"
             : "=&a"(res), "=&d" (overflow)
             : "%0"(res),
               "rm"(size),
               "rm"(offset));