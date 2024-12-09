	if (atomic_dec_and_test(&file->device->refcount))
		ib_uverbs_comp_dev(file->device);

	put_device(&file->device->dev);
	kfree(file);
}


		/* Get an arbitrary mm pointer that hasn't been cleaned yet */
		mutex_lock(&ufile->umap_lock);
		if (!list_empty(&ufile->umaps)) {
			mm = list_first_entry(&ufile->umaps,
					      struct rdma_umap_priv, list)
				     ->vma->vm_mm;
			mmget(mm);
		}
		mutex_unlock(&ufile->umap_lock);
		if (!mm)
			return;
	list_del_init(&file->list);
	mutex_unlock(&file->device->lists_mutex);

	if (file->async_file)
		kref_put(&file->async_file->ref,
			 ib_uverbs_release_async_event_file);

	kref_put(&file->ref, ib_uverbs_release_file);

	return 0;
}