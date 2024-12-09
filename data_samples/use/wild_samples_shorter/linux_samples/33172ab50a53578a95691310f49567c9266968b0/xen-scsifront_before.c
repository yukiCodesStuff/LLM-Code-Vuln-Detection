		return;

	for (i = 0; i < shadow->nr_grants; i++) {
		if (unlikely(gnttab_query_foreign_access(shadow->gref[i]))) {
			shost_printk(KERN_ALERT, info->host, KBUILD_MODNAME
				     "grant still in use by backend\n");
			BUG();
		}
		gnttab_end_foreign_access(shadow->gref[i], 0, 0UL);
	}

	kfree(shadow->sg);
}