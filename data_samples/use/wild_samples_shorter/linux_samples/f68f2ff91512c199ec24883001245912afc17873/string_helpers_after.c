EXPORT_SYMBOL(memcpy_and_pad);

#ifdef CONFIG_FORTIFY_SOURCE
/* These are placeholders for fortify compile-time warnings. */
void __read_overflow2_field(size_t avail, size_t wanted) { }
EXPORT_SYMBOL(__read_overflow2_field);
void __write_overflow_field(size_t avail, size_t wanted) { }
EXPORT_SYMBOL(__write_overflow_field);

void fortify_panic(const char *name)
{
	pr_emerg("detected buffer overflow in %s\n", name);
	BUG();