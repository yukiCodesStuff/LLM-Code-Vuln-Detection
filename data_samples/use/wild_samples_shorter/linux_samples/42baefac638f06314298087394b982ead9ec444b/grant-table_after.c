	 * return the frame.
	 */
	unsigned long (*end_foreign_transfer_ref)(grant_ref_t ref);
	/*
	 * Read the frame number related to a given grant reference.
	 */
	unsigned long (*read_frame)(grant_ref_t ref);
};

struct unmap_refs_callback_data {
	struct completion completion;
}
EXPORT_SYMBOL_GPL(gnttab_end_foreign_access_ref);

static unsigned long gnttab_read_frame_v1(grant_ref_t ref)
{
	return gnttab_shared.v1[ref].frame;
}

static unsigned long gnttab_read_frame_v2(grant_ref_t ref)
{
	return gnttab_shared.v2[ref].full_page.frame;
}

struct deferred_entry {
	struct list_head list;
	grant_ref_t ref;
	bool ro;
		spin_unlock_irqrestore(&gnttab_list_lock, flags);
		if (_gnttab_end_foreign_access_ref(entry->ref, entry->ro)) {
			put_free_entry(entry->ref);
			pr_debug("freeing g.e. %#x (pfn %#lx)\n",
				 entry->ref, page_to_pfn(entry->page));
			put_page(entry->page);
			kfree(entry);
			entry = NULL;
		} else {
			if (!--entry->warn_delay)
static void gnttab_add_deferred(grant_ref_t ref, bool readonly,
				struct page *page)
{
	struct deferred_entry *entry;
	gfp_t gfp = (in_atomic() || irqs_disabled()) ? GFP_ATOMIC : GFP_KERNEL;
	const char *what = KERN_WARNING "leaking";

	entry = kmalloc(sizeof(*entry), gfp);
	if (!page) {
		unsigned long gfn = gnttab_interface->read_frame(ref);

		page = pfn_to_page(gfn_to_pfn(gfn));
		get_page(page);
	}

	if (entry) {
		unsigned long flags;

		entry->ref = ref;
	.update_entry			= gnttab_update_entry_v1,
	.end_foreign_access_ref		= gnttab_end_foreign_access_ref_v1,
	.end_foreign_transfer_ref	= gnttab_end_foreign_transfer_ref_v1,
	.read_frame			= gnttab_read_frame_v1,
};

static const struct gnttab_ops gnttab_v2_ops = {
	.version			= 2,
	.update_entry			= gnttab_update_entry_v2,
	.end_foreign_access_ref		= gnttab_end_foreign_access_ref_v2,
	.end_foreign_transfer_ref	= gnttab_end_foreign_transfer_ref_v2,
	.read_frame			= gnttab_read_frame_v2,
};

static bool gnttab_need_v2(void)
{