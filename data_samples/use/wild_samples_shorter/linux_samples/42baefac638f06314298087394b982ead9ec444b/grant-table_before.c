	 * return the frame.
	 */
	unsigned long (*end_foreign_transfer_ref)(grant_ref_t ref);
};

struct unmap_refs_callback_data {
	struct completion completion;
}
EXPORT_SYMBOL_GPL(gnttab_end_foreign_access_ref);

struct deferred_entry {
	struct list_head list;
	grant_ref_t ref;
	bool ro;
		spin_unlock_irqrestore(&gnttab_list_lock, flags);
		if (_gnttab_end_foreign_access_ref(entry->ref, entry->ro)) {
			put_free_entry(entry->ref);
			if (entry->page) {
				pr_debug("freeing g.e. %#x (pfn %#lx)\n",
					 entry->ref, page_to_pfn(entry->page));
				put_page(entry->page);
			} else
				pr_info("freeing g.e. %#x\n", entry->ref);
			kfree(entry);
			entry = NULL;
		} else {
			if (!--entry->warn_delay)
static void gnttab_add_deferred(grant_ref_t ref, bool readonly,
				struct page *page)
{
	struct deferred_entry *entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
	const char *what = KERN_WARNING "leaking";

	if (entry) {
		unsigned long flags;

		entry->ref = ref;
	.update_entry			= gnttab_update_entry_v1,
	.end_foreign_access_ref		= gnttab_end_foreign_access_ref_v1,
	.end_foreign_transfer_ref	= gnttab_end_foreign_transfer_ref_v1,
};

static const struct gnttab_ops gnttab_v2_ops = {
	.version			= 2,
	.update_entry			= gnttab_update_entry_v2,
	.end_foreign_access_ref		= gnttab_end_foreign_access_ref_v2,
	.end_foreign_transfer_ref	= gnttab_end_foreign_transfer_ref_v2,
};

static bool gnttab_need_v2(void)
{