		/* These depend on page entry type, so compute them now.  */
		__field(bool, r)
		__field(bool, x)
		__field(u8, u)
	),

	TP_fast_assign(
		__entry->gfn = gfn;