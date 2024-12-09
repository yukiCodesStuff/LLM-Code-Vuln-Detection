 * Note that the granted page might still be accessed (read or write) by the
 * other side after gnttab_end_foreign_access() returns, so even if page was
 * specified as 0 it is not allowed to just reuse the page for other
 * purposes immediately. gnttab_end_foreign_access() will take an additional
 * reference to the granted page in this case, which is dropped only after
 * the grant is no longer in use.
 * This requires that multi page allocations for areas subject to
 * gnttab_end_foreign_access() are done via alloc_pages_exact() (and freeing
 * via free_pages_exact()) in order to avoid high order pages.
 */
void gnttab_end_foreign_access(grant_ref_t ref, int readonly,
			       unsigned long page);
