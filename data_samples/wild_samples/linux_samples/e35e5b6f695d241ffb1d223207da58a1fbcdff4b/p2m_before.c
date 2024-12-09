	while (*link) {
		parent = *link;
		entry = rb_entry(parent, struct xen_p2m_entry, rbnode_phys);

		if (new->pfn == entry->pfn)
			goto err_out;

		if (new->pfn < entry->pfn)
			link = &(*link)->rb_left;
		else
			link = &(*link)->rb_right;
	}
	rb_link_node(&new->rbnode_phys, parent, link);
	rb_insert_color(&new->rbnode_phys, &phys_to_mach);
	goto out;

err_out:
	rc = -EINVAL;
	pr_warn("%s: cannot add pfn=%pa -> mfn=%pa: pfn=%pa -> mfn=%pa already exists\n",
			__func__, &new->pfn, &new->mfn, &entry->pfn, &entry->mfn);
out:
	return rc;
}

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
			unsigned long mfn = entry->mfn + (pfn - entry->pfn);
			read_unlock_irqrestore(&p2m_lock, irqflags);
			return mfn;
		}
		if (pfn < entry->pfn)
			n = n->rb_left;
		else
			n = n->rb_right;
	}
	read_unlock_irqrestore(&p2m_lock, irqflags);

	return INVALID_P2M_ENTRY;
}
{
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
				rb_erase(&p2m_entry->rbnode_phys, &phys_to_mach);
				write_unlock_irqrestore(&p2m_lock, irqflags);
				kfree(p2m_entry);
				return true;
			}
			if (pfn < p2m_entry->pfn)
				n = n->rb_left;
			else
				n = n->rb_right;
		}
		write_unlock_irqrestore(&p2m_lock, irqflags);
		return true;
	}