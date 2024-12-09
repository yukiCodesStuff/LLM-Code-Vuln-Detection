	} else {
		spin_unlock(&walk->mm->page_table_lock);
	}
	/*
	 * The mmap_sem held all the way back in m_start() is what
	 * keeps khugepaged out of here and from collapsing things
	 * in here.
	 */
	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE)
		smaps_pte_entry(*pte, addr, PAGE_SIZE, walk);
	pte_unmap_unlock(pte - 1, ptl);
	cond_resched();
	return 0;
}

static int show_smap(struct seq_file *m, void *v)
{
	struct proc_maps_private *priv = m->private;
	struct task_struct *task = priv->task;
	struct vm_area_struct *vma = v;
	struct mem_size_stats mss;
	struct mm_walk smaps_walk = {
		.pmd_entry = smaps_pte_range,
		.mm = vma->vm_mm,
		.private = &mss,
	};

	memset(&mss, 0, sizeof mss);
	mss.vma = vma;
	/* mmap_sem is held in m_start */
	if (vma->vm_mm && !is_vm_hugetlb_page(vma))
		walk_page_range(vma->vm_start, vma->vm_end, &smaps_walk);

	show_map_vma(m, vma);

	seq_printf(m,
		   "Size:           %8lu kB\n"
		   "Rss:            %8lu kB\n"
		   "Pss:            %8lu kB\n"
		   "Shared_Clean:   %8lu kB\n"
		   "Shared_Dirty:   %8lu kB\n"
		   "Private_Clean:  %8lu kB\n"
		   "Private_Dirty:  %8lu kB\n"
		   "Referenced:     %8lu kB\n"
		   "Anonymous:      %8lu kB\n"
		   "AnonHugePages:  %8lu kB\n"
		   "Swap:           %8lu kB\n"
		   "KernelPageSize: %8lu kB\n"
		   "MMUPageSize:    %8lu kB\n"
		   "Locked:         %8lu kB\n",
		   (vma->vm_end - vma->vm_start) >> 10,
		   mss.resident >> 10,
		   (unsigned long)(mss.pss >> (10 + PSS_SHIFT)),
		   mss.shared_clean  >> 10,
		   mss.shared_dirty  >> 10,
		   mss.private_clean >> 10,
		   mss.private_dirty >> 10,
		   mss.referenced >> 10,
		   mss.anonymous >> 10,
		   mss.anonymous_thp >> 10,
		   mss.swap >> 10,
		   vma_kernel_pagesize(vma) >> 10,
		   vma_mmu_pagesize(vma) >> 10,
		   (vma->vm_flags & VM_LOCKED) ?
			(unsigned long)(mss.pss >> (10 + PSS_SHIFT)) : 0);

	if (m->count < m->size)  /* vma is copied successfully */
		m->version = (vma != get_gate_vma(task->mm))
			? vma->vm_start : 0;
	return 0;
}

static const struct seq_operations proc_pid_smaps_op = {
	.start	= m_start,
	.next	= m_next,
	.stop	= m_stop,
	.show	= show_smap
};

static int smaps_open(struct inode *inode, struct file *file)
{
	return do_maps_open(inode, file, &proc_pid_smaps_op);
}
{
	struct vm_area_struct *vma = walk->private;
	pte_t *pte, ptent;
	spinlock_t *ptl;
	struct page *page;

	split_huge_page_pmd(walk->mm, pmd);

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;
		if (!pte_present(ptent))
			continue;

		page = vm_normal_page(vma, addr, ptent);
		if (!page)
			continue;

		if (PageReserved(page))
			continue;

		/* Clear accessed and referenced bits. */
		ptep_test_and_clear_young(vma, addr, pte);
		ClearPageReferenced(page);
	}
{
	struct vm_area_struct *vma;
	struct pagemapread *pm = walk->private;
	pte_t *pte;
	int err = 0;

	split_huge_page_pmd(walk->mm, pmd);

	/* find the first VMA at or above 'addr' */
	vma = find_vma(walk->mm, addr);
	for (; addr != end; addr += PAGE_SIZE) {
		u64 pfn = PM_NOT_PRESENT;

		/* check to see if we've left 'vma' behind
		 * and need a new, higher one */
		if (vma && (addr >= vma->vm_end))
			vma = find_vma(walk->mm, addr);

		/* check that 'vma' actually covers this address,
		 * and that it isn't a huge page vma */
		if (vma && (vma->vm_start <= addr) &&
		    !is_vm_hugetlb_page(vma)) {
			pte = pte_offset_map(pmd, addr);
			pfn = pte_to_pagemap_entry(*pte);
			/* unmap before userspace copy */
			pte_unmap(pte);
		}
		err = add_to_pagemap(addr, pfn, pm);
		if (err)
			return err;
	}
	} else {
		spin_unlock(&walk->mm->page_table_lock);
	}

	orig_pte = pte = pte_offset_map_lock(walk->mm, pmd, addr, &ptl);
	do {
		struct page *page = can_gather_numa_stats(*pte, md->vma, addr);
		if (!page)
			continue;
		gather_stats(page, md, pte_dirty(*pte), 1);

	} while (pte++, addr += PAGE_SIZE, addr != end);
	pte_unmap_unlock(orig_pte, ptl);
	return 0;
}
#ifdef CONFIG_HUGETLB_PAGE
static int gather_hugetbl_stats(pte_t *pte, unsigned long hmask,
		unsigned long addr, unsigned long end, struct mm_walk *walk)
{
	struct numa_maps *md;
	struct page *page;

	if (pte_none(*pte))
		return 0;

	page = pte_page(*pte);
	if (!page)
		return 0;

	md = walk->private;
	gather_stats(page, md, pte_dirty(*pte), 1);
	return 0;
}

