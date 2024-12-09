	local_flush_tlb_kernel_range(ta->ta_start, ta->ta_end);
}

static inline void ipi_flush_bp_all(void *ignored)
{
	local_flush_bp_all();
}

void flush_tlb_all(void)
{
	if (tlb_ops_need_broadcast())
		on_each_cpu(ipi_flush_tlb_all, NULL, 1);
		local_flush_tlb_kernel_range(start, end);
}

void flush_bp_all(void)
{
	if (tlb_ops_need_broadcast())
		on_each_cpu(ipi_flush_bp_all, NULL, 1);
	else
		local_flush_bp_all();
}