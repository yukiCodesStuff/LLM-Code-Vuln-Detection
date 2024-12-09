ENTRY(cpu_v7_switch_mm)
#ifdef CONFIG_MMU
	mmid	r1, r1				@ get mm->context.id
	and	r3, r1, #0xff
	mov	r3, r3, lsl #(48 - 32)		@ ASID
	mcrr	p15, 0, r0, r3, c2		@ set TTB 0
	isb
#endif