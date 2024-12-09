static atomic_t pages_mapped = ATOMIC_INIT(0);

static int use_ptemod;
#define populate_freeable_maps use_ptemod

struct gntdev_priv {
	/* maps with visible offsets in the file descriptor */
	struct list_head maps;
	/* maps that are not visible; will be freed on munmap.
	 * Only populated if populate_freeable_maps == 1 */
	struct list_head freeable_maps;
	/* lock protects maps and freeable_maps */
	spinlock_t lock;
	struct mm_struct *mm;
	struct mmu_notifier mn;
};
	return NULL;
}

static void gntdev_put_map(struct gntdev_priv *priv, struct grant_map *map)
{
	if (!map)
		return;

		evtchn_put(map->notify.event);
	}

	if (populate_freeable_maps && priv) {
		spin_lock(&priv->lock);
		list_del(&map->next);
		spin_unlock(&priv->lock);
	}

	if (map->pages && !use_ptemod)
		unmap_grant_pages(map, 0, map->count);
	gntdev_free_map(map);
}

	if (map->notify.flags & UNMAP_NOTIFY_CLEAR_BYTE) {
		int pgno = (map->notify.addr >> PAGE_SHIFT);
		if (pgno >= offset && pgno < offset + pages) {
			/* No need for kmap, pages are in lowmem */
			uint8_t *tmp = pfn_to_kaddr(page_to_pfn(map->pages[pgno]));
			tmp[map->notify.addr & (PAGE_SIZE-1)] = 0;
			map->notify.flags &= ~UNMAP_NOTIFY_CLEAR_BYTE;
		}
	}

static void gntdev_vma_close(struct vm_area_struct *vma)
{
	struct grant_map *map = vma->vm_private_data;
	struct file *file = vma->vm_file;
	struct gntdev_priv *priv = file->private_data;

	pr_debug("gntdev_vma_close %p\n", vma);
	if (use_ptemod) {
		/* It is possible that an mmu notifier could be running
		 * concurrently, so take priv->lock to ensure that the vma won't
		 * vanishing during the unmap_grant_pages call, since we will
		 * spin here until that completes. Such a concurrent call will
		 * not do any unmapping, since that has been done prior to
		 * closing the vma, but it may still iterate the unmap_ops list.
		 */
		spin_lock(&priv->lock);
		map->vma = NULL;
		spin_unlock(&priv->lock);
	}
	vma->vm_private_data = NULL;
	gntdev_put_map(priv, map);
}

static struct vm_operations_struct gntdev_vmops = {
	.open = gntdev_vma_open,

/* ------------------------------------------------------------------ */

static void unmap_if_in_range(struct grant_map *map,
			      unsigned long start, unsigned long end)
{
	unsigned long mstart, mend;
	int err;

	if (!map->vma)
		return;
	if (map->vma->vm_start >= end)
		return;
	if (map->vma->vm_end <= start)
		return;
	mstart = max(start, map->vma->vm_start);
	mend   = min(end,   map->vma->vm_end);
	pr_debug("map %d+%d (%lx %lx), range %lx %lx, mrange %lx %lx\n",
			map->index, map->count,
			map->vma->vm_start, map->vma->vm_end,
			start, end, mstart, mend);
	err = unmap_grant_pages(map,
				(mstart - map->vma->vm_start) >> PAGE_SHIFT,
				(mend - mstart) >> PAGE_SHIFT);
	WARN_ON(err);
}

static void mn_invl_range_start(struct mmu_notifier *mn,
				struct mm_struct *mm,
				unsigned long start, unsigned long end)
{
	struct gntdev_priv *priv = container_of(mn, struct gntdev_priv, mn);
	struct grant_map *map;

	spin_lock(&priv->lock);
	list_for_each_entry(map, &priv->maps, next) {
		unmap_if_in_range(map, start, end);
	}
	list_for_each_entry(map, &priv->freeable_maps, next) {
		unmap_if_in_range(map, start, end);
	}
	spin_unlock(&priv->lock);
}

		err = unmap_grant_pages(map, /* offset */ 0, map->count);
		WARN_ON(err);
	}
	list_for_each_entry(map, &priv->freeable_maps, next) {
		if (!map->vma)
			continue;
		pr_debug("map %d+%d (%lx %lx)\n",
				map->index, map->count,
				map->vma->vm_start, map->vma->vm_end);
		err = unmap_grant_pages(map, /* offset */ 0, map->count);
		WARN_ON(err);
	}
	spin_unlock(&priv->lock);
}

static struct mmu_notifier_ops gntdev_mmu_ops = {
		return -ENOMEM;

	INIT_LIST_HEAD(&priv->maps);
	INIT_LIST_HEAD(&priv->freeable_maps);
	spin_lock_init(&priv->lock);

	if (use_ptemod) {
		priv->mm = get_task_mm(current);
	while (!list_empty(&priv->maps)) {
		map = list_entry(priv->maps.next, struct grant_map, next);
		list_del(&map->next);
		gntdev_put_map(NULL /* already removed */, map);
	}
	WARN_ON(!list_empty(&priv->freeable_maps));

	if (use_ptemod)
		mmu_notifier_unregister(&priv->mn, priv->mm);
	kfree(priv);

	if (unlikely(atomic_add_return(op.count, &pages_mapped) > limit)) {
		pr_debug("can't map: over limit\n");
		gntdev_put_map(NULL, map);
		return err;
	}

	if (copy_from_user(map->grants, &u->refs,
			   sizeof(map->grants[0]) * op.count) != 0) {
		gntdev_put_map(NULL, map);
		return -EFAULT;
	}

	spin_lock(&priv->lock);
	gntdev_add_map(priv, map);
	map = gntdev_find_map_index(priv, op.index >> PAGE_SHIFT, op.count);
	if (map) {
		list_del(&map->next);
		if (populate_freeable_maps)
			list_add_tail(&map->next, &priv->freeable_maps);
		err = 0;
	}
	spin_unlock(&priv->lock);
	if (map)
		gntdev_put_map(priv, map);
	return err;
}

static long gntdev_ioctl_get_offset_for_vaddr(struct gntdev_priv *priv,
	struct ioctl_gntdev_get_offset_for_vaddr op;
	struct vm_area_struct *vma;
	struct grant_map *map;
	int rv = -EINVAL;

	if (copy_from_user(&op, u, sizeof(op)) != 0)
		return -EFAULT;
	pr_debug("priv %p, offset for vaddr %lx\n", priv, (unsigned long)op.vaddr);

	down_read(&current->mm->mmap_sem);
	vma = find_vma(current->mm, op.vaddr);
	if (!vma || vma->vm_ops != &gntdev_vmops)
		goto out_unlock;

	map = vma->vm_private_data;
	if (!map)
		goto out_unlock;

	op.offset = map->index << PAGE_SHIFT;
	op.count = map->count;
	rv = 0;

 out_unlock:
	up_read(&current->mm->mmap_sem);

	if (rv == 0 && copy_to_user(u, &op, sizeof(op)) != 0)
		return -EFAULT;
	return rv;
}

static long gntdev_ioctl_notify(struct gntdev_priv *priv, void __user *u)
{
out_put_map:
	if (use_ptemod)
		map->vma = NULL;
	gntdev_put_map(priv, map);
	return err;
}

static const struct file_operations gntdev_fops = {