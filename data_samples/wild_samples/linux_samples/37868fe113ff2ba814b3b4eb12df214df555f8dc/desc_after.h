{
	set_ldt(NULL, 0);
}

static inline unsigned long get_desc_base(const struct desc_struct *desc)
{
	return (unsigned)(desc->base0 | ((desc->base1) << 16) | ((desc->base2) << 24));
}

static inline void set_desc_base(struct desc_struct *desc, unsigned long base)
{
	desc->base0 = base & 0xffff;
	desc->base1 = (base >> 16) & 0xff;
	desc->base2 = (base >> 24) & 0xff;
}

static inline unsigned long get_desc_limit(const struct desc_struct *desc)
{
	return desc->limit0 | (desc->limit << 16);
}

static inline void set_desc_limit(struct desc_struct *desc, unsigned long limit)
{
	desc->limit0 = limit & 0xffff;
	desc->limit = (limit >> 16) & 0xf;
}

#ifdef CONFIG_X86_64
static inline void set_nmi_gate(int gate, void *addr)
{
	gate_desc s;

	pack_gate(&s, GATE_INTERRUPT, (unsigned long)addr, 0, 0, __KERNEL_CS);
	write_idt_entry(debug_idt_table, gate, &s);
}
#endif

#ifdef CONFIG_TRACING
extern struct desc_ptr trace_idt_descr;
extern gate_desc trace_idt_table[];
static inline void write_trace_idt_entry(int entry, const gate_desc *gate)
{
	write_idt_entry(trace_idt_table, entry, gate);
}

static inline void _trace_set_gate(int gate, unsigned type, void *addr,
				   unsigned dpl, unsigned ist, unsigned seg)
{
	gate_desc s;

	pack_gate(&s, type, (unsigned long)addr, dpl, ist, seg);
	/*
	 * does not need to be atomic because it is only done once at
	 * setup time
	 */
	write_trace_idt_entry(gate, &s);
}
#else
static inline void write_trace_idt_entry(int entry, const gate_desc *gate)
{
}

#define _trace_set_gate(gate, type, addr, dpl, ist, seg)
#endif

static inline void _set_gate(int gate, unsigned type, void *addr,
			     unsigned dpl, unsigned ist, unsigned seg)
{
	gate_desc s;

	pack_gate(&s, type, (unsigned long)addr, dpl, ist, seg);
	/*
	 * does not need to be atomic because it is only done once at
	 * setup time
	 */
	write_idt_entry(idt_table, gate, &s);
	write_trace_idt_entry(gate, &s);
}

/*
 * This needs to use 'idt_table' rather than 'idt', and
 * thus use the _nonmapped_ version of the IDT, as the
 * Pentium F0 0F bugfix can have resulted in the mapped
 * IDT being write-protected.
 */
#define set_intr_gate_notrace(n, addr)					\
	do {								\
		BUG_ON((unsigned)n > 0xFF);				\
		_set_gate(n, GATE_INTERRUPT, (void *)addr, 0, 0,	\
			  __KERNEL_CS);					\
	} while (0)

#define set_intr_gate(n, addr)						\
	do {								\
		set_intr_gate_notrace(n, addr);				\
		_trace_set_gate(n, GATE_INTERRUPT, (void *)trace_##addr,\
				0, 0, __KERNEL_CS);			\
	} while (0)

extern int first_system_vector;
/* used_vectors is BITMAP for irq is not managed by percpu vector_irq */
extern unsigned long used_vectors[];

static inline void alloc_system_vector(int vector)
{
	if (!test_bit(vector, used_vectors)) {
		set_bit(vector, used_vectors);
		if (first_system_vector > vector)
			first_system_vector = vector;
	} else {
		BUG();
	}
}