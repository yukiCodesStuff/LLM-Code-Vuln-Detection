{
	struct desc_struct *gdt = get_cpu_gdt_table(cpu);
	unsigned int i;

	for (i = 0; i < GDT_ENTRY_TLS_ENTRIES; i++)
		gdt[GDT_ENTRY_TLS_MIN + i] = t->tls_array[i];
}

#define _LDT_empty(info)				\
	((info)->base_addr		== 0	&&	\
	 (info)->limit			== 0	&&	\
	 (info)->contents		== 0	&&	\
	 (info)->read_exec_only		== 1	&&	\
	 (info)->seg_32bit		== 0	&&	\
	 (info)->limit_in_pages		== 0	&&	\
	 (info)->seg_not_present	== 1	&&	\
	 (info)->useable		== 0)

#ifdef CONFIG_X86_64
#define LDT_empty(info) (_LDT_empty(info) && ((info)->lm == 0))
#else
#define LDT_empty(info) (_LDT_empty(info))
#endif

static inline void clear_LDT(void)
{
	set_ldt(NULL, 0);
}

/*
 * load one particular LDT into the current CPU
 */
static inline void load_LDT_nolock(mm_context_t *pc)
{
	set_ldt(pc->ldt, pc->size);
}

static inline void load_LDT(mm_context_t *pc)
{
	preempt_disable();
	load_LDT_nolock(pc);
	preempt_enable();
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
#define set_intr_gate(n, addr)						\
	do {								\
		BUG_ON((unsigned)n > 0xFF);				\
		_set_gate(n, GATE_INTERRUPT, (void *)addr, 0, 0,	\
			  __KERNEL_CS);					\
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
{
	struct desc_struct *gdt = get_cpu_gdt_table(cpu);
	unsigned int i;

	for (i = 0; i < GDT_ENTRY_TLS_ENTRIES; i++)
		gdt[GDT_ENTRY_TLS_MIN + i] = t->tls_array[i];
}

#define _LDT_empty(info)				\
	((info)->base_addr		== 0	&&	\
	 (info)->limit			== 0	&&	\
	 (info)->contents		== 0	&&	\
	 (info)->read_exec_only		== 1	&&	\
	 (info)->seg_32bit		== 0	&&	\
	 (info)->limit_in_pages		== 0	&&	\
	 (info)->seg_not_present	== 1	&&	\
	 (info)->useable		== 0)

#ifdef CONFIG_X86_64
#define LDT_empty(info) (_LDT_empty(info) && ((info)->lm == 0))
#else
#define LDT_empty(info) (_LDT_empty(info))
#endif

static inline void clear_LDT(void)
{
	set_ldt(NULL, 0);
}