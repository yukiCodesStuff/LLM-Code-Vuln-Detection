static atomic_t pages_mapped = ATOMIC_INIT(0);

static int use_ptemod;

struct gntdev_priv {
	struct list_head maps;
	/* lock protects maps from concurrent changes */
	spinlock_t lock;
	struct mm_struct *mm;
	struct mmu_notifier mn;
};
	return NULL;
}

static void gntdev_put_map(struct grant_map *map)
{
	if (!map)
		return;

		evtchn_put(map->notify.event);
	}

	if (map->pages && !use_ptemod)
		unmap_grant_pages(map, 0, map->count);
	gntdev_free_map(map);
}

	if (map->notify.flags & UNMAP_NOTIFY_CLEAR_BYTE) {
		int pgno = (map->notify.addr >> PAGE_SHIFT);
		if (pgno >= offset && pgno < offset + pages && use_ptemod) {
			void __user *tmp = (void __user *)
				map->vma->vm_start + map->notify.addr;
			err = copy_to_user(tmp, &err, 1);
			if (err)
				return -EFAULT;
			map->notify.flags &= ~UNMAP_NOTIFY_CLEAR_BYTE;
		} else if (pgno >= offset && pgno < offset + pages) {
			uint8_t *tmp = kmap(map->pages[pgno]);
			tmp[map->notify.addr & (PAGE_SIZE-1)] = 0;
			kunmap(map->pages[pgno]);
			map->notify.flags &= ~UNMAP_NOTIFY_CLEAR_BYTE;
		}
	}

static void gntdev_vma_close(struct vm_area_struct *vma)
{
	struct grant_map *map = vma->vm_private_data;

	pr_debug("gntdev_vma_close %p\n", vma);
	map->vma = NULL;
	vma->vm_private_data = NULL;
	gntdev_put_map(map);
}

static struct vm_operations_struct gntdev_vmops = {
	.open = gntdev_vma_open,

/* ------------------------------------------------------------------ */

static void mn_invl_range_start(struct mmu_notifier *mn,
				struct mm_struct *mm,
				unsigned long start, unsigned long end)
{
	struct gntdev_priv *priv = container_of(mn, struct gntdev_priv, mn);
	struct grant_map *map;
	unsigned long mstart, mend;
	int err;

	spin_lock(&priv->lock);
	list_for_each_entry(map, &priv->maps, next) {
		if (!map->vma)
			continue;
		if (map->vma->vm_start >= end)
			continue;
		if (map->vma->vm_end <= start)
			continue;
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
	spin_unlock(&priv->lock);
}

		err = unmap_grant_pages(map, /* offset */ 0, map->count);
		WARN_ON(err);
	}
	spin_unlock(&priv->lock);
}

static struct mmu_notifier_ops gntdev_mmu_ops = {
		return -ENOMEM;

	INIT_LIST_HEAD(&priv->maps);
	spin_lock_init(&priv->lock);

	if (use_ptemod) {
		priv->mm = get_task_mm(current);
	while (!list_empty(&priv->maps)) {
		map = list_entry(priv->maps.next, struct grant_map, next);
		list_del(&map->next);
		gntdev_put_map(map);
	}

	if (use_ptemod)
		mmu_notifier_unregister(&priv->mn, priv->mm);
	kfree(priv);

	if (unlikely(atomic_add_return(op.count, &pages_mapped) > limit)) {
		pr_debug("can't map: over limit\n");
		gntdev_put_map(map);
		return err;
	}

	if (copy_from_user(map->grants, &u->refs,
			   sizeof(map->grants[0]) * op.count) != 0) {
		gntdev_put_map(map);
		return err;
	}

	spin_lock(&priv->lock);
	gntdev_add_map(priv, map);
	map = gntdev_find_map_index(priv, op.index >> PAGE_SHIFT, op.count);
	if (map) {
		list_del(&map->next);
		err = 0;
	}
	spin_unlock(&priv->lock);
	if (map)
		gntdev_put_map(map);
	return err;
}

static long gntdev_ioctl_get_offset_for_vaddr(struct gntdev_priv *priv,
	struct ioctl_gntdev_get_offset_for_vaddr op;
	struct vm_area_struct *vma;
	struct grant_map *map;

	if (copy_from_user(&op, u, sizeof(op)) != 0)
		return -EFAULT;
	pr_debug("priv %p, offset for vaddr %lx\n", priv, (unsigned long)op.vaddr);

	vma = find_vma(current->mm, op.vaddr);
	if (!vma || vma->vm_ops != &gntdev_vmops)
		return -EINVAL;

	map = vma->vm_private_data;
	if (!map)
		return -EINVAL;

	op.offset = map->index << PAGE_SHIFT;
	op.count = map->count;

	if (copy_to_user(u, &op, sizeof(op)) != 0)
		return -EFAULT;
	return 0;
}

static long gntdev_ioctl_notify(struct gntdev_priv *priv, void __user *u)
{
out_put_map:
	if (use_ptemod)
		map->vma = NULL;
	gntdev_put_map(map);
	return err;
}

static const struct file_operations gntdev_fops = {