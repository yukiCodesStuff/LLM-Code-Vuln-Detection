	TP_ARGS(ip, parent_ip),

	TP_STRUCT__entry(
		__field(u32, caller_offs)
		__field(u32, parent_offs)
	),

	TP_fast_assign(
		__entry->caller_offs = (u32)(ip - (unsigned long)_stext);
		__entry->parent_offs = (u32)(parent_ip - (unsigned long)_stext);
	),

	TP_printk("caller=%pS parent=%pS",
		  (void *)((unsigned long)(_stext) + __entry->caller_offs),