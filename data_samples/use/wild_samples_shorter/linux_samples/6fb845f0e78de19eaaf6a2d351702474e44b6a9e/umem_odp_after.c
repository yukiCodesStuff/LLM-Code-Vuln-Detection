	umem->writable   = 1;
	umem->is_odp = 1;
	odp_data->per_mm = per_mm;
	umem->owning_mm  = per_mm->mm;
	mmgrab(umem->owning_mm);

	mutex_init(&odp_data->umem_mutex);
	init_completion(&odp_data->notifier_completion);

out_page_list:
	vfree(odp_data->page_list);
out_odp_data:
	mmdrop(umem->owning_mm);
	kfree(odp_data);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL(ib_alloc_odp_umem);