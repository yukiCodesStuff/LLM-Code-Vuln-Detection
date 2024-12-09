void fortify_panic(const char *name) __noreturn __cold;
void __read_overflow(void) __compiletime_error("detected read beyond size of object (1st parameter)");
void __read_overflow2(void) __compiletime_error("detected read beyond size of object (2nd parameter)");
void __read_overflow2_field(size_t avail, size_t wanted) __compiletime_warning("detected read beyond size of field (2nd parameter); maybe use struct_group()?");
void __write_overflow(void) __compiletime_error("detected write beyond size of object (1st parameter)");
void __write_overflow_field(size_t avail, size_t wanted) __compiletime_warning("detected write beyond size of field (1st parameter); maybe use struct_group()?");

#define __compiletime_strlen(p)					\
({								\
	unsigned char *__p = (unsigned char *)(p);		\
	return __underlying_memset(p, c, size);
}

/*
 * To make sure the compiler can enforce protection against buffer overflows,
 * memcpy(), memmove(), and memset() must not be used beyond individual
 * struct members. If you need to copy across multiple members, please use
 * struct_group() to create a named mirror of an anonymous struct union.
 * (e.g. see struct sk_buff.) Read overflow checking is currently only
 * done when a write overflow is also present, or when building with W=1.
 *
 * Mitigation coverage matrix
 *					Bounds checking at:
 *					+-------+-------+-------+-------+
 *					| Compile time  |   Run time    |
 * memcpy() argument sizes:		| write | read  | write | read  |
 *        dest     source   length      +-------+-------+-------+-------+
 * memcpy(known,   known,   constant)	|   y   |   y   |  n/a  |  n/a  |
 * memcpy(known,   unknown, constant)	|   y   |   n   |  n/a  |   V   |
 * memcpy(known,   known,   dynamic)	|   n   |   n   |   B   |   B   |
 * memcpy(known,   unknown, dynamic)	|   n   |   n   |   B   |   V   |
 * memcpy(unknown, known,   constant)	|   n   |   y   |   V   |  n/a  |
 * memcpy(unknown, unknown, constant)	|   n   |   n   |   V   |   V   |
 * memcpy(unknown, known,   dynamic)	|   n   |   n   |   V   |   B   |
 * memcpy(unknown, unknown, dynamic)	|   n   |   n   |   V   |   V   |
 *					+-------+-------+-------+-------+
 *
 * y = perform deterministic compile-time bounds checking
 * n = cannot perform deterministic compile-time bounds checking
 * n/a = no run-time bounds checking needed since compile-time deterministic
 * B = can perform run-time bounds checking (currently unimplemented)
 * V = vulnerable to run-time overflow (will need refactoring to solve)
 *
 */
__FORTIFY_INLINE void fortify_memcpy_chk(__kernel_size_t size,
					 const size_t p_size,
					 const size_t q_size,
					 const size_t p_size_field,
					 const size_t q_size_field,
					 const char *func)
{
	if (__builtin_constant_p(size)) {
		/*
		 * Length argument is a constant expression, so we
		 * can perform compile-time bounds checking where
		 * buffer sizes are known.
		 */

		/* Error when size is larger than enclosing struct. */
		if (p_size > p_size_field && p_size < size)
			__write_overflow();
		if (q_size > q_size_field && q_size < size)
			__read_overflow2();

		/* Warn when write size argument larger than dest field. */
		if (p_size_field < size)
			__write_overflow_field(p_size_field, size);
		/*
		 * Warn for source field over-read when building with W=1
		 * or when an over-write happened, so both can be fixed at
		 * the same time.
		 */
		if ((IS_ENABLED(KBUILD_EXTRA_WARN1) || p_size_field < size) &&
		    q_size_field < size)
			__read_overflow2_field(q_size_field, size);
	}
	/*
	 * At this point, length argument may not be a constant expression,
	 * so run-time bounds checking can be done where buffer sizes are
	 * known. (This is not an "else" because the above checks may only
	 * be compile-time warnings, and we want to still warn for run-time
	 * overflows.)
	 */

	/*
	 * Always stop accesses beyond the struct that contains the
	 * field, when the buffer's remaining size is known.
	 * (The -1 test is to optimize away checks where the buffer
	 * lengths are unknown.)
	 */
	if ((p_size != (size_t)(-1) && p_size < size) ||
	    (q_size != (size_t)(-1) && q_size < size))
		fortify_panic(func);
}

#define __fortify_memcpy_chk(p, q, size, p_size, q_size,		\
			     p_size_field, q_size_field, op) ({		\
	size_t __fortify_size = (size_t)(size);				\
	fortify_memcpy_chk(__fortify_size, p_size, q_size,		\
			   p_size_field, q_size_field, #op);		\
	__underlying_##op(p, q, __fortify_size);			\
})

/*
 * __builtin_object_size() must be captured here to avoid evaluating argument
 * side-effects further into the macro layers.
 */
#define memcpy(p, q, s)  __fortify_memcpy_chk(p, q, s,			\
		__builtin_object_size(p, 0), __builtin_object_size(q, 0), \
		__builtin_object_size(p, 1), __builtin_object_size(q, 1), \
		memcpy)

__FORTIFY_INLINE void *memmove(void *p, const void *q, __kernel_size_t size)
{
	size_t p_size = __builtin_object_size(p, 0);
	size_t q_size = __builtin_object_size(q, 0);
	return __real_kmemdup(p, size, gfp);
}

/* Defined after fortified strlen to reuse it. */
__FORTIFY_INLINE char *strcpy(char *p, const char *q)
{
	size_t p_size = __builtin_object_size(p, 1);
	size_t q_size = __builtin_object_size(q, 1);
	size_t size;

	/* If neither buffer size is known, immediately give up. */
	if (p_size == (size_t)-1 && q_size == (size_t)-1)
		return __underlying_strcpy(p, q);
	size = strlen(q) + 1;
	/* Compile-time check for const size overflow. */
	/* Run-time check for dynamic size overflow. */
	if (p_size < size)
		fortify_panic(__func__);
	__underlying_memcpy(p, q, size);
	return p;
}

/* Don't use these outside the FORITFY_SOURCE implementation */
#undef __underlying_memchr
#undef __underlying_memcmp
#undef __underlying_memmove
#undef __underlying_memset
#undef __underlying_strcat
#undef __underlying_strcpy