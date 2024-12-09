
static bool tls_desc_okay(const struct user_desc *info)
{
	if (LDT_empty(info))
		return true;

	/*
	 * espfix is required for 16-bit data segments, but espfix
	cpu = get_cpu();

	while (n-- > 0) {
		if (LDT_empty(info))
			desc->a = desc->b = 0;
		else
			fill_ldt(desc, info);
		++info;