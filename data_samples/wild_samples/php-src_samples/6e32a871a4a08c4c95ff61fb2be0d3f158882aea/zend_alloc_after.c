{
	size_t res = nmemb;
	unsigned long overflow = 0;

	__asm__ ("mull %3\n\taddl %4,%0\n\tadcl $0,%1"
	     : "=&a"(res), "=&d" (overflow)
	     : "%0"(res),
	       "rm"(size),
	       "rm"(offset));
	
	if (UNEXPECTED(overflow)) {
		zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
		return 0;
	}
{
        size_t res = nmemb;
        unsigned long overflow = 0;

        __asm__ ("mulq %3\n\taddq %4,%0\n\tadcq $0,%1"
             : "=&a"(res), "=&d" (overflow)
             : "%0"(res),
               "rm"(size),
               "rm"(offset));

        if (UNEXPECTED(overflow)) {
                zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
                return 0;
        }