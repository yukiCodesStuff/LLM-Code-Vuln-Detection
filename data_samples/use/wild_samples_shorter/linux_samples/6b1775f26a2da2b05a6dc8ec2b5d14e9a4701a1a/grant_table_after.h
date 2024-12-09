 * access has been ended, free the given page too.  Access will be ended
 * immediately iff the grant entry is not in use, otherwise it will happen
 * some time later.  page may be 0, in which case no freeing will occur.
 * Note that the granted page might still be accessed (read or write) by the
 * other side after gnttab_end_foreign_access() returns, so even if page was
 * specified as 0 it is not allowed to just reuse the page for other
 * purposes immediately.
 */
void gnttab_end_foreign_access(grant_ref_t ref, int readonly,
			       unsigned long page);

/*
 * End access through the given grant reference, iff the grant entry is
 * no longer in use.  In case of success ending foreign access, the
 * grant reference is deallocated.
 * Return 1 if the grant entry was freed, 0 if it is still in use.
 */
int gnttab_try_end_foreign_access(grant_ref_t ref);

int gnttab_grant_foreign_transfer(domid_t domid, unsigned long pfn);

unsigned long gnttab_end_foreign_transfer_ref(grant_ref_t ref);
unsigned long gnttab_end_foreign_transfer(grant_ref_t ref);