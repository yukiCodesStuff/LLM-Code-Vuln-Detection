	if (atomic_dec_and_test(&file->device->refcount))
		ib_uverbs_comp_dev(file->device);

	if (file->async_file)
		kref_put(&file->async_file->ref,
			 ib_uverbs_release_async_event_file);
	put_device(&file->device->dev);
	kfree(file);
}


		/* Get an arbitrary mm pointer that hasn't been cleaned yet */
		mutex_lock(&ufile->umap_lock);
		while (!list_empty(&ufile->umaps)) {
			int ret;

			priv = list_first_entry(&ufile->umaps,
						struct rdma_umap_priv, list);
			mm = priv->vma->vm_mm;
			ret = mmget_not_zero(mm);
			if (!ret) {
				list_del_init(&priv->list);
				mm = NULL;
				continue;
			}
			break;
		}
		mutex_unlock(&ufile->umap_lock);
		if (!mm)
			return;
	list_del_init(&file->list);
	mutex_unlock(&file->device->lists_mutex);

	kref_put(&file->ref, ib_uverbs_release_file);

	return 0;
}