#else
static int gather_hugetbl_stats(pte_t *pte, unsigned long hmask,
		unsigned long addr, unsigned long end, struct mm_walk *walk)
{
	return 0;
}
#endif

/*
 * Display pages allocated per node and memory policy via /proc.
 */
static int show_numa_map(struct seq_file *m, void *v)
{
	struct numa_maps_private *numa_priv = m->private;
	struct proc_maps_private *proc_priv = &numa_priv->proc_maps;
	struct vm_area_struct *vma = v;
	struct numa_maps *md = &numa_priv->md;
	struct file *file = vma->vm_file;
	struct mm_struct *mm = vma->vm_mm;
	struct mm_walk walk = {};
	struct mempolicy *pol;
	int n;
	char buffer[50];

	if (!mm)
		return 0;

	/* Ensure we start with an empty set of numa_maps statistics. */
	memset(md, 0, sizeof(*md));

	md->vma = vma;

	walk.hugetlb_entry = gather_hugetbl_stats;
	walk.pmd_entry = gather_pte_stats;
	walk.private = md;
	walk.mm = mm;

	pol = get_vma_policy(proc_priv->task, vma, vma->vm_start);
	mpol_to_str(buffer, sizeof(buffer), pol, 0);
	mpol_cond_put(pol);

	seq_printf(m, "%08lx %s", vma->vm_start, buffer);

	if (file) {
		seq_printf(m, " file=");
		seq_path(m, &file->f_path, "\n\t= ");
	} else if (vma->vm_start <= mm->brk && vma->vm_end >= mm->start_brk) {
		seq_printf(m, " heap");
	} else if (vma->vm_start <= mm->start_stack &&
			vma->vm_end >= mm->start_stack) {
		seq_printf(m, " stack");
	}

	if (is_vm_hugetlb_page(vma))
		seq_printf(m, " huge");

	walk_page_range(vma->vm_start, vma->vm_end, &walk);

	if (!md->pages)
		goto out;

	if (md->anon)
		seq_printf(m, " anon=%lu", md->anon);

	if (md->dirty)
		seq_printf(m, " dirty=%lu", md->dirty);

	if (md->pages != md->anon && md->pages != md->dirty)
		seq_printf(m, " mapped=%lu", md->pages);

	if (md->mapcount_max > 1)
		seq_printf(m, " mapmax=%lu", md->mapcount_max);

	if (md->swapcache)
		seq_printf(m, " swapcache=%lu", md->swapcache);

	if (md->active < md->pages && !is_vm_hugetlb_page(vma))
		seq_printf(m, " active=%lu", md->active);

	if (md->writeback)
		seq_printf(m, " writeback=%lu", md->writeback);

	for_each_node_state(n, N_HIGH_MEMORY)
		if (md->node[n])
			seq_printf(m, " N%d=%lu", n, md->node[n]);
out:
	seq_putc(m, '\n');

	if (m->count < m->size)
		m->version = (vma != proc_priv->tail_vma) ? vma->vm_start : 0;
	return 0;
}

static const struct seq_operations proc_pid_numa_maps_op = {
        .start  = m_start,
        .next   = m_next,
        .stop   = m_stop,
        .show   = show_numa_map,
};

static int numa_maps_open(struct inode *inode, struct file *file)
{
	struct numa_maps_private *priv;
	int ret = -ENOMEM;
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv) {
		priv->proc_maps.pid = proc_pid(inode);
		ret = seq_open(file, &proc_pid_numa_maps_op);
		if (!ret) {
			struct seq_file *m = file->private_data;
			m->private = priv;
		} else {
			kfree(priv);
		}
	}
	return ret;
}

const struct file_operations proc_numa_maps_operations = {
	.open		= numa_maps_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_private,
};
#endif /* CONFIG_NUMA */