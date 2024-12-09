
unsigned long __pfn_to_mfn(unsigned long pfn)
{
	struct rb_node *n = phys_to_mach.rb_node;
	struct xen_p2m_entry *entry;
	unsigned long irqflags;

	read_lock_irqsave(&p2m_lock, irqflags);
	while (n) {
		entry = rb_entry(n, struct xen_p2m_entry, rbnode_phys);
		if (entry->pfn <= pfn &&
				entry->pfn + entry->nr_pages > pfn) {
	int rc;
	unsigned long irqflags;
	struct xen_p2m_entry *p2m_entry;
	struct rb_node *n = phys_to_mach.rb_node;

	if (mfn == INVALID_P2M_ENTRY) {
		write_lock_irqsave(&p2m_lock, irqflags);
		while (n) {
			p2m_entry = rb_entry(n, struct xen_p2m_entry, rbnode_phys);
			if (p2m_entry->pfn <= pfn &&
					p2m_entry->pfn + p2m_entry->nr_pages > pfn) {