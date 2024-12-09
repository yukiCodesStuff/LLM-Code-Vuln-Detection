 * Note that the granted page might still be accessed (read or write) by the
 * other side after gnttab_end_foreign_access() returns, so even if page was
 * specified as 0 it is not allowed to just reuse the page for other
 * purposes immediately.
 */
void gnttab_end_foreign_access(grant_ref_t ref, int readonly,
			       unsigned long page);
