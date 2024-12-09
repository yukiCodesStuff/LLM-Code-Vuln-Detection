
typedef struct {
#ifdef CONFIG_CPU_HAS_ASID
	u64 id;
#endif
	unsigned int vmalloc_seq;
} mm_context_t;

#ifdef CONFIG_CPU_HAS_ASID
#define ASID_BITS	8
#define ASID_MASK	((~0ULL) << ASID_BITS)
#define ASID(mm)	((mm)->context.id & ~ASID_MASK)
#else
#define ASID(mm)	(0)
#endif

 *  modified for 2.6 by Hyok S. Choi <hyok.choi@samsung.com>
 */
typedef struct {
	unsigned long		end_brk;
} mm_context_t;

#endif
