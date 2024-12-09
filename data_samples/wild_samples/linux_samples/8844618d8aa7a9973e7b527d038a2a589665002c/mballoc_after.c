	if (meta_group_info[i] == NULL) {
		ext4_msg(sb, KERN_ERR, "can't allocate buddy mem");
		goto exit_group_info;
	}
	set_bit(EXT4_GROUP_INFO_NEED_INIT_BIT,
		&(meta_group_info[i]->bb_state));

	/*
	 * initialize bb_free to be able to skip
	 * empty groups without initialization
	 */
	if (ext4_has_group_desc_csum(sb) &&
	    (desc->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT))) {
		meta_group_info[i]->bb_free =
			ext4_free_clusters_after_init(sb, group, desc);
	} else {
		meta_group_info[i]->bb_free =
			ext4_free_group_clusters(sb, desc);
	}

	INIT_LIST_HEAD(&meta_group_info[i]->bb_prealloc_list);
	init_rwsem(&meta_group_info[i]->alloc_sem);
	meta_group_info[i]->bb_free_root = RB_ROOT;
	meta_group_info[i]->bb_largest_free_order = -1;  /* uninit */

#ifdef DOUBLE_CHECK
	{
		struct buffer_head *bh;
		meta_group_info[i]->bb_bitmap =
			kmalloc(sb->s_blocksize, GFP_NOFS);
		BUG_ON(meta_group_info[i]->bb_bitmap == NULL);
		bh = ext4_read_block_bitmap(sb, group);
		BUG_ON(IS_ERR_OR_NULL(bh));
		memcpy(meta_group_info[i]->bb_bitmap, bh->b_data,
			sb->s_blocksize);
		put_bh(bh);
	}
#endif

	return 0;

exit_group_info:
	/* If a meta_group_info table has been allocated, release it now */
	if (group % EXT4_DESC_PER_BLOCK(sb) == 0) {
		kfree(sbi->s_group_info[group >> EXT4_DESC_PER_BLOCK_BITS(sb)]);
		sbi->s_group_info[group >> EXT4_DESC_PER_BLOCK_BITS(sb)] = NULL;
	}
exit_meta_group_info:
	return -ENOMEM;
} /* ext4_mb_add_groupinfo */

static int ext4_mb_init_backend(struct super_block *sb)
{
	ext4_group_t ngroups = ext4_get_groups_count(sb);
	ext4_group_t i;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	int err;
	struct ext4_group_desc *desc;
	struct kmem_cache *cachep;

	err = ext4_mb_alloc_groupinfo(sb, ngroups);
	if (err)
		return err;

	sbi->s_buddy_cache = new_inode(sb);
	if (sbi->s_buddy_cache == NULL) {
		ext4_msg(sb, KERN_ERR, "can't get new inode");
		goto err_freesgi;
	}
	/* To avoid potentially colliding with an valid on-disk inode number,
	 * use EXT4_BAD_INO for the buddy cache inode number.  This inode is
	 * not in the inode hash, so it should never be found by iget(), but
	 * this will avoid confusion if it ever shows up during debugging. */
	sbi->s_buddy_cache->i_ino = EXT4_BAD_INO;
	EXT4_I(sbi->s_buddy_cache)->i_disksize = 0;
	for (i = 0; i < ngroups; i++) {
		desc = ext4_get_group_desc(sb, i, NULL);
		if (desc == NULL) {
			ext4_msg(sb, KERN_ERR, "can't read descriptor %u", i);
			goto err_freebuddy;
		}
		if (ext4_mb_add_groupinfo(sb, i, desc) != 0)
			goto err_freebuddy;
	}

	return 0;

err_freebuddy:
	cachep = get_groupinfo_cache(sb->s_blocksize_bits);
	while (i-- > 0)
		kmem_cache_free(cachep, ext4_get_group_info(sb, i));
	i = sbi->s_group_info_size;
	while (i-- > 0)
		kfree(sbi->s_group_info[i]);
	iput(sbi->s_buddy_cache);
err_freesgi:
	kvfree(sbi->s_group_info);
	return -ENOMEM;
}

static void ext4_groupinfo_destroy_slabs(void)
{
	int i;

	for (i = 0; i < NR_GRPINFO_CACHES; i++) {
		kmem_cache_destroy(ext4_groupinfo_caches[i]);
		ext4_groupinfo_caches[i] = NULL;
	}
}

static int ext4_groupinfo_create_slab(size_t size)
{
	static DEFINE_MUTEX(ext4_grpinfo_slab_create_mutex);
	int slab_size;
	int blocksize_bits = order_base_2(size);
	int cache_index = blocksize_bits - EXT4_MIN_BLOCK_LOG_SIZE;
	struct kmem_cache *cachep;

	if (cache_index >= NR_GRPINFO_CACHES)
		return -EINVAL;

	if (unlikely(cache_index < 0))
		cache_index = 0;

	mutex_lock(&ext4_grpinfo_slab_create_mutex);
	if (ext4_groupinfo_caches[cache_index]) {
		mutex_unlock(&ext4_grpinfo_slab_create_mutex);
		return 0;	/* Already created */
	}

	slab_size = offsetof(struct ext4_group_info,
				bb_counters[blocksize_bits + 2]);

	cachep = kmem_cache_create(ext4_groupinfo_slab_names[cache_index],
					slab_size, 0, SLAB_RECLAIM_ACCOUNT,
					NULL);

	ext4_groupinfo_caches[cache_index] = cachep;

	mutex_unlock(&ext4_grpinfo_slab_create_mutex);
	if (!cachep) {
		printk(KERN_EMERG
		       "EXT4-fs: no memory for groupinfo slab cache\n");
		return -ENOMEM;
	}

	return 0;
}

int ext4_mb_init(struct super_block *sb)
{
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	unsigned i, j;
	unsigned offset, offset_incr;
	unsigned max;
	int ret;

	i = (sb->s_blocksize_bits + 2) * sizeof(*sbi->s_mb_offsets);

	sbi->s_mb_offsets = kmalloc(i, GFP_KERNEL);
	if (sbi->s_mb_offsets == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	i = (sb->s_blocksize_bits + 2) * sizeof(*sbi->s_mb_maxs);
	sbi->s_mb_maxs = kmalloc(i, GFP_KERNEL);
	if (sbi->s_mb_maxs == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	ret = ext4_groupinfo_create_slab(sb->s_blocksize);
	if (ret < 0)
		goto out;

	/* order 0 is regular bitmap */
	sbi->s_mb_maxs[0] = sb->s_blocksize << 3;
	sbi->s_mb_offsets[0] = 0;

	i = 1;
	offset = 0;
	offset_incr = 1 << (sb->s_blocksize_bits - 1);
	max = sb->s_blocksize << 2;
	do {
		sbi->s_mb_offsets[i] = offset;
		sbi->s_mb_maxs[i] = max;
		offset += offset_incr;
		offset_incr = offset_incr >> 1;
		max = max >> 1;
		i++;
	} while (i <= sb->s_blocksize_bits + 1);

	spin_lock_init(&sbi->s_md_lock);
	spin_lock_init(&sbi->s_bal_lock);
	sbi->s_mb_free_pending = 0;
	INIT_LIST_HEAD(&sbi->s_freed_data_list);

	sbi->s_mb_max_to_scan = MB_DEFAULT_MAX_TO_SCAN;
	sbi->s_mb_min_to_scan = MB_DEFAULT_MIN_TO_SCAN;
	sbi->s_mb_stats = MB_DEFAULT_STATS;
	sbi->s_mb_stream_request = MB_DEFAULT_STREAM_THRESHOLD;
	sbi->s_mb_order2_reqs = MB_DEFAULT_ORDER2_REQS;
	/*
	 * The default group preallocation is 512, which for 4k block
	 * sizes translates to 2 megabytes.  However for bigalloc file
	 * systems, this is probably too big (i.e, if the cluster size
	 * is 1 megabyte, then group preallocation size becomes half a
	 * gigabyte!).  As a default, we will keep a two megabyte
	 * group pralloc size for cluster sizes up to 64k, and after
	 * that, we will force a minimum group preallocation size of
	 * 32 clusters.  This translates to 8 megs when the cluster
	 * size is 256k, and 32 megs when the cluster size is 1 meg,
	 * which seems reasonable as a default.
	 */
	sbi->s_mb_group_prealloc = max(MB_DEFAULT_GROUP_PREALLOC >>
				       sbi->s_cluster_bits, 32);
	/*
	 * If there is a s_stripe > 1, then we set the s_mb_group_prealloc
	 * to the lowest multiple of s_stripe which is bigger than
	 * the s_mb_group_prealloc as determined above. We want
	 * the preallocation size to be an exact multiple of the
	 * RAID stripe size so that preallocations don't fragment
	 * the stripes.
	 */
	if (sbi->s_stripe > 1) {
		sbi->s_mb_group_prealloc = roundup(
			sbi->s_mb_group_prealloc, sbi->s_stripe);
	}

	sbi->s_locality_groups = alloc_percpu(struct ext4_locality_group);
	if (sbi->s_locality_groups == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	for_each_possible_cpu(i) {
		struct ext4_locality_group *lg;
		lg = per_cpu_ptr(sbi->s_locality_groups, i);
		mutex_init(&lg->lg_mutex);
		for (j = 0; j < PREALLOC_TB_SIZE; j++)
			INIT_LIST_HEAD(&lg->lg_prealloc_list[j]);
		spin_lock_init(&lg->lg_prealloc_lock);
	}

	/* init file for buddy data */
	ret = ext4_mb_init_backend(sb);
	if (ret != 0)
		goto out_free_locality_groups;

	return 0;

out_free_locality_groups:
	free_percpu(sbi->s_locality_groups);
	sbi->s_locality_groups = NULL;
out:
	kfree(sbi->s_mb_offsets);
	sbi->s_mb_offsets = NULL;
	kfree(sbi->s_mb_maxs);
	sbi->s_mb_maxs = NULL;
	return ret;
}

/* need to called with the ext4 group lock held */
static void ext4_mb_cleanup_pa(struct ext4_group_info *grp)
{
	struct ext4_prealloc_space *pa;
	struct list_head *cur, *tmp;
	int count = 0;

	list_for_each_safe(cur, tmp, &grp->bb_prealloc_list) {
		pa = list_entry(cur, struct ext4_prealloc_space, pa_group_list);
		list_del(&pa->pa_group_list);
		count++;
		kmem_cache_free(ext4_pspace_cachep, pa);
	}
	if (count)
		mb_debug(1, "mballoc: %u PAs left\n", count);

}

int ext4_mb_release(struct super_block *sb)
{
	ext4_group_t ngroups = ext4_get_groups_count(sb);
	ext4_group_t i;
	int num_meta_group_infos;
	struct ext4_group_info *grinfo;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct kmem_cache *cachep = get_groupinfo_cache(sb->s_blocksize_bits);

	if (sbi->s_group_info) {
		for (i = 0; i < ngroups; i++) {
			grinfo = ext4_get_group_info(sb, i);
#ifdef DOUBLE_CHECK
			kfree(grinfo->bb_bitmap);
#endif
			ext4_lock_group(sb, i);
			ext4_mb_cleanup_pa(grinfo);
			ext4_unlock_group(sb, i);
			kmem_cache_free(cachep, grinfo);
		}
		num_meta_group_infos = (ngroups +
				EXT4_DESC_PER_BLOCK(sb) - 1) >>
			EXT4_DESC_PER_BLOCK_BITS(sb);
		for (i = 0; i < num_meta_group_infos; i++)
			kfree(sbi->s_group_info[i]);
		kvfree(sbi->s_group_info);
	}
	kfree(sbi->s_mb_offsets);
	kfree(sbi->s_mb_maxs);
	iput(sbi->s_buddy_cache);
	if (sbi->s_mb_stats) {
		ext4_msg(sb, KERN_INFO,
		       "mballoc: %u blocks %u reqs (%u success)",
				atomic_read(&sbi->s_bal_allocated),
				atomic_read(&sbi->s_bal_reqs),
				atomic_read(&sbi->s_bal_success));
		ext4_msg(sb, KERN_INFO,
		      "mballoc: %u extents scanned, %u goal hits, "
				"%u 2^N hits, %u breaks, %u lost",
				atomic_read(&sbi->s_bal_ex_scanned),
				atomic_read(&sbi->s_bal_goals),
				atomic_read(&sbi->s_bal_2orders),
				atomic_read(&sbi->s_bal_breaks),
				atomic_read(&sbi->s_mb_lost_chunks));
		ext4_msg(sb, KERN_INFO,
		       "mballoc: %lu generated and it took %Lu",
				sbi->s_mb_buddies_generated,
				sbi->s_mb_generation_time);
		ext4_msg(sb, KERN_INFO,
		       "mballoc: %u preallocated, %u discarded",
				atomic_read(&sbi->s_mb_preallocated),
				atomic_read(&sbi->s_mb_discarded));
	}

	free_percpu(sbi->s_locality_groups);

	return 0;
}

static inline int ext4_issue_discard(struct super_block *sb,
		ext4_group_t block_group, ext4_grpblk_t cluster, int count,
		struct bio **biop)
{
	ext4_fsblk_t discard_block;

	discard_block = (EXT4_C2B(EXT4_SB(sb), cluster) +
			 ext4_group_first_block_no(sb, block_group));
	count = EXT4_C2B(EXT4_SB(sb), count);
	trace_ext4_discard_blocks(sb,
			(unsigned long long) discard_block, count);
	if (biop) {
		return __blkdev_issue_discard(sb->s_bdev,
			(sector_t)discard_block << (sb->s_blocksize_bits - 9),
			(sector_t)count << (sb->s_blocksize_bits - 9),
			GFP_NOFS, 0, biop);
	} else
		return sb_issue_discard(sb, discard_block, count, GFP_NOFS, 0);
}

static void ext4_free_data_in_buddy(struct super_block *sb,
				    struct ext4_free_data *entry)
{
	struct ext4_buddy e4b;
	struct ext4_group_info *db;
	int err, count = 0, count2 = 0;

	mb_debug(1, "gonna free %u blocks in group %u (0x%p):",
		 entry->efd_count, entry->efd_group, entry);

	err = ext4_mb_load_buddy(sb, entry->efd_group, &e4b);
	/* we expect to find existing buddy because it's pinned */
	BUG_ON(err != 0);

	spin_lock(&EXT4_SB(sb)->s_md_lock);
	EXT4_SB(sb)->s_mb_free_pending -= entry->efd_count;
	spin_unlock(&EXT4_SB(sb)->s_md_lock);

	db = e4b.bd_info;
	/* there are blocks to put in buddy to make them really free */
	count += entry->efd_count;
	count2++;
	ext4_lock_group(sb, entry->efd_group);
	/* Take it out of per group rb tree */
	rb_erase(&entry->efd_node, &(db->bb_free_root));
	mb_free_blocks(NULL, &e4b, entry->efd_start_cluster, entry->efd_count);

	/*
	 * Clear the trimmed flag for the group so that the next
	 * ext4_trim_fs can trim it.
	 * If the volume is mounted with -o discard, online discard
	 * is supported and the free blocks will be trimmed online.
	 */
	if (!test_opt(sb, DISCARD))
		EXT4_MB_GRP_CLEAR_TRIMMED(db);

	if (!db->bb_free_root.rb_node) {
		/* No more items in the per group rb tree
		 * balance refcounts from ext4_mb_free_metadata()
		 */
		put_page(e4b.bd_buddy_page);
		put_page(e4b.bd_bitmap_page);
	}
	ext4_unlock_group(sb, entry->efd_group);
	kmem_cache_free(ext4_free_data_cachep, entry);
	ext4_mb_unload_buddy(&e4b);

	mb_debug(1, "freed %u blocks in %u structures\n", count, count2);
}

/*
 * This function is called by the jbd2 layer once the commit has finished,
 * so we know we can free the blocks that were released with that commit.
 */
void ext4_process_freed_data(struct super_block *sb, tid_t commit_tid)
{
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct ext4_free_data *entry, *tmp;
	struct bio *discard_bio = NULL;
	struct list_head freed_data_list;
	struct list_head *cut_pos = NULL;
	int err;

	INIT_LIST_HEAD(&freed_data_list);

	spin_lock(&sbi->s_md_lock);
	list_for_each_entry(entry, &sbi->s_freed_data_list, efd_list) {
		if (entry->efd_tid != commit_tid)
			break;
		cut_pos = &entry->efd_list;
	}
	if (cut_pos)
		list_cut_position(&freed_data_list, &sbi->s_freed_data_list,
				  cut_pos);
	spin_unlock(&sbi->s_md_lock);

	if (test_opt(sb, DISCARD)) {
		list_for_each_entry(entry, &freed_data_list, efd_list) {
			err = ext4_issue_discard(sb, entry->efd_group,
						 entry->efd_start_cluster,
						 entry->efd_count,
						 &discard_bio);
			if (err && err != -EOPNOTSUPP) {
				ext4_msg(sb, KERN_WARNING, "discard request in"
					 " group:%d block:%d count:%d failed"
					 " with %d", entry->efd_group,
					 entry->efd_start_cluster,
					 entry->efd_count, err);
			} else if (err == -EOPNOTSUPP)
				break;
		}

		if (discard_bio) {
			submit_bio_wait(discard_bio);
			bio_put(discard_bio);
		}
	}

	list_for_each_entry_safe(entry, tmp, &freed_data_list, efd_list)
		ext4_free_data_in_buddy(sb, entry);
}

int __init ext4_init_mballoc(void)
{
	ext4_pspace_cachep = KMEM_CACHE(ext4_prealloc_space,
					SLAB_RECLAIM_ACCOUNT);
	if (ext4_pspace_cachep == NULL)
		return -ENOMEM;

	ext4_ac_cachep = KMEM_CACHE(ext4_allocation_context,
				    SLAB_RECLAIM_ACCOUNT);
	if (ext4_ac_cachep == NULL) {
		kmem_cache_destroy(ext4_pspace_cachep);
		return -ENOMEM;
	}

	ext4_free_data_cachep = KMEM_CACHE(ext4_free_data,
					   SLAB_RECLAIM_ACCOUNT);
	if (ext4_free_data_cachep == NULL) {
		kmem_cache_destroy(ext4_pspace_cachep);
		kmem_cache_destroy(ext4_ac_cachep);
		return -ENOMEM;
	}
	return 0;
}

void ext4_exit_mballoc(void)
{
	/*
	 * Wait for completion of call_rcu()'s on ext4_pspace_cachep
	 * before destroying the slab cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(ext4_pspace_cachep);
	kmem_cache_destroy(ext4_ac_cachep);
	kmem_cache_destroy(ext4_free_data_cachep);
	ext4_groupinfo_destroy_slabs();
}


/*
 * Check quota and mark chosen space (ac->ac_b_ex) non-free in bitmaps
 * Returns 0 if success or error code
 */
static noinline_for_stack int
ext4_mb_mark_diskspace_used(struct ext4_allocation_context *ac,
				handle_t *handle, unsigned int reserv_clstrs)
{
	struct buffer_head *bitmap_bh = NULL;
	struct ext4_group_desc *gdp;
	struct buffer_head *gdp_bh;
	struct ext4_sb_info *sbi;
	struct super_block *sb;
	ext4_fsblk_t block;
	int err, len;

	BUG_ON(ac->ac_status != AC_STATUS_FOUND);
	BUG_ON(ac->ac_b_ex.fe_len <= 0);

	sb = ac->ac_sb;
	sbi = EXT4_SB(sb);

	bitmap_bh = ext4_read_block_bitmap(sb, ac->ac_b_ex.fe_group);
	if (IS_ERR(bitmap_bh)) {
		err = PTR_ERR(bitmap_bh);
		bitmap_bh = NULL;
		goto out_err;
	}

	BUFFER_TRACE(bitmap_bh, "getting write access");
	err = ext4_journal_get_write_access(handle, bitmap_bh);
	if (err)
		goto out_err;

	err = -EIO;
	gdp = ext4_get_group_desc(sb, ac->ac_b_ex.fe_group, &gdp_bh);
	if (!gdp)
		goto out_err;

	ext4_debug("using block group %u(%d)\n", ac->ac_b_ex.fe_group,
			ext4_free_group_clusters(sb, gdp));

	BUFFER_TRACE(gdp_bh, "get_write_access");
	err = ext4_journal_get_write_access(handle, gdp_bh);
	if (err)
		goto out_err;

	block = ext4_grp_offs_to_block(sb, &ac->ac_b_ex);

	len = EXT4_C2B(sbi, ac->ac_b_ex.fe_len);
	if (!ext4_data_block_valid(sbi, block, len)) {
		ext4_error(sb, "Allocating blocks %llu-%llu which overlap "
			   "fs metadata", block, block+len);
		/* File system mounted not to panic on error
		 * Fix the bitmap and return EFSCORRUPTED
		 * We leak some of the blocks here.
		 */
		ext4_lock_group(sb, ac->ac_b_ex.fe_group);
		ext4_set_bits(bitmap_bh->b_data, ac->ac_b_ex.fe_start,
			      ac->ac_b_ex.fe_len);
		ext4_unlock_group(sb, ac->ac_b_ex.fe_group);
		err = ext4_handle_dirty_metadata(handle, NULL, bitmap_bh);
		if (!err)
			err = -EFSCORRUPTED;
		goto out_err;
	}

	ext4_lock_group(sb, ac->ac_b_ex.fe_group);
#ifdef AGGRESSIVE_CHECK
	{
		int i;
		for (i = 0; i < ac->ac_b_ex.fe_len; i++) {
			BUG_ON(mb_test_bit(ac->ac_b_ex.fe_start + i,
						bitmap_bh->b_data));
		}
	}
#endif
	ext4_set_bits(bitmap_bh->b_data, ac->ac_b_ex.fe_start,
		      ac->ac_b_ex.fe_len);
	if (ext4_has_group_desc_csum(sb) &&
	    (gdp->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT))) {
		gdp->bg_flags &= cpu_to_le16(~EXT4_BG_BLOCK_UNINIT);
		ext4_free_group_clusters_set(sb, gdp,
					     ext4_free_clusters_after_init(sb,
						ac->ac_b_ex.fe_group, gdp));
	}
	len = ext4_free_group_clusters(sb, gdp) - ac->ac_b_ex.fe_len;
	ext4_free_group_clusters_set(sb, gdp, len);
	ext4_block_bitmap_csum_set(sb, ac->ac_b_ex.fe_group, gdp, bitmap_bh);
	ext4_group_desc_csum_set(sb, ac->ac_b_ex.fe_group, gdp);

	ext4_unlock_group(sb, ac->ac_b_ex.fe_group);
	percpu_counter_sub(&sbi->s_freeclusters_counter, ac->ac_b_ex.fe_len);
	/*
	 * Now reduce the dirty block count also. Should not go negative
	 */
	if (!(ac->ac_flags & EXT4_MB_DELALLOC_RESERVED))
		/* release all the reserved blocks if non delalloc */
		percpu_counter_sub(&sbi->s_dirtyclusters_counter,
				   reserv_clstrs);

	if (sbi->s_log_groups_per_flex) {
		ext4_group_t flex_group = ext4_flex_group(sbi,
							  ac->ac_b_ex.fe_group);
		atomic64_sub(ac->ac_b_ex.fe_len,
			     &sbi->s_flex_groups[flex_group].free_clusters);
	}

	err = ext4_handle_dirty_metadata(handle, NULL, bitmap_bh);
	if (err)
		goto out_err;
	err = ext4_handle_dirty_metadata(handle, NULL, gdp_bh);

out_err:
	brelse(bitmap_bh);
	return err;
}

/*
 * here we normalize request for locality group
 * Group request are normalized to s_mb_group_prealloc, which goes to
 * s_strip if we set the same via mount option.
 * s_mb_group_prealloc can be configured via
 * /sys/fs/ext4/<partition>/mb_group_prealloc
 *
 * XXX: should we try to preallocate more than the group has now?
 */
static void ext4_mb_normalize_group_request(struct ext4_allocation_context *ac)
{
	struct super_block *sb = ac->ac_sb;
	struct ext4_locality_group *lg = ac->ac_lg;

	BUG_ON(lg == NULL);
	ac->ac_g_ex.fe_len = EXT4_SB(sb)->s_mb_group_prealloc;
	mb_debug(1, "#%u: goal %u blocks for locality group\n",
		current->pid, ac->ac_g_ex.fe_len);
}

/*
 * Normalization means making request better in terms of
 * size and alignment
 */
static noinline_for_stack void
ext4_mb_normalize_request(struct ext4_allocation_context *ac,
				struct ext4_allocation_request *ar)
{
	struct ext4_sb_info *sbi = EXT4_SB(ac->ac_sb);
	int bsbits, max;
	ext4_lblk_t end;
	loff_t size, start_off;
	loff_t orig_size __maybe_unused;
	ext4_lblk_t start;
	struct ext4_inode_info *ei = EXT4_I(ac->ac_inode);
	struct ext4_prealloc_space *pa;

	/* do normalize only data requests, metadata requests
	   do not need preallocation */
	if (!(ac->ac_flags & EXT4_MB_HINT_DATA))
		return;

	/* sometime caller may want exact blocks */
	if (unlikely(ac->ac_flags & EXT4_MB_HINT_GOAL_ONLY))
		return;

	/* caller may indicate that preallocation isn't
	 * required (it's a tail, for example) */
	if (ac->ac_flags & EXT4_MB_HINT_NOPREALLOC)
		return;

	if (ac->ac_flags & EXT4_MB_HINT_GROUP_ALLOC) {
		ext4_mb_normalize_group_request(ac);
		return ;
	}

	bsbits = ac->ac_sb->s_blocksize_bits;

	/* first, let's learn actual file size
	 * given current request is allocated */
	size = ac->ac_o_ex.fe_logical + EXT4_C2B(sbi, ac->ac_o_ex.fe_len);
	size = size << bsbits;
	if (size < i_size_read(ac->ac_inode))
		size = i_size_read(ac->ac_inode);
	orig_size = size;

	/* max size of free chunks */
	max = 2 << bsbits;

#define NRL_CHECK_SIZE(req, size, max, chunk_size)	\
		(req <= (size) || max <= (chunk_size))

	/* first, try to predict filesize */
	/* XXX: should this table be tunable? */
	start_off = 0;
	if (size <= 16 * 1024) {
		size = 16 * 1024;
	} else if (size <= 32 * 1024) {
		size = 32 * 1024;
	} else if (size <= 64 * 1024) {
		size = 64 * 1024;
	} else if (size <= 128 * 1024) {
		size = 128 * 1024;
	} else if (size <= 256 * 1024) {
		size = 256 * 1024;
	} else if (size <= 512 * 1024) {
		size = 512 * 1024;
	} else if (size <= 1024 * 1024) {
		size = 1024 * 1024;
	} else if (NRL_CHECK_SIZE(size, 4 * 1024 * 1024, max, 2 * 1024)) {
		start_off = ((loff_t)ac->ac_o_ex.fe_logical >>
						(21 - bsbits)) << 21;
		size = 2 * 1024 * 1024;
	} else if (NRL_CHECK_SIZE(size, 8 * 1024 * 1024, max, 4 * 1024)) {
		start_off = ((loff_t)ac->ac_o_ex.fe_logical >>
							(22 - bsbits)) << 22;
		size = 4 * 1024 * 1024;
	} else if (NRL_CHECK_SIZE(ac->ac_o_ex.fe_len,
					(8<<20)>>bsbits, max, 8 * 1024)) {
		start_off = ((loff_t)ac->ac_o_ex.fe_logical >>
							(23 - bsbits)) << 23;
		size = 8 * 1024 * 1024;
	} else {
		start_off = (loff_t) ac->ac_o_ex.fe_logical << bsbits;
		size	  = (loff_t) EXT4_C2B(EXT4_SB(ac->ac_sb),
					      ac->ac_o_ex.fe_len) << bsbits;
	}
	size = size >> bsbits;
	start = start_off >> bsbits;

	/* don't cover already allocated blocks in selected range */
	if (ar->pleft && start <= ar->lleft) {
		size -= ar->lleft + 1 - start;
		start = ar->lleft + 1;
	}
	if (ar->pright && start + size - 1 >= ar->lright)
		size -= start + size - ar->lright;

	/*
	 * Trim allocation request for filesystems with artificially small
	 * groups.
	 */
	if (size > EXT4_BLOCKS_PER_GROUP(ac->ac_sb))
		size = EXT4_BLOCKS_PER_GROUP(ac->ac_sb);

	end = start + size;

	/* check we don't cross already preallocated blocks */
	rcu_read_lock();
	list_for_each_entry_rcu(pa, &ei->i_prealloc_list, pa_inode_list) {
		ext4_lblk_t pa_end;

		if (pa->pa_deleted)
			continue;
		spin_lock(&pa->pa_lock);
		if (pa->pa_deleted) {
			spin_unlock(&pa->pa_lock);
			continue;
		}

		pa_end = pa->pa_lstart + EXT4_C2B(EXT4_SB(ac->ac_sb),
						  pa->pa_len);

		/* PA must not overlap original request */
		BUG_ON(!(ac->ac_o_ex.fe_logical >= pa_end ||
			ac->ac_o_ex.fe_logical < pa->pa_lstart));

		/* skip PAs this normalized request doesn't overlap with */
		if (pa->pa_lstart >= end || pa_end <= start) {
			spin_unlock(&pa->pa_lock);
			continue;
		}
		BUG_ON(pa->pa_lstart <= start && pa_end >= end);

		/* adjust start or end to be adjacent to this pa */
		if (pa_end <= ac->ac_o_ex.fe_logical) {
			BUG_ON(pa_end < start);
			start = pa_end;
		} else if (pa->pa_lstart > ac->ac_o_ex.fe_logical) {
			BUG_ON(pa->pa_lstart > end);
			end = pa->pa_lstart;
		}
		spin_unlock(&pa->pa_lock);
	}
	rcu_read_unlock();
	size = end - start;

	/* XXX: extra loop to check we really don't overlap preallocations */
	rcu_read_lock();
	list_for_each_entry_rcu(pa, &ei->i_prealloc_list, pa_inode_list) {
		ext4_lblk_t pa_end;

		spin_lock(&pa->pa_lock);
		if (pa->pa_deleted == 0) {
			pa_end = pa->pa_lstart + EXT4_C2B(EXT4_SB(ac->ac_sb),
							  pa->pa_len);
			BUG_ON(!(start >= pa_end || end <= pa->pa_lstart));
		}
		spin_unlock(&pa->pa_lock);
	}
	rcu_read_unlock();

	if (start + size <= ac->ac_o_ex.fe_logical &&
			start > ac->ac_o_ex.fe_logical) {
		ext4_msg(ac->ac_sb, KERN_ERR,
			 "start %lu, size %lu, fe_logical %lu",
			 (unsigned long) start, (unsigned long) size,
			 (unsigned long) ac->ac_o_ex.fe_logical);
		BUG();
	}
	BUG_ON(size <= 0 || size > EXT4_BLOCKS_PER_GROUP(ac->ac_sb));

	/* now prepare goal request */

	/* XXX: is it better to align blocks WRT to logical
	 * placement or satisfy big request as is */
	ac->ac_g_ex.fe_logical = start;
	ac->ac_g_ex.fe_len = EXT4_NUM_B2C(sbi, size);

	/* define goal start in order to merge */
	if (ar->pright && (ar->lright == (start + size))) {
		/* merge to the right */
		ext4_get_group_no_and_offset(ac->ac_sb, ar->pright - size,
						&ac->ac_f_ex.fe_group,
						&ac->ac_f_ex.fe_start);
		ac->ac_flags |= EXT4_MB_HINT_TRY_GOAL;
	}
	if (ar->pleft && (ar->lleft + 1 == start)) {
		/* merge to the left */
		ext4_get_group_no_and_offset(ac->ac_sb, ar->pleft + 1,
						&ac->ac_f_ex.fe_group,
						&ac->ac_f_ex.fe_start);
		ac->ac_flags |= EXT4_MB_HINT_TRY_GOAL;
	}

	mb_debug(1, "goal: %u(was %u) blocks at %u\n", (unsigned) size,
		(unsigned) orig_size, (unsigned) start);
}

static void ext4_mb_collect_stats(struct ext4_allocation_context *ac)
{
	struct ext4_sb_info *sbi = EXT4_SB(ac->ac_sb);

	if (sbi->s_mb_stats && ac->ac_g_ex.fe_len > 1) {
		atomic_inc(&sbi->s_bal_reqs);
		atomic_add(ac->ac_b_ex.fe_len, &sbi->s_bal_allocated);
		if (ac->ac_b_ex.fe_len >= ac->ac_o_ex.fe_len)
			atomic_inc(&sbi->s_bal_success);
		atomic_add(ac->ac_found, &sbi->s_bal_ex_scanned);
		if (ac->ac_g_ex.fe_start == ac->ac_b_ex.fe_start &&
				ac->ac_g_ex.fe_group == ac->ac_b_ex.fe_group)
			atomic_inc(&sbi->s_bal_goals);
		if (ac->ac_found > sbi->s_mb_max_to_scan)
			atomic_inc(&sbi->s_bal_breaks);
	}

	if (ac->ac_op == EXT4_MB_HISTORY_ALLOC)
		trace_ext4_mballoc_alloc(ac);
	else
		trace_ext4_mballoc_prealloc(ac);
}

/*
 * Called on failure; free up any blocks from the inode PA for this
 * context.  We don't need this for MB_GROUP_PA because we only change
 * pa_free in ext4_mb_release_context(), but on failure, we've already
 * zeroed out ac->ac_b_ex.fe_len, so group_pa->pa_free is not changed.
 */
static void ext4_discard_allocated_blocks(struct ext4_allocation_context *ac)
{
	struct ext4_prealloc_space *pa = ac->ac_pa;
	struct ext4_buddy e4b;
	int err;

	if (pa == NULL) {
		if (ac->ac_f_ex.fe_len == 0)
			return;
		err = ext4_mb_load_buddy(ac->ac_sb, ac->ac_f_ex.fe_group, &e4b);
		if (err) {
			/*
			 * This should never happen since we pin the
			 * pages in the ext4_allocation_context so
			 * ext4_mb_load_buddy() should never fail.
			 */
			WARN(1, "mb_load_buddy failed (%d)", err);
			return;
		}
		ext4_lock_group(ac->ac_sb, ac->ac_f_ex.fe_group);
		mb_free_blocks(ac->ac_inode, &e4b, ac->ac_f_ex.fe_start,
			       ac->ac_f_ex.fe_len);
		ext4_unlock_group(ac->ac_sb, ac->ac_f_ex.fe_group);
		ext4_mb_unload_buddy(&e4b);
		return;
	}
	if (pa->pa_type == MB_INODE_PA)
		pa->pa_free += ac->ac_b_ex.fe_len;
}

/*
 * use blocks preallocated to inode
 */
static void ext4_mb_use_inode_pa(struct ext4_allocation_context *ac,
				struct ext4_prealloc_space *pa)
{
	struct ext4_sb_info *sbi = EXT4_SB(ac->ac_sb);
	ext4_fsblk_t start;
	ext4_fsblk_t end;
	int len;

	/* found preallocated blocks, use them */
	start = pa->pa_pstart + (ac->ac_o_ex.fe_logical - pa->pa_lstart);
	end = min(pa->pa_pstart + EXT4_C2B(sbi, pa->pa_len),
		  start + EXT4_C2B(sbi, ac->ac_o_ex.fe_len));
	len = EXT4_NUM_B2C(sbi, end - start);
	ext4_get_group_no_and_offset(ac->ac_sb, start, &ac->ac_b_ex.fe_group,
					&ac->ac_b_ex.fe_start);
	ac->ac_b_ex.fe_len = len;
	ac->ac_status = AC_STATUS_FOUND;
	ac->ac_pa = pa;

	BUG_ON(start < pa->pa_pstart);
	BUG_ON(end > pa->pa_pstart + EXT4_C2B(sbi, pa->pa_len));
	BUG_ON(pa->pa_free < len);
	pa->pa_free -= len;

	mb_debug(1, "use %llu/%u from inode pa %p\n", start, len, pa);
}

/*
 * use blocks preallocated to locality group
 */
static void ext4_mb_use_group_pa(struct ext4_allocation_context *ac,
				struct ext4_prealloc_space *pa)
{
	unsigned int len = ac->ac_o_ex.fe_len;

	ext4_get_group_no_and_offset(ac->ac_sb, pa->pa_pstart,
					&ac->ac_b_ex.fe_group,
					&ac->ac_b_ex.fe_start);
	ac->ac_b_ex.fe_len = len;
	ac->ac_status = AC_STATUS_FOUND;
	ac->ac_pa = pa;

	/* we don't correct pa_pstart or pa_plen here to avoid
	 * possible race when the group is being loaded concurrently
	 * instead we correct pa later, after blocks are marked
	 * in on-disk bitmap -- see ext4_mb_release_context()
	 * Other CPUs are prevented from allocating from this pa by lg_mutex
	 */
	mb_debug(1, "use %u/%u from group pa %p\n", pa->pa_lstart-len, len, pa);
}

/*
 * Return the prealloc space that have minimal distance
 * from the goal block. @cpa is the prealloc
 * space that is having currently known minimal distance
 * from the goal block.
 */
static struct ext4_prealloc_space *
ext4_mb_check_group_pa(ext4_fsblk_t goal_block,
			struct ext4_prealloc_space *pa,
			struct ext4_prealloc_space *cpa)
{
	ext4_fsblk_t cur_distance, new_distance;

	if (cpa == NULL) {
		atomic_inc(&pa->pa_count);
		return pa;
	}
	cur_distance = abs(goal_block - cpa->pa_pstart);
	new_distance = abs(goal_block - pa->pa_pstart);

	if (cur_distance <= new_distance)
		return cpa;

	/* drop the previous reference */
	atomic_dec(&cpa->pa_count);
	atomic_inc(&pa->pa_count);
	return pa;
}

/*
 * search goal blocks in preallocated space
 */
static noinline_for_stack int
ext4_mb_use_preallocated(struct ext4_allocation_context *ac)
{
	struct ext4_sb_info *sbi = EXT4_SB(ac->ac_sb);
	int order, i;
	struct ext4_inode_info *ei = EXT4_I(ac->ac_inode);
	struct ext4_locality_group *lg;
	struct ext4_prealloc_space *pa, *cpa = NULL;
	ext4_fsblk_t goal_block;

	/* only data can be preallocated */
	if (!(ac->ac_flags & EXT4_MB_HINT_DATA))
		return 0;

	/* first, try per-file preallocation */
	rcu_read_lock();
	list_for_each_entry_rcu(pa, &ei->i_prealloc_list, pa_inode_list) {

		/* all fields in this condition don't change,
		 * so we can skip locking for them */
		if (ac->ac_o_ex.fe_logical < pa->pa_lstart ||
		    ac->ac_o_ex.fe_logical >= (pa->pa_lstart +
					       EXT4_C2B(sbi, pa->pa_len)))
			continue;

		/* non-extent files can't have physical blocks past 2^32 */
		if (!(ext4_test_inode_flag(ac->ac_inode, EXT4_INODE_EXTENTS)) &&
		    (pa->pa_pstart + EXT4_C2B(sbi, pa->pa_len) >
		     EXT4_MAX_BLOCK_FILE_PHYS))
			continue;

		/* found preallocated blocks, use them */
		spin_lock(&pa->pa_lock);
		if (pa->pa_deleted == 0 && pa->pa_free) {
			atomic_inc(&pa->pa_count);
			ext4_mb_use_inode_pa(ac, pa);
			spin_unlock(&pa->pa_lock);
			ac->ac_criteria = 10;
			rcu_read_unlock();
			return 1;
		}
		spin_unlock(&pa->pa_lock);
	}
	rcu_read_unlock();

	/* can we use group allocation? */
	if (!(ac->ac_flags & EXT4_MB_HINT_GROUP_ALLOC))
		return 0;

	/* inode may have no locality group for some reason */
	lg = ac->ac_lg;
	if (lg == NULL)
		return 0;
	order  = fls(ac->ac_o_ex.fe_len) - 1;
	if (order > PREALLOC_TB_SIZE - 1)
		/* The max size of hash table is PREALLOC_TB_SIZE */
		order = PREALLOC_TB_SIZE - 1;

	goal_block = ext4_grp_offs_to_block(ac->ac_sb, &ac->ac_g_ex);
	/*
	 * search for the prealloc space that is having
	 * minimal distance from the goal block.
	 */
	for (i = order; i < PREALLOC_TB_SIZE; i++) {
		rcu_read_lock();
		list_for_each_entry_rcu(pa, &lg->lg_prealloc_list[i],
					pa_inode_list) {
			spin_lock(&pa->pa_lock);
			if (pa->pa_deleted == 0 &&
					pa->pa_free >= ac->ac_o_ex.fe_len) {

				cpa = ext4_mb_check_group_pa(goal_block,
								pa, cpa);
			}
			spin_unlock(&pa->pa_lock);
		}
		rcu_read_unlock();
	}
	if (cpa) {
		ext4_mb_use_group_pa(ac, cpa);
		ac->ac_criteria = 20;
		return 1;
	}
	return 0;
}

/*
 * the function goes through all block freed in the group
 * but not yet committed and marks them used in in-core bitmap.
 * buddy must be generated from this bitmap
 * Need to be called with the ext4 group lock held
 */
static void ext4_mb_generate_from_freelist(struct super_block *sb, void *bitmap,
						ext4_group_t group)
{
	struct rb_node *n;
	struct ext4_group_info *grp;
	struct ext4_free_data *entry;

	grp = ext4_get_group_info(sb, group);
	n = rb_first(&(grp->bb_free_root));

	while (n) {
		entry = rb_entry(n, struct ext4_free_data, efd_node);
		ext4_set_bits(bitmap, entry->efd_start_cluster, entry->efd_count);
		n = rb_next(n);
	}
	return;
}

/*
 * the function goes through all preallocation in this group and marks them
 * used in in-core bitmap. buddy must be generated from this bitmap
 * Need to be called with ext4 group lock held
 */
static noinline_for_stack
void ext4_mb_generate_from_pa(struct super_block *sb, void *bitmap,
					ext4_group_t group)
{
	struct ext4_group_info *grp = ext4_get_group_info(sb, group);
	struct ext4_prealloc_space *pa;
	struct list_head *cur;
	ext4_group_t groupnr;
	ext4_grpblk_t start;
	int preallocated = 0;
	int len;

	/* all form of preallocation discards first load group,
	 * so the only competing code is preallocation use.
	 * we don't need any locking here
	 * notice we do NOT ignore preallocations with pa_deleted
	 * otherwise we could leave used blocks available for
	 * allocation in buddy when concurrent ext4_mb_put_pa()
	 * is dropping preallocation
	 */
	list_for_each(cur, &grp->bb_prealloc_list) {
		pa = list_entry(cur, struct ext4_prealloc_space, pa_group_list);
		spin_lock(&pa->pa_lock);
		ext4_get_group_no_and_offset(sb, pa->pa_pstart,
					     &groupnr, &start);
		len = pa->pa_len;
		spin_unlock(&pa->pa_lock);
		if (unlikely(len == 0))
			continue;
		BUG_ON(groupnr != group);
		ext4_set_bits(bitmap, start, len);
		preallocated += len;
	}
	mb_debug(1, "preallocated %u for group %u\n", preallocated, group);
}

static void ext4_mb_pa_callback(struct rcu_head *head)
{
	struct ext4_prealloc_space *pa;
	pa = container_of(head, struct ext4_prealloc_space, u.pa_rcu);

	BUG_ON(atomic_read(&pa->pa_count));
	BUG_ON(pa->pa_deleted == 0);
	kmem_cache_free(ext4_pspace_cachep, pa);
}

/*
 * drops a reference to preallocated space descriptor
 * if this was the last reference and the space is consumed
 */
static void ext4_mb_put_pa(struct ext4_allocation_context *ac,
			struct super_block *sb, struct ext4_prealloc_space *pa)
{
	ext4_group_t grp;
	ext4_fsblk_t grp_blk;

	/* in this short window concurrent discard can set pa_deleted */
	spin_lock(&pa->pa_lock);
	if (!atomic_dec_and_test(&pa->pa_count) || pa->pa_free != 0) {
		spin_unlock(&pa->pa_lock);
		return;
	}

	if (pa->pa_deleted == 1) {
		spin_unlock(&pa->pa_lock);
		return;
	}

	pa->pa_deleted = 1;
	spin_unlock(&pa->pa_lock);

	grp_blk = pa->pa_pstart;
	/*
	 * If doing group-based preallocation, pa_pstart may be in the
	 * next group when pa is used up
	 */
	if (pa->pa_type == MB_GROUP_PA)
		grp_blk--;

	grp = ext4_get_group_number(sb, grp_blk);

	/*
	 * possible race:
	 *
	 *  P1 (buddy init)			P2 (regular allocation)
	 *					find block B in PA
	 *  copy on-disk bitmap to buddy
	 *  					mark B in on-disk bitmap
	 *					drop PA from group
	 *  mark all PAs in buddy
	 *
	 * thus, P1 initializes buddy with B available. to prevent this
	 * we make "copy" and "mark all PAs" atomic and serialize "drop PA"
	 * against that pair
	 */
	ext4_lock_group(sb, grp);
	list_del(&pa->pa_group_list);
	ext4_unlock_group(sb, grp);

	spin_lock(pa->pa_obj_lock);
	list_del_rcu(&pa->pa_inode_list);
	spin_unlock(pa->pa_obj_lock);

	call_rcu(&(pa)->u.pa_rcu, ext4_mb_pa_callback);
}

/*
 * creates new preallocated space for given inode
 */
static noinline_for_stack int
ext4_mb_new_inode_pa(struct ext4_allocation_context *ac)
{
	struct super_block *sb = ac->ac_sb;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct ext4_prealloc_space *pa;
	struct ext4_group_info *grp;
	struct ext4_inode_info *ei;

	/* preallocate only when found space is larger then requested */
	BUG_ON(ac->ac_o_ex.fe_len >= ac->ac_b_ex.fe_len);
	BUG_ON(ac->ac_status != AC_STATUS_FOUND);
	BUG_ON(!S_ISREG(ac->ac_inode->i_mode));

	pa = kmem_cache_alloc(ext4_pspace_cachep, GFP_NOFS);
	if (pa == NULL)
		return -ENOMEM;

	if (ac->ac_b_ex.fe_len < ac->ac_g_ex.fe_len) {
		int winl;
		int wins;
		int win;
		int offs;

		/* we can't allocate as much as normalizer wants.
		 * so, found space must get proper lstart
		 * to cover original request */
		BUG_ON(ac->ac_g_ex.fe_logical > ac->ac_o_ex.fe_logical);
		BUG_ON(ac->ac_g_ex.fe_len < ac->ac_o_ex.fe_len);

		/* we're limited by original request in that
		 * logical block must be covered any way
		 * winl is window we can move our chunk within */
		winl = ac->ac_o_ex.fe_logical - ac->ac_g_ex.fe_logical;

		/* also, we should cover whole original request */
		wins = EXT4_C2B(sbi, ac->ac_b_ex.fe_len - ac->ac_o_ex.fe_len);

		/* the smallest one defines real window */
		win = min(winl, wins);

		offs = ac->ac_o_ex.fe_logical %
			EXT4_C2B(sbi, ac->ac_b_ex.fe_len);
		if (offs && offs < win)
			win = offs;

		ac->ac_b_ex.fe_logical = ac->ac_o_ex.fe_logical -
			EXT4_NUM_B2C(sbi, win);
		BUG_ON(ac->ac_o_ex.fe_logical < ac->ac_b_ex.fe_logical);
		BUG_ON(ac->ac_o_ex.fe_len > ac->ac_b_ex.fe_len);
	}

	/* preallocation can change ac_b_ex, thus we store actually
	 * allocated blocks for history */
	ac->ac_f_ex = ac->ac_b_ex;

	pa->pa_lstart = ac->ac_b_ex.fe_logical;
	pa->pa_pstart = ext4_grp_offs_to_block(sb, &ac->ac_b_ex);
	pa->pa_len = ac->ac_b_ex.fe_len;
	pa->pa_free = pa->pa_len;
	atomic_set(&pa->pa_count, 1);
	spin_lock_init(&pa->pa_lock);
	INIT_LIST_HEAD(&pa->pa_inode_list);
	INIT_LIST_HEAD(&pa->pa_group_list);
	pa->pa_deleted = 0;
	pa->pa_type = MB_INODE_PA;

	mb_debug(1, "new inode pa %p: %llu/%u for %u\n", pa,
			pa->pa_pstart, pa->pa_len, pa->pa_lstart);
	trace_ext4_mb_new_inode_pa(ac, pa);

	ext4_mb_use_inode_pa(ac, pa);
	atomic_add(pa->pa_free, &sbi->s_mb_preallocated);

	ei = EXT4_I(ac->ac_inode);
	grp = ext4_get_group_info(sb, ac->ac_b_ex.fe_group);

	pa->pa_obj_lock = &ei->i_prealloc_lock;
	pa->pa_inode = ac->ac_inode;

	ext4_lock_group(sb, ac->ac_b_ex.fe_group);
	list_add(&pa->pa_group_list, &grp->bb_prealloc_list);
	ext4_unlock_group(sb, ac->ac_b_ex.fe_group);

	spin_lock(pa->pa_obj_lock);
	list_add_rcu(&pa->pa_inode_list, &ei->i_prealloc_list);
	spin_unlock(pa->pa_obj_lock);

	return 0;
}

/*
 * creates new preallocated space for locality group inodes belongs to
 */
static noinline_for_stack int
ext4_mb_new_group_pa(struct ext4_allocation_context *ac)
{
	struct super_block *sb = ac->ac_sb;
	struct ext4_locality_group *lg;
	struct ext4_prealloc_space *pa;
	struct ext4_group_info *grp;

	/* preallocate only when found space is larger then requested */
	BUG_ON(ac->ac_o_ex.fe_len >= ac->ac_b_ex.fe_len);
	BUG_ON(ac->ac_status != AC_STATUS_FOUND);
	BUG_ON(!S_ISREG(ac->ac_inode->i_mode));

	BUG_ON(ext4_pspace_cachep == NULL);
	pa = kmem_cache_alloc(ext4_pspace_cachep, GFP_NOFS);
	if (pa == NULL)
		return -ENOMEM;

	/* preallocation can change ac_b_ex, thus we store actually
	 * allocated blocks for history */
	ac->ac_f_ex = ac->ac_b_ex;

	pa->pa_pstart = ext4_grp_offs_to_block(sb, &ac->ac_b_ex);
	pa->pa_lstart = pa->pa_pstart;
	pa->pa_len = ac->ac_b_ex.fe_len;
	pa->pa_free = pa->pa_len;
	atomic_set(&pa->pa_count, 1);
	spin_lock_init(&pa->pa_lock);
	INIT_LIST_HEAD(&pa->pa_inode_list);
	INIT_LIST_HEAD(&pa->pa_group_list);
	pa->pa_deleted = 0;
	pa->pa_type = MB_GROUP_PA;

	mb_debug(1, "new group pa %p: %llu/%u for %u\n", pa,
			pa->pa_pstart, pa->pa_len, pa->pa_lstart);
	trace_ext4_mb_new_group_pa(ac, pa);

	ext4_mb_use_group_pa(ac, pa);
	atomic_add(pa->pa_free, &EXT4_SB(sb)->s_mb_preallocated);

	grp = ext4_get_group_info(sb, ac->ac_b_ex.fe_group);
	lg = ac->ac_lg;
	BUG_ON(lg == NULL);

	pa->pa_obj_lock = &lg->lg_prealloc_lock;
	pa->pa_inode = NULL;

	ext4_lock_group(sb, ac->ac_b_ex.fe_group);
	list_add(&pa->pa_group_list, &grp->bb_prealloc_list);
	ext4_unlock_group(sb, ac->ac_b_ex.fe_group);

	/*
	 * We will later add the new pa to the right bucket
	 * after updating the pa_free in ext4_mb_release_context
	 */
	return 0;
}

static int ext4_mb_new_preallocation(struct ext4_allocation_context *ac)
{
	int err;

	if (ac->ac_flags & EXT4_MB_HINT_GROUP_ALLOC)
		err = ext4_mb_new_group_pa(ac);
	else
		err = ext4_mb_new_inode_pa(ac);
	return err;
}

/*
 * finds all unused blocks in on-disk bitmap, frees them in
 * in-core bitmap and buddy.
 * @pa must be unlinked from inode and group lists, so that
 * nobody else can find/use it.
 * the caller MUST hold group/inode locks.
 * TODO: optimize the case when there are no in-core structures yet
 */
static noinline_for_stack int
ext4_mb_release_inode_pa(struct ext4_buddy *e4b, struct buffer_head *bitmap_bh,
			struct ext4_prealloc_space *pa)
{
	struct super_block *sb = e4b->bd_sb;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	unsigned int end;
	unsigned int next;
	ext4_group_t group;
	ext4_grpblk_t bit;
	unsigned long long grp_blk_start;
	int err = 0;
	int free = 0;

	BUG_ON(pa->pa_deleted == 0);
	ext4_get_group_no_and_offset(sb, pa->pa_pstart, &group, &bit);
	grp_blk_start = pa->pa_pstart - EXT4_C2B(sbi, bit);
	BUG_ON(group != e4b->bd_group && pa->pa_len != 0);
	end = bit + pa->pa_len;

	while (bit < end) {
		bit = mb_find_next_zero_bit(bitmap_bh->b_data, end, bit);
		if (bit >= end)
			break;
		next = mb_find_next_bit(bitmap_bh->b_data, end, bit);
		mb_debug(1, "    free preallocated %u/%u in group %u\n",
			 (unsigned) ext4_group_first_block_no(sb, group) + bit,
			 (unsigned) next - bit, (unsigned) group);
		free += next - bit;

		trace_ext4_mballoc_discard(sb, NULL, group, bit, next - bit);
		trace_ext4_mb_release_inode_pa(pa, (grp_blk_start +
						    EXT4_C2B(sbi, bit)),
					       next - bit);
		mb_free_blocks(pa->pa_inode, e4b, bit, next - bit);
		bit = next + 1;
	}
	if (free != pa->pa_free) {
		ext4_msg(e4b->bd_sb, KERN_CRIT,
			 "pa %p: logic %lu, phys. %lu, len %lu",
			 pa, (unsigned long) pa->pa_lstart,
			 (unsigned long) pa->pa_pstart,
			 (unsigned long) pa->pa_len);
		ext4_grp_locked_error(sb, group, 0, 0, "free %u, pa_free %u",
					free, pa->pa_free);
		/*
		 * pa is already deleted so we use the value obtained
		 * from the bitmap and continue.
		 */
	}
	atomic_add(free, &sbi->s_mb_discarded);

	return err;
}

static noinline_for_stack int
ext4_mb_release_group_pa(struct ext4_buddy *e4b,
				struct ext4_prealloc_space *pa)
{
	struct super_block *sb = e4b->bd_sb;
	ext4_group_t group;
	ext4_grpblk_t bit;

	trace_ext4_mb_release_group_pa(sb, pa);
	BUG_ON(pa->pa_deleted == 0);
	ext4_get_group_no_and_offset(sb, pa->pa_pstart, &group, &bit);
	BUG_ON(group != e4b->bd_group && pa->pa_len != 0);
	mb_free_blocks(pa->pa_inode, e4b, bit, pa->pa_len);
	atomic_add(pa->pa_len, &EXT4_SB(sb)->s_mb_discarded);
	trace_ext4_mballoc_discard(sb, NULL, group, bit, pa->pa_len);

	return 0;
}

/*
 * releases all preallocations in given group
 *
 * first, we need to decide discard policy:
 * - when do we discard
 *   1) ENOSPC
 * - how many do we discard
 *   1) how many requested
 */
static noinline_for_stack int
ext4_mb_discard_group_preallocations(struct super_block *sb,
					ext4_group_t group, int needed)
{
	struct ext4_group_info *grp = ext4_get_group_info(sb, group);
	struct buffer_head *bitmap_bh = NULL;
	struct ext4_prealloc_space *pa, *tmp;
	struct list_head list;
	struct ext4_buddy e4b;
	int err;
	int busy = 0;
	int free = 0;

	mb_debug(1, "discard preallocation for group %u\n", group);

	if (list_empty(&grp->bb_prealloc_list))
		return 0;

	bitmap_bh = ext4_read_block_bitmap(sb, group);
	if (IS_ERR(bitmap_bh)) {
		err = PTR_ERR(bitmap_bh);
		ext4_error(sb, "Error %d reading block bitmap for %u",
			   err, group);
		return 0;
	}

	err = ext4_mb_load_buddy(sb, group, &e4b);
	if (err) {
		ext4_warning(sb, "Error %d loading buddy information for %u",
			     err, group);
		put_bh(bitmap_bh);
		return 0;
	}

	if (needed == 0)
		needed = EXT4_CLUSTERS_PER_GROUP(sb) + 1;

	INIT_LIST_HEAD(&list);
repeat:
	ext4_lock_group(sb, group);
	list_for_each_entry_safe(pa, tmp,
				&grp->bb_prealloc_list, pa_group_list) {
		spin_lock(&pa->pa_lock);
		if (atomic_read(&pa->pa_count)) {
			spin_unlock(&pa->pa_lock);
			busy = 1;
			continue;
		}
		if (pa->pa_deleted) {
			spin_unlock(&pa->pa_lock);
			continue;
		}

		/* seems this one can be freed ... */
		pa->pa_deleted = 1;

		/* we can trust pa_free ... */
		free += pa->pa_free;

		spin_unlock(&pa->pa_lock);

		list_del(&pa->pa_group_list);
		list_add(&pa->u.pa_tmp_list, &list);
	}

	/* if we still need more blocks and some PAs were used, try again */
	if (free < needed && busy) {
		busy = 0;
		ext4_unlock_group(sb, group);
		cond_resched();
		goto repeat;
	}

	/* found anything to free? */
	if (list_empty(&list)) {
		BUG_ON(free != 0);
		goto out;
	}

	/* now free all selected PAs */
	list_for_each_entry_safe(pa, tmp, &list, u.pa_tmp_list) {

		/* remove from object (inode or locality group) */
		spin_lock(pa->pa_obj_lock);
		list_del_rcu(&pa->pa_inode_list);
		spin_unlock(pa->pa_obj_lock);

		if (pa->pa_type == MB_GROUP_PA)
			ext4_mb_release_group_pa(&e4b, pa);
		else
			ext4_mb_release_inode_pa(&e4b, bitmap_bh, pa);

		list_del(&pa->u.pa_tmp_list);
		call_rcu(&(pa)->u.pa_rcu, ext4_mb_pa_callback);
	}

out:
	ext4_unlock_group(sb, group);
	ext4_mb_unload_buddy(&e4b);
	put_bh(bitmap_bh);
	return free;
}

/*
 * releases all non-used preallocated blocks for given inode
 *
 * It's important to discard preallocations under i_data_sem
 * We don't want another block to be served from the prealloc
 * space when we are discarding the inode prealloc space.
 *
 * FIXME!! Make sure it is valid at all the call sites
 */
void ext4_discard_preallocations(struct inode *inode)
{
	struct ext4_inode_info *ei = EXT4_I(inode);
	struct super_block *sb = inode->i_sb;
	struct buffer_head *bitmap_bh = NULL;
	struct ext4_prealloc_space *pa, *tmp;
	ext4_group_t group = 0;
	struct list_head list;
	struct ext4_buddy e4b;
	int err;

	if (!S_ISREG(inode->i_mode)) {
		/*BUG_ON(!list_empty(&ei->i_prealloc_list));*/
		return;
	}

	mb_debug(1, "discard preallocation for inode %lu\n", inode->i_ino);
	trace_ext4_discard_preallocations(inode);

	INIT_LIST_HEAD(&list);

repeat:
	/* first, collect all pa's in the inode */
	spin_lock(&ei->i_prealloc_lock);
	while (!list_empty(&ei->i_prealloc_list)) {
		pa = list_entry(ei->i_prealloc_list.next,
				struct ext4_prealloc_space, pa_inode_list);
		BUG_ON(pa->pa_obj_lock != &ei->i_prealloc_lock);
		spin_lock(&pa->pa_lock);
		if (atomic_read(&pa->pa_count)) {
			/* this shouldn't happen often - nobody should
			 * use preallocation while we're discarding it */
			spin_unlock(&pa->pa_lock);
			spin_unlock(&ei->i_prealloc_lock);
			ext4_msg(sb, KERN_ERR,
				 "uh-oh! used pa while discarding");
			WARN_ON(1);
			schedule_timeout_uninterruptible(HZ);
			goto repeat;

		}
		if (pa->pa_deleted == 0) {
			pa->pa_deleted = 1;
			spin_unlock(&pa->pa_lock);
			list_del_rcu(&pa->pa_inode_list);
			list_add(&pa->u.pa_tmp_list, &list);
			continue;
		}

		/* someone is deleting pa right now */
		spin_unlock(&pa->pa_lock);
		spin_unlock(&ei->i_prealloc_lock);

		/* we have to wait here because pa_deleted
		 * doesn't mean pa is already unlinked from
		 * the list. as we might be called from
		 * ->clear_inode() the inode will get freed
		 * and concurrent thread which is unlinking
		 * pa from inode's list may access already
		 * freed memory, bad-bad-bad */

		/* XXX: if this happens too often, we can
		 * add a flag to force wait only in case
		 * of ->clear_inode(), but not in case of
		 * regular truncate */
		schedule_timeout_uninterruptible(HZ);
		goto repeat;
	}
	spin_unlock(&ei->i_prealloc_lock);

	list_for_each_entry_safe(pa, tmp, &list, u.pa_tmp_list) {
		BUG_ON(pa->pa_type != MB_INODE_PA);
		group = ext4_get_group_number(sb, pa->pa_pstart);

		err = ext4_mb_load_buddy_gfp(sb, group, &e4b,
					     GFP_NOFS|__GFP_NOFAIL);
		if (err) {
			ext4_error(sb, "Error %d loading buddy information for %u",
				   err, group);
			continue;
		}

		bitmap_bh = ext4_read_block_bitmap(sb, group);
		if (IS_ERR(bitmap_bh)) {
			err = PTR_ERR(bitmap_bh);
			ext4_error(sb, "Error %d reading block bitmap for %u",
					err, group);
			ext4_mb_unload_buddy(&e4b);
			continue;
		}

		ext4_lock_group(sb, group);
		list_del(&pa->pa_group_list);
		ext4_mb_release_inode_pa(&e4b, bitmap_bh, pa);
		ext4_unlock_group(sb, group);

		ext4_mb_unload_buddy(&e4b);
		put_bh(bitmap_bh);

		list_del(&pa->u.pa_tmp_list);
		call_rcu(&(pa)->u.pa_rcu, ext4_mb_pa_callback);
	}
}

#ifdef CONFIG_EXT4_DEBUG
static void ext4_mb_show_ac(struct ext4_allocation_context *ac)
{
	struct super_block *sb = ac->ac_sb;
	ext4_group_t ngroups, i;

	if (!ext4_mballoc_debug ||
	    (EXT4_SB(sb)->s_mount_flags & EXT4_MF_FS_ABORTED))
		return;

	ext4_msg(ac->ac_sb, KERN_ERR, "Can't allocate:"
			" Allocation context details:");
	ext4_msg(ac->ac_sb, KERN_ERR, "status %d flags %d",
			ac->ac_status, ac->ac_flags);
	ext4_msg(ac->ac_sb, KERN_ERR, "orig %lu/%lu/%lu@%lu, "
		 	"goal %lu/%lu/%lu@%lu, "
			"best %lu/%lu/%lu@%lu cr %d",
			(unsigned long)ac->ac_o_ex.fe_group,
			(unsigned long)ac->ac_o_ex.fe_start,
			(unsigned long)ac->ac_o_ex.fe_len,
			(unsigned long)ac->ac_o_ex.fe_logical,
			(unsigned long)ac->ac_g_ex.fe_group,
			(unsigned long)ac->ac_g_ex.fe_start,
			(unsigned long)ac->ac_g_ex.fe_len,
			(unsigned long)ac->ac_g_ex.fe_logical,
			(unsigned long)ac->ac_b_ex.fe_group,
			(unsigned long)ac->ac_b_ex.fe_start,
			(unsigned long)ac->ac_b_ex.fe_len,
			(unsigned long)ac->ac_b_ex.fe_logical,
			(int)ac->ac_criteria);
	ext4_msg(ac->ac_sb, KERN_ERR, "%d found", ac->ac_found);
	ext4_msg(ac->ac_sb, KERN_ERR, "groups: ");
	ngroups = ext4_get_groups_count(sb);
	for (i = 0; i < ngroups; i++) {
		struct ext4_group_info *grp = ext4_get_group_info(sb, i);
		struct ext4_prealloc_space *pa;
		ext4_grpblk_t start;
		struct list_head *cur;
		ext4_lock_group(sb, i);
		list_for_each(cur, &grp->bb_prealloc_list) {
			pa = list_entry(cur, struct ext4_prealloc_space,
					pa_group_list);
			spin_lock(&pa->pa_lock);
			ext4_get_group_no_and_offset(sb, pa->pa_pstart,
						     NULL, &start);
			spin_unlock(&pa->pa_lock);
			printk(KERN_ERR "PA:%u:%d:%u \n", i,
			       start, pa->pa_len);
		}
		ext4_unlock_group(sb, i);

		if (grp->bb_free == 0)
			continue;
		printk(KERN_ERR "%u: %d/%d \n",
		       i, grp->bb_free, grp->bb_fragments);
	}
	printk(KERN_ERR "\n");
}
#else
static inline void ext4_mb_show_ac(struct ext4_allocation_context *ac)
{
	return;
}
#endif

/*
 * We use locality group preallocation for small size file. The size of the
 * file is determined by the current size or the resulting size after
 * allocation which ever is larger
 *
 * One can tune this size via /sys/fs/ext4/<partition>/mb_stream_req
 */
static void ext4_mb_group_or_file(struct ext4_allocation_context *ac)
{
	struct ext4_sb_info *sbi = EXT4_SB(ac->ac_sb);
	int bsbits = ac->ac_sb->s_blocksize_bits;
	loff_t size, isize;

	if (!(ac->ac_flags & EXT4_MB_HINT_DATA))
		return;

	if (unlikely(ac->ac_flags & EXT4_MB_HINT_GOAL_ONLY))
		return;

	size = ac->ac_o_ex.fe_logical + EXT4_C2B(sbi, ac->ac_o_ex.fe_len);
	isize = (i_size_read(ac->ac_inode) + ac->ac_sb->s_blocksize - 1)
		>> bsbits;

	if ((size == isize) &&
	    !ext4_fs_is_busy(sbi) &&
	    (atomic_read(&ac->ac_inode->i_writecount) == 0)) {
		ac->ac_flags |= EXT4_MB_HINT_NOPREALLOC;
		return;
	}

	if (sbi->s_mb_group_prealloc <= 0) {
		ac->ac_flags |= EXT4_MB_STREAM_ALLOC;
		return;
	}

	/* don't use group allocation for large files */
	size = max(size, isize);
	if (size > sbi->s_mb_stream_request) {
		ac->ac_flags |= EXT4_MB_STREAM_ALLOC;
		return;
	}

	BUG_ON(ac->ac_lg != NULL);
	/*
	 * locality group prealloc space are per cpu. The reason for having
	 * per cpu locality group is to reduce the contention between block
	 * request from multiple CPUs.
	 */
	ac->ac_lg = raw_cpu_ptr(sbi->s_locality_groups);

	/* we're going to use group allocation */
	ac->ac_flags |= EXT4_MB_HINT_GROUP_ALLOC;

	/* serialize all allocations in the group */
	mutex_lock(&ac->ac_lg->lg_mutex);
}

static noinline_for_stack int
ext4_mb_initialize_context(struct ext4_allocation_context *ac,
				struct ext4_allocation_request *ar)
{
	struct super_block *sb = ar->inode->i_sb;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct ext4_super_block *es = sbi->s_es;
	ext4_group_t group;
	unsigned int len;
	ext4_fsblk_t goal;
	ext4_grpblk_t block;

	/* we can't allocate > group size */
	len = ar->len;

	/* just a dirty hack to filter too big requests  */
	if (len >= EXT4_CLUSTERS_PER_GROUP(sb))
		len = EXT4_CLUSTERS_PER_GROUP(sb);

	/* start searching from the goal */
	goal = ar->goal;
	if (goal < le32_to_cpu(es->s_first_data_block) ||
			goal >= ext4_blocks_count(es))
		goal = le32_to_cpu(es->s_first_data_block);
	ext4_get_group_no_and_offset(sb, goal, &group, &block);

	/* set up allocation goals */
	ac->ac_b_ex.fe_logical = EXT4_LBLK_CMASK(sbi, ar->logical);
	ac->ac_status = AC_STATUS_CONTINUE;
	ac->ac_sb = sb;
	ac->ac_inode = ar->inode;
	ac->ac_o_ex.fe_logical = ac->ac_b_ex.fe_logical;
	ac->ac_o_ex.fe_group = group;
	ac->ac_o_ex.fe_start = block;
	ac->ac_o_ex.fe_len = len;
	ac->ac_g_ex = ac->ac_o_ex;
	ac->ac_flags = ar->flags;

	/* we have to define context: we'll we work with a file or
	 * locality group. this is a policy, actually */
	ext4_mb_group_or_file(ac);

	mb_debug(1, "init ac: %u blocks @ %u, goal %u, flags %x, 2^%d, "
			"left: %u/%u, right %u/%u to %swritable\n",
			(unsigned) ar->len, (unsigned) ar->logical,
			(unsigned) ar->goal, ac->ac_flags, ac->ac_2order,
			(unsigned) ar->lleft, (unsigned) ar->pleft,
			(unsigned) ar->lright, (unsigned) ar->pright,
			atomic_read(&ar->inode->i_writecount) ? "" : "non-");
	return 0;

}

static noinline_for_stack void
ext4_mb_discard_lg_preallocations(struct super_block *sb,
					struct ext4_locality_group *lg,
					int order, int total_entries)
{
	ext4_group_t group = 0;
	struct ext4_buddy e4b;
	struct list_head discard_list;
	struct ext4_prealloc_space *pa, *tmp;

	mb_debug(1, "discard locality group preallocation\n");

	INIT_LIST_HEAD(&discard_list);

	spin_lock(&lg->lg_prealloc_lock);
	list_for_each_entry_rcu(pa, &lg->lg_prealloc_list[order],
						pa_inode_list) {
		spin_lock(&pa->pa_lock);
		if (atomic_read(&pa->pa_count)) {
			/*
			 * This is the pa that we just used
			 * for block allocation. So don't
			 * free that
			 */
			spin_unlock(&pa->pa_lock);
			continue;
		}
		if (pa->pa_deleted) {
			spin_unlock(&pa->pa_lock);
			continue;
		}
		/* only lg prealloc space */
		BUG_ON(pa->pa_type != MB_GROUP_PA);

		/* seems this one can be freed ... */
		pa->pa_deleted = 1;
		spin_unlock(&pa->pa_lock);

		list_del_rcu(&pa->pa_inode_list);
		list_add(&pa->u.pa_tmp_list, &discard_list);

		total_entries--;
		if (total_entries <= 5) {
			/*
			 * we want to keep only 5 entries
			 * allowing it to grow to 8. This
			 * mak sure we don't call discard
			 * soon for this list.
			 */
			break;
		}
	}
	spin_unlock(&lg->lg_prealloc_lock);

	list_for_each_entry_safe(pa, tmp, &discard_list, u.pa_tmp_list) {
		int err;

		group = ext4_get_group_number(sb, pa->pa_pstart);
		err = ext4_mb_load_buddy_gfp(sb, group, &e4b,
					     GFP_NOFS|__GFP_NOFAIL);
		if (err) {
			ext4_error(sb, "Error %d loading buddy information for %u",
				   err, group);
			continue;
		}
		ext4_lock_group(sb, group);
		list_del(&pa->pa_group_list);
		ext4_mb_release_group_pa(&e4b, pa);
		ext4_unlock_group(sb, group);

		ext4_mb_unload_buddy(&e4b);
		list_del(&pa->u.pa_tmp_list);
		call_rcu(&(pa)->u.pa_rcu, ext4_mb_pa_callback);
	}
}

/*
 * We have incremented pa_count. So it cannot be freed at this
 * point. Also we hold lg_mutex. So no parallel allocation is
 * possible from this lg. That means pa_free cannot be updated.
 *
 * A parallel ext4_mb_discard_group_preallocations is possible.
 * which can cause the lg_prealloc_list to be updated.
 */

static void ext4_mb_add_n_trim(struct ext4_allocation_context *ac)
{
	int order, added = 0, lg_prealloc_count = 1;
	struct super_block *sb = ac->ac_sb;
	struct ext4_locality_group *lg = ac->ac_lg;
	struct ext4_prealloc_space *tmp_pa, *pa = ac->ac_pa;

	order = fls(pa->pa_free) - 1;
	if (order > PREALLOC_TB_SIZE - 1)
		/* The max size of hash table is PREALLOC_TB_SIZE */
		order = PREALLOC_TB_SIZE - 1;
	/* Add the prealloc space to lg */
	spin_lock(&lg->lg_prealloc_lock);
	list_for_each_entry_rcu(tmp_pa, &lg->lg_prealloc_list[order],
						pa_inode_list) {
		spin_lock(&tmp_pa->pa_lock);
		if (tmp_pa->pa_deleted) {
			spin_unlock(&tmp_pa->pa_lock);
			continue;
		}
		if (!added && pa->pa_free < tmp_pa->pa_free) {
			/* Add to the tail of the previous entry */
			list_add_tail_rcu(&pa->pa_inode_list,
						&tmp_pa->pa_inode_list);
			added = 1;
			/*
			 * we want to count the total
			 * number of entries in the list
			 */
		}
		spin_unlock(&tmp_pa->pa_lock);
		lg_prealloc_count++;
	}
	if (!added)
		list_add_tail_rcu(&pa->pa_inode_list,
					&lg->lg_prealloc_list[order]);
	spin_unlock(&lg->lg_prealloc_lock);

	/* Now trim the list to be not more than 8 elements */
	if (lg_prealloc_count > 8) {
		ext4_mb_discard_lg_preallocations(sb, lg,
						  order, lg_prealloc_count);
		return;
	}
	return ;
}

/*
 * release all resource we used in allocation
 */
static int ext4_mb_release_context(struct ext4_allocation_context *ac)
{
	struct ext4_sb_info *sbi = EXT4_SB(ac->ac_sb);
	struct ext4_prealloc_space *pa = ac->ac_pa;
	if (pa) {
		if (pa->pa_type == MB_GROUP_PA) {
			/* see comment in ext4_mb_use_group_pa() */
			spin_lock(&pa->pa_lock);
			pa->pa_pstart += EXT4_C2B(sbi, ac->ac_b_ex.fe_len);
			pa->pa_lstart += EXT4_C2B(sbi, ac->ac_b_ex.fe_len);
			pa->pa_free -= ac->ac_b_ex.fe_len;
			pa->pa_len -= ac->ac_b_ex.fe_len;
			spin_unlock(&pa->pa_lock);
		}
	}
	if (pa) {
		/*
		 * We want to add the pa to the right bucket.
		 * Remove it from the list and while adding
		 * make sure the list to which we are adding
		 * doesn't grow big.
		 */
		if ((pa->pa_type == MB_GROUP_PA) && likely(pa->pa_free)) {
			spin_lock(pa->pa_obj_lock);
			list_del_rcu(&pa->pa_inode_list);
			spin_unlock(pa->pa_obj_lock);
			ext4_mb_add_n_trim(ac);
		}
		ext4_mb_put_pa(ac, ac->ac_sb, pa);
	}
	if (ac->ac_bitmap_page)
		put_page(ac->ac_bitmap_page);
	if (ac->ac_buddy_page)
		put_page(ac->ac_buddy_page);
	if (ac->ac_flags & EXT4_MB_HINT_GROUP_ALLOC)
		mutex_unlock(&ac->ac_lg->lg_mutex);
	ext4_mb_collect_stats(ac);
	return 0;
}

static int ext4_mb_discard_preallocations(struct super_block *sb, int needed)
{
	ext4_group_t i, ngroups = ext4_get_groups_count(sb);
	int ret;
	int freed = 0;

	trace_ext4_mb_discard_preallocations(sb, needed);
	for (i = 0; i < ngroups && needed > 0; i++) {
		ret = ext4_mb_discard_group_preallocations(sb, i, needed);
		freed += ret;
		needed -= ret;
	}

	return freed;
}

/*
 * Main entry point into mballoc to allocate blocks
 * it tries to use preallocation first, then falls back
 * to usual allocation
 */
ext4_fsblk_t ext4_mb_new_blocks(handle_t *handle,
				struct ext4_allocation_request *ar, int *errp)
{
	int freed;
	struct ext4_allocation_context *ac = NULL;
	struct ext4_sb_info *sbi;
	struct super_block *sb;
	ext4_fsblk_t block = 0;
	unsigned int inquota = 0;
	unsigned int reserv_clstrs = 0;

	might_sleep();
	sb = ar->inode->i_sb;
	sbi = EXT4_SB(sb);

	trace_ext4_request_blocks(ar);

	/* Allow to use superuser reservation for quota file */
	if (ext4_is_quota_file(ar->inode))
		ar->flags |= EXT4_MB_USE_ROOT_BLOCKS;

	if ((ar->flags & EXT4_MB_DELALLOC_RESERVED) == 0) {
		/* Without delayed allocation we need to verify
		 * there is enough free blocks to do block allocation
		 * and verify allocation doesn't exceed the quota limits.
		 */
		while (ar->len &&
			ext4_claim_free_clusters(sbi, ar->len, ar->flags)) {

			/* let others to free the space */
			cond_resched();
			ar->len = ar->len >> 1;
		}
		if (!ar->len) {
			*errp = -ENOSPC;
			return 0;
		}
		reserv_clstrs = ar->len;
		if (ar->flags & EXT4_MB_USE_ROOT_BLOCKS) {
			dquot_alloc_block_nofail(ar->inode,
						 EXT4_C2B(sbi, ar->len));
		} else {
			while (ar->len &&
				dquot_alloc_block(ar->inode,
						  EXT4_C2B(sbi, ar->len))) {

				ar->flags |= EXT4_MB_HINT_NOPREALLOC;
				ar->len--;
			}
		}
		inquota = ar->len;
		if (ar->len == 0) {
			*errp = -EDQUOT;
			goto out;
		}
	}

	ac = kmem_cache_zalloc(ext4_ac_cachep, GFP_NOFS);
	if (!ac) {
		ar->len = 0;
		*errp = -ENOMEM;
		goto out;
	}

	*errp = ext4_mb_initialize_context(ac, ar);
	if (*errp) {
		ar->len = 0;
		goto out;
	}

	ac->ac_op = EXT4_MB_HISTORY_PREALLOC;
	if (!ext4_mb_use_preallocated(ac)) {
		ac->ac_op = EXT4_MB_HISTORY_ALLOC;
		ext4_mb_normalize_request(ac, ar);
repeat:
		/* allocate space in core */
		*errp = ext4_mb_regular_allocator(ac);
		if (*errp)
			goto discard_and_exit;

		/* as we've just preallocated more space than
		 * user requested originally, we store allocated
		 * space in a special descriptor */
		if (ac->ac_status == AC_STATUS_FOUND &&
		    ac->ac_o_ex.fe_len < ac->ac_b_ex.fe_len)
			*errp = ext4_mb_new_preallocation(ac);
		if (*errp) {
		discard_and_exit:
			ext4_discard_allocated_blocks(ac);
			goto errout;
		}
	}
	if (likely(ac->ac_status == AC_STATUS_FOUND)) {
		*errp = ext4_mb_mark_diskspace_used(ac, handle, reserv_clstrs);
		if (*errp) {
			ext4_discard_allocated_blocks(ac);
			goto errout;
		} else {
			block = ext4_grp_offs_to_block(sb, &ac->ac_b_ex);
			ar->len = ac->ac_b_ex.fe_len;
		}
	} else {
		freed  = ext4_mb_discard_preallocations(sb, ac->ac_o_ex.fe_len);
		if (freed)
			goto repeat;
		*errp = -ENOSPC;
	}

errout:
	if (*errp) {
		ac->ac_b_ex.fe_len = 0;
		ar->len = 0;
		ext4_mb_show_ac(ac);
	}
	ext4_mb_release_context(ac);
out:
	if (ac)
		kmem_cache_free(ext4_ac_cachep, ac);
	if (inquota && ar->len < inquota)
		dquot_free_block(ar->inode, EXT4_C2B(sbi, inquota - ar->len));
	if (!ar->len) {
		if ((ar->flags & EXT4_MB_DELALLOC_RESERVED) == 0)
			/* release all the reserved blocks if non delalloc */
			percpu_counter_sub(&sbi->s_dirtyclusters_counter,
						reserv_clstrs);
	}

	trace_ext4_allocate_blocks(ar, (unsigned long long)block);

	return block;
}

/*
 * We can merge two free data extents only if the physical blocks
 * are contiguous, AND the extents were freed by the same transaction,
 * AND the blocks are associated with the same group.
 */
static void ext4_try_merge_freed_extent(struct ext4_sb_info *sbi,
					struct ext4_free_data *entry,
					struct ext4_free_data *new_entry,
					struct rb_root *entry_rb_root)
{
	if ((entry->efd_tid != new_entry->efd_tid) ||
	    (entry->efd_group != new_entry->efd_group))
		return;
	if (entry->efd_start_cluster + entry->efd_count ==
	    new_entry->efd_start_cluster) {
		new_entry->efd_start_cluster = entry->efd_start_cluster;
		new_entry->efd_count += entry->efd_count;
	} else if (new_entry->efd_start_cluster + new_entry->efd_count ==
		   entry->efd_start_cluster) {
		new_entry->efd_count += entry->efd_count;
	} else
		return;
	spin_lock(&sbi->s_md_lock);
	list_del(&entry->efd_list);
	spin_unlock(&sbi->s_md_lock);
	rb_erase(&entry->efd_node, entry_rb_root);
	kmem_cache_free(ext4_free_data_cachep, entry);
}

static noinline_for_stack int
ext4_mb_free_metadata(handle_t *handle, struct ext4_buddy *e4b,
		      struct ext4_free_data *new_entry)
{
	ext4_group_t group = e4b->bd_group;
	ext4_grpblk_t cluster;
	ext4_grpblk_t clusters = new_entry->efd_count;
	struct ext4_free_data *entry;
	struct ext4_group_info *db = e4b->bd_info;
	struct super_block *sb = e4b->bd_sb;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct rb_node **n = &db->bb_free_root.rb_node, *node;
	struct rb_node *parent = NULL, *new_node;

	BUG_ON(!ext4_handle_valid(handle));
	BUG_ON(e4b->bd_bitmap_page == NULL);
	BUG_ON(e4b->bd_buddy_page == NULL);

	new_node = &new_entry->efd_node;
	cluster = new_entry->efd_start_cluster;

	if (!*n) {
		/* first free block exent. We need to
		   protect buddy cache from being freed,
		 * otherwise we'll refresh it from
		 * on-disk bitmap and lose not-yet-available
		 * blocks */
		get_page(e4b->bd_buddy_page);
		get_page(e4b->bd_bitmap_page);
	}
	while (*n) {
		parent = *n;
		entry = rb_entry(parent, struct ext4_free_data, efd_node);
		if (cluster < entry->efd_start_cluster)
			n = &(*n)->rb_left;
		else if (cluster >= (entry->efd_start_cluster + entry->efd_count))
			n = &(*n)->rb_right;
		else {
			ext4_grp_locked_error(sb, group, 0,
				ext4_group_first_block_no(sb, group) +
				EXT4_C2B(sbi, cluster),
				"Block already on to-be-freed list");
			return 0;
		}
	}

	rb_link_node(new_node, parent, n);
	rb_insert_color(new_node, &db->bb_free_root);

	/* Now try to see the extent can be merged to left and right */
	node = rb_prev(new_node);
	if (node) {
		entry = rb_entry(node, struct ext4_free_data, efd_node);
		ext4_try_merge_freed_extent(sbi, entry, new_entry,
					    &(db->bb_free_root));
	}

	node = rb_next(new_node);
	if (node) {
		entry = rb_entry(node, struct ext4_free_data, efd_node);
		ext4_try_merge_freed_extent(sbi, entry, new_entry,
					    &(db->bb_free_root));
	}

	spin_lock(&sbi->s_md_lock);
	list_add_tail(&new_entry->efd_list, &sbi->s_freed_data_list);
	sbi->s_mb_free_pending += clusters;
	spin_unlock(&sbi->s_md_lock);
	return 0;
}

/**
 * ext4_free_blocks() -- Free given blocks and update quota
 * @handle:		handle for this transaction
 * @inode:		inode
 * @block:		start physical block to free
 * @count:		number of blocks to count
 * @flags:		flags used by ext4_free_blocks
 */
void ext4_free_blocks(handle_t *handle, struct inode *inode,
		      struct buffer_head *bh, ext4_fsblk_t block,
		      unsigned long count, int flags)
{
	struct buffer_head *bitmap_bh = NULL;
	struct super_block *sb = inode->i_sb;
	struct ext4_group_desc *gdp;
	unsigned int overflow;
	ext4_grpblk_t bit;
	struct buffer_head *gd_bh;
	ext4_group_t block_group;
	struct ext4_sb_info *sbi;
	struct ext4_buddy e4b;
	unsigned int count_clusters;
	int err = 0;
	int ret;

	might_sleep();
	if (bh) {
		if (block)
			BUG_ON(block != bh->b_blocknr);
		else
			block = bh->b_blocknr;
	}

	sbi = EXT4_SB(sb);
	if (!(flags & EXT4_FREE_BLOCKS_VALIDATED) &&
	    !ext4_data_block_valid(sbi, block, count)) {
		ext4_error(sb, "Freeing blocks not in datazone - "
			   "block = %llu, count = %lu", block, count);
		goto error_return;
	}

	ext4_debug("freeing block %llu\n", block);
	trace_ext4_free_blocks(inode, block, count, flags);

	if (bh && (flags & EXT4_FREE_BLOCKS_FORGET)) {
		BUG_ON(count > 1);

		ext4_forget(handle, flags & EXT4_FREE_BLOCKS_METADATA,
			    inode, bh, block);
	}

	/*
	 * If the extent to be freed does not begin on a cluster
	 * boundary, we need to deal with partial clusters at the
	 * beginning and end of the extent.  Normally we will free
	 * blocks at the beginning or the end unless we are explicitly
	 * requested to avoid doing so.
	 */
	overflow = EXT4_PBLK_COFF(sbi, block);
	if (overflow) {
		if (flags & EXT4_FREE_BLOCKS_NOFREE_FIRST_CLUSTER) {
			overflow = sbi->s_cluster_ratio - overflow;
			block += overflow;
			if (count > overflow)
				count -= overflow;
			else
				return;
		} else {
			block -= overflow;
			count += overflow;
		}
	}
	overflow = EXT4_LBLK_COFF(sbi, count);
	if (overflow) {
		if (flags & EXT4_FREE_BLOCKS_NOFREE_LAST_CLUSTER) {
			if (count > overflow)
				count -= overflow;
			else
				return;
		} else
			count += sbi->s_cluster_ratio - overflow;
	}

	if (!bh && (flags & EXT4_FREE_BLOCKS_FORGET)) {
		int i;
		int is_metadata = flags & EXT4_FREE_BLOCKS_METADATA;

		for (i = 0; i < count; i++) {
			cond_resched();
			if (is_metadata)
				bh = sb_find_get_block(inode->i_sb, block + i);
			ext4_forget(handle, is_metadata, inode, bh, block + i);
		}
	}

do_more:
	overflow = 0;
	ext4_get_group_no_and_offset(sb, block, &block_group, &bit);

	if (unlikely(EXT4_MB_GRP_BBITMAP_CORRUPT(
			ext4_get_group_info(sb, block_group))))
		return;

	/*
	 * Check to see if we are freeing blocks across a group
	 * boundary.
	 */
	if (EXT4_C2B(sbi, bit) + count > EXT4_BLOCKS_PER_GROUP(sb)) {
		overflow = EXT4_C2B(sbi, bit) + count -
			EXT4_BLOCKS_PER_GROUP(sb);
		count -= overflow;
	}
	count_clusters = EXT4_NUM_B2C(sbi, count);
	bitmap_bh = ext4_read_block_bitmap(sb, block_group);
	if (IS_ERR(bitmap_bh)) {
		err = PTR_ERR(bitmap_bh);
		bitmap_bh = NULL;
		goto error_return;
	}
	gdp = ext4_get_group_desc(sb, block_group, &gd_bh);
	if (!gdp) {
		err = -EIO;
		goto error_return;
	}

	if (in_range(ext4_block_bitmap(sb, gdp), block, count) ||
	    in_range(ext4_inode_bitmap(sb, gdp), block, count) ||
	    in_range(block, ext4_inode_table(sb, gdp),
		     sbi->s_itb_per_group) ||
	    in_range(block + count - 1, ext4_inode_table(sb, gdp),
		     sbi->s_itb_per_group)) {

		ext4_error(sb, "Freeing blocks in system zone - "
			   "Block = %llu, count = %lu", block, count);
		/* err = 0. ext4_std_error should be a no op */
		goto error_return;
	}

	BUFFER_TRACE(bitmap_bh, "getting write access");
	err = ext4_journal_get_write_access(handle, bitmap_bh);
	if (err)
		goto error_return;

	/*
	 * We are about to modify some metadata.  Call the journal APIs
	 * to unshare ->b_data if a currently-committing transaction is
	 * using it
	 */
	BUFFER_TRACE(gd_bh, "get_write_access");
	err = ext4_journal_get_write_access(handle, gd_bh);
	if (err)
		goto error_return;
#ifdef AGGRESSIVE_CHECK
	{
		int i;
		for (i = 0; i < count_clusters; i++)
			BUG_ON(!mb_test_bit(bit + i, bitmap_bh->b_data));
	}
#endif
	trace_ext4_mballoc_free(sb, inode, block_group, bit, count_clusters);

	/* __GFP_NOFAIL: retry infinitely, ignore TIF_MEMDIE and memcg limit. */
	err = ext4_mb_load_buddy_gfp(sb, block_group, &e4b,
				     GFP_NOFS|__GFP_NOFAIL);
	if (err)
		goto error_return;

	/*
	 * We need to make sure we don't reuse the freed block until after the
	 * transaction is committed. We make an exception if the inode is to be
	 * written in writeback mode since writeback mode has weak data
	 * consistency guarantees.
	 */
	if (ext4_handle_valid(handle) &&
	    ((flags & EXT4_FREE_BLOCKS_METADATA) ||
	     !ext4_should_writeback_data(inode))) {
		struct ext4_free_data *new_entry;
		/*
		 * We use __GFP_NOFAIL because ext4_free_blocks() is not allowed
		 * to fail.
		 */
		new_entry = kmem_cache_alloc(ext4_free_data_cachep,
				GFP_NOFS|__GFP_NOFAIL);
		new_entry->efd_start_cluster = bit;
		new_entry->efd_group = block_group;
		new_entry->efd_count = count_clusters;
		new_entry->efd_tid = handle->h_transaction->t_tid;

		ext4_lock_group(sb, block_group);
		mb_clear_bits(bitmap_bh->b_data, bit, count_clusters);
		ext4_mb_free_metadata(handle, &e4b, new_entry);
	} else {
		/* need to update group_info->bb_free and bitmap
		 * with group lock held. generate_buddy look at
		 * them with group lock_held
		 */
		if (test_opt(sb, DISCARD)) {
			err = ext4_issue_discard(sb, block_group, bit, count,
						 NULL);
			if (err && err != -EOPNOTSUPP)
				ext4_msg(sb, KERN_WARNING, "discard request in"
					 " group:%d block:%d count:%lu failed"
					 " with %d", block_group, bit, count,
					 err);
		} else
			EXT4_MB_GRP_CLEAR_TRIMMED(e4b.bd_info);

		ext4_lock_group(sb, block_group);
		mb_clear_bits(bitmap_bh->b_data, bit, count_clusters);
		mb_free_blocks(inode, &e4b, bit, count_clusters);
	}

	ret = ext4_free_group_clusters(sb, gdp) + count_clusters;
	ext4_free_group_clusters_set(sb, gdp, ret);
	ext4_block_bitmap_csum_set(sb, block_group, gdp, bitmap_bh);
	ext4_group_desc_csum_set(sb, block_group, gdp);
	ext4_unlock_group(sb, block_group);

	if (sbi->s_log_groups_per_flex) {
		ext4_group_t flex_group = ext4_flex_group(sbi, block_group);
		atomic64_add(count_clusters,
			     &sbi->s_flex_groups[flex_group].free_clusters);
	}

	if (!(flags & EXT4_FREE_BLOCKS_NO_QUOT_UPDATE))
		dquot_free_block(inode, EXT4_C2B(sbi, count_clusters));
	percpu_counter_add(&sbi->s_freeclusters_counter, count_clusters);

	ext4_mb_unload_buddy(&e4b);

	/* We dirtied the bitmap block */
	BUFFER_TRACE(bitmap_bh, "dirtied bitmap block");
	err = ext4_handle_dirty_metadata(handle, NULL, bitmap_bh);

	/* And the group descriptor block */
	BUFFER_TRACE(gd_bh, "dirtied group descriptor block");
	ret = ext4_handle_dirty_metadata(handle, NULL, gd_bh);
	if (!err)
		err = ret;

	if (overflow && !err) {
		block += count;
		count = overflow;
		put_bh(bitmap_bh);
		goto do_more;
	}
error_return:
	brelse(bitmap_bh);
	ext4_std_error(sb, err);
	return;
}

/**
 * ext4_group_add_blocks() -- Add given blocks to an existing group
 * @handle:			handle to this transaction
 * @sb:				super block
 * @block:			start physical block to add to the block group
 * @count:			number of blocks to free
 *
 * This marks the blocks as free in the bitmap and buddy.
 */
int ext4_group_add_blocks(handle_t *handle, struct super_block *sb,
			 ext4_fsblk_t block, unsigned long count)
{
	struct buffer_head *bitmap_bh = NULL;
	struct buffer_head *gd_bh;
	ext4_group_t block_group;
	ext4_grpblk_t bit;
	unsigned int i;
	struct ext4_group_desc *desc;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	struct ext4_buddy e4b;
	int err = 0, ret, free_clusters_count;
	ext4_grpblk_t clusters_freed;
	ext4_fsblk_t first_cluster = EXT4_B2C(sbi, block);
	ext4_fsblk_t last_cluster = EXT4_B2C(sbi, block + count - 1);
	unsigned long cluster_count = last_cluster - first_cluster + 1;

	ext4_debug("Adding block(s) %llu-%llu\n", block, block + count - 1);

	if (count == 0)
		return 0;

	ext4_get_group_no_and_offset(sb, block, &block_group, &bit);
	/*
	 * Check to see if we are freeing blocks across a group
	 * boundary.
	 */
	if (bit + cluster_count > EXT4_CLUSTERS_PER_GROUP(sb)) {
		ext4_warning(sb, "too many blocks added to group %u",
			     block_group);
		err = -EINVAL;
		goto error_return;
	}

	bitmap_bh = ext4_read_block_bitmap(sb, block_group);
	if (IS_ERR(bitmap_bh)) {
		err = PTR_ERR(bitmap_bh);
		bitmap_bh = NULL;
		goto error_return;
	}

	desc = ext4_get_group_desc(sb, block_group, &gd_bh);
	if (!desc) {
		err = -EIO;
		goto error_return;
	}

	if (in_range(ext4_block_bitmap(sb, desc), block, count) ||
	    in_range(ext4_inode_bitmap(sb, desc), block, count) ||
	    in_range(block, ext4_inode_table(sb, desc), sbi->s_itb_per_group) ||
	    in_range(block + count - 1, ext4_inode_table(sb, desc),
		     sbi->s_itb_per_group)) {
		ext4_error(sb, "Adding blocks in system zones - "
			   "Block = %llu, count = %lu",
			   block, count);
		err = -EINVAL;
		goto error_return;
	}

	BUFFER_TRACE(bitmap_bh, "getting write access");
	err = ext4_journal_get_write_access(handle, bitmap_bh);
	if (err)
		goto error_return;

	/*
	 * We are about to modify some metadata.  Call the journal APIs
	 * to unshare ->b_data if a currently-committing transaction is
	 * using it
	 */
	BUFFER_TRACE(gd_bh, "get_write_access");
	err = ext4_journal_get_write_access(handle, gd_bh);
	if (err)
		goto error_return;

	for (i = 0, clusters_freed = 0; i < cluster_count; i++) {
		BUFFER_TRACE(bitmap_bh, "clear bit");
		if (!mb_test_bit(bit + i, bitmap_bh->b_data)) {
			ext4_error(sb, "bit already cleared for block %llu",
				   (ext4_fsblk_t)(block + i));
			BUFFER_TRACE(bitmap_bh, "bit already cleared");
		} else {
			clusters_freed++;
		}
	}

	err = ext4_mb_load_buddy(sb, block_group, &e4b);
	if (err)
		goto error_return;

	/*
	 * need to update group_info->bb_free and bitmap
	 * with group lock held. generate_buddy look at
	 * them with group lock_held
	 */
	ext4_lock_group(sb, block_group);
	mb_clear_bits(bitmap_bh->b_data, bit, cluster_count);
	mb_free_blocks(NULL, &e4b, bit, cluster_count);
	free_clusters_count = clusters_freed +
		ext4_free_group_clusters(sb, desc);
	ext4_free_group_clusters_set(sb, desc, free_clusters_count);
	ext4_block_bitmap_csum_set(sb, block_group, desc, bitmap_bh);
	ext4_group_desc_csum_set(sb, block_group, desc);
	ext4_unlock_group(sb, block_group);
	percpu_counter_add(&sbi->s_freeclusters_counter,
			   clusters_freed);

	if (sbi->s_log_groups_per_flex) {
		ext4_group_t flex_group = ext4_flex_group(sbi, block_group);
		atomic64_add(clusters_freed,
			     &sbi->s_flex_groups[flex_group].free_clusters);
	}

	ext4_mb_unload_buddy(&e4b);

	/* We dirtied the bitmap block */
	BUFFER_TRACE(bitmap_bh, "dirtied bitmap block");
	err = ext4_handle_dirty_metadata(handle, NULL, bitmap_bh);

	/* And the group descriptor block */
	BUFFER_TRACE(gd_bh, "dirtied group descriptor block");
	ret = ext4_handle_dirty_metadata(handle, NULL, gd_bh);
	if (!err)
		err = ret;

error_return:
	brelse(bitmap_bh);
	ext4_std_error(sb, err);
	return err;
}

/**
 * ext4_trim_extent -- function to TRIM one single free extent in the group
 * @sb:		super block for the file system
 * @start:	starting block of the free extent in the alloc. group
 * @count:	number of blocks to TRIM
 * @group:	alloc. group we are working with
 * @e4b:	ext4 buddy for the group
 *
 * Trim "count" blocks starting at "start" in the "group". To assure that no
 * one will allocate those blocks, mark it as used in buddy bitmap. This must
 * be called with under the group lock.
 */
static int ext4_trim_extent(struct super_block *sb, int start, int count,
			     ext4_group_t group, struct ext4_buddy *e4b)
__releases(bitlock)
__acquires(bitlock)
{
	struct ext4_free_extent ex;
	int ret = 0;

	trace_ext4_trim_extent(sb, group, start, count);

	assert_spin_locked(ext4_group_lock_ptr(sb, group));

	ex.fe_start = start;
	ex.fe_group = group;
	ex.fe_len = count;

	/*
	 * Mark blocks used, so no one can reuse them while
	 * being trimmed.
	 */
	mb_mark_used(e4b, &ex);
	ext4_unlock_group(sb, group);
	ret = ext4_issue_discard(sb, group, start, count, NULL);
	ext4_lock_group(sb, group);
	mb_free_blocks(NULL, e4b, start, ex.fe_len);
	return ret;
}

/**
 * ext4_trim_all_free -- function to trim all free space in alloc. group
 * @sb:			super block for file system
 * @group:		group to be trimmed
 * @start:		first group block to examine
 * @max:		last group block to examine
 * @minblocks:		minimum extent block count
 *
 * ext4_trim_all_free walks through group's buddy bitmap searching for free
 * extents. When the free block is found, ext4_trim_extent is called to TRIM
 * the extent.
 *
 *
 * ext4_trim_all_free walks through group's block bitmap searching for free
 * extents. When the free extent is found, mark it as used in group buddy
 * bitmap. Then issue a TRIM command on this extent and free the extent in
 * the group buddy bitmap. This is done until whole group is scanned.
 */
static ext4_grpblk_t
ext4_trim_all_free(struct super_block *sb, ext4_group_t group,
		   ext4_grpblk_t start, ext4_grpblk_t max,
		   ext4_grpblk_t minblocks)
{
	void *bitmap;
	ext4_grpblk_t next, count = 0, free_count = 0;
	struct ext4_buddy e4b;
	int ret = 0;

	trace_ext4_trim_all_free(sb, group, start, max);

	ret = ext4_mb_load_buddy(sb, group, &e4b);
	if (ret) {
		ext4_warning(sb, "Error %d loading buddy information for %u",
			     ret, group);
		return ret;
	}
	bitmap = e4b.bd_bitmap;

	ext4_lock_group(sb, group);
	if (EXT4_MB_GRP_WAS_TRIMMED(e4b.bd_info) &&
	    minblocks >= atomic_read(&EXT4_SB(sb)->s_last_trim_minblks))
		goto out;

	start = (e4b.bd_info->bb_first_free > start) ?
		e4b.bd_info->bb_first_free : start;

	while (start <= max) {
		start = mb_find_next_zero_bit(bitmap, max + 1, start);
		if (start > max)
			break;
		next = mb_find_next_bit(bitmap, max + 1, start);

		if ((next - start) >= minblocks) {
			ret = ext4_trim_extent(sb, start,
					       next - start, group, &e4b);
			if (ret && ret != -EOPNOTSUPP)
				break;
			ret = 0;
			count += next - start;
		}
		free_count += next - start;
		start = next + 1;

		if (fatal_signal_pending(current)) {
			count = -ERESTARTSYS;
			break;
		}

		if (need_resched()) {
			ext4_unlock_group(sb, group);
			cond_resched();
			ext4_lock_group(sb, group);
		}

		if ((e4b.bd_info->bb_free - free_count) < minblocks)
			break;
	}

	if (!ret) {
		ret = count;
		EXT4_MB_GRP_SET_TRIMMED(e4b.bd_info);
	}
out:
	ext4_unlock_group(sb, group);
	ext4_mb_unload_buddy(&e4b);

	ext4_debug("trimmed %d blocks in the group %d\n",
		count, group);

	return ret;
}

/**
 * ext4_trim_fs() -- trim ioctl handle function
 * @sb:			superblock for filesystem
 * @range:		fstrim_range structure
 *
 * start:	First Byte to trim
 * len:		number of Bytes to trim from start
 * minlen:	minimum extent length in Bytes
 * ext4_trim_fs goes through all allocation groups containing Bytes from
 * start to start+len. For each such a group ext4_trim_all_free function
 * is invoked to trim all free space.
 */
int ext4_trim_fs(struct super_block *sb, struct fstrim_range *range)
{
	struct ext4_group_info *grp;
	ext4_group_t group, first_group, last_group;
	ext4_grpblk_t cnt = 0, first_cluster, last_cluster;
	uint64_t start, end, minlen, trimmed = 0;
	ext4_fsblk_t first_data_blk =
			le32_to_cpu(EXT4_SB(sb)->s_es->s_first_data_block);
	ext4_fsblk_t max_blks = ext4_blocks_count(EXT4_SB(sb)->s_es);
	int ret = 0;

	start = range->start >> sb->s_blocksize_bits;
	end = start + (range->len >> sb->s_blocksize_bits) - 1;
	minlen = EXT4_NUM_B2C(EXT4_SB(sb),
			      range->minlen >> sb->s_blocksize_bits);

	if (minlen > EXT4_CLUSTERS_PER_GROUP(sb) ||
	    start >= max_blks ||
	    range->len < sb->s_blocksize)
		return -EINVAL;
	if (end >= max_blks)
		end = max_blks - 1;
	if (end <= first_data_blk)
		goto out;
	if (start < first_data_blk)
		start = first_data_blk;

	/* Determine first and last group to examine based on start and end */
	ext4_get_group_no_and_offset(sb, (ext4_fsblk_t) start,
				     &first_group, &first_cluster);
	ext4_get_group_no_and_offset(sb, (ext4_fsblk_t) end,
				     &last_group, &last_cluster);

	/* end now represents the last cluster to discard in this group */
	end = EXT4_CLUSTERS_PER_GROUP(sb) - 1;

	for (group = first_group; group <= last_group; group++) {
		grp = ext4_get_group_info(sb, group);
		/* We only do this if the grp has never been initialized */
		if (unlikely(EXT4_MB_GRP_NEED_INIT(grp))) {
			ret = ext4_mb_init_group(sb, group, GFP_NOFS);
			if (ret)
				break;
		}

		/*
		 * For all the groups except the last one, last cluster will
		 * always be EXT4_CLUSTERS_PER_GROUP(sb)-1, so we only need to
		 * change it for the last group, note that last_cluster is
		 * already computed earlier by ext4_get_group_no_and_offset()
		 */
		if (group == last_group)
			end = last_cluster;

		if (grp->bb_free >= minlen) {
			cnt = ext4_trim_all_free(sb, group, first_cluster,
						end, minlen);
			if (cnt < 0) {
				ret = cnt;
				break;
			}
			trimmed += cnt;
		}

		/*
		 * For every group except the first one, we are sure
		 * that the first cluster to discard will be cluster #0.
		 */
		first_cluster = 0;
	}

	if (!ret)
		atomic_set(&EXT4_SB(sb)->s_last_trim_minblks, minlen);

out:
	range->len = EXT4_C2B(EXT4_SB(sb), trimmed) << sb->s_blocksize_bits;
	return ret;
}

/* Iterate all the free extents in the group. */
int
ext4_mballoc_query_range(
	struct super_block		*sb,
	ext4_group_t			group,
	ext4_grpblk_t			start,
	ext4_grpblk_t			end,
	ext4_mballoc_query_range_fn	formatter,
	void				*priv)
{
	void				*bitmap;
	ext4_grpblk_t			next;
	struct ext4_buddy		e4b;
	int				error;

	error = ext4_mb_load_buddy(sb, group, &e4b);
	if (error)
		return error;
	bitmap = e4b.bd_bitmap;

	ext4_lock_group(sb, group);

	start = (e4b.bd_info->bb_first_free > start) ?
		e4b.bd_info->bb_first_free : start;
	if (end >= EXT4_CLUSTERS_PER_GROUP(sb))
		end = EXT4_CLUSTERS_PER_GROUP(sb) - 1;

	while (start <= end) {
		start = mb_find_next_zero_bit(bitmap, end + 1, start);
		if (start > end)
			break;
		next = mb_find_next_bit(bitmap, end + 1, start);

		ext4_unlock_group(sb, group);
		error = formatter(sb, group, start, next - start, priv);
		if (error)
			goto out_unload;
		ext4_lock_group(sb, group);

		start = next + 1;
	}

	ext4_unlock_group(sb, group);
out_unload:
	ext4_mb_unload_buddy(&e4b);

	return error;
}
		for (i = 0; i < ac->ac_b_ex.fe_len; i++) {
			BUG_ON(mb_test_bit(ac->ac_b_ex.fe_start + i,
						bitmap_bh->b_data));
		}
	}
#endif
	ext4_set_bits(bitmap_bh->b_data, ac->ac_b_ex.fe_start,
		      ac->ac_b_ex.fe_len);
	if (ext4_has_group_desc_csum(sb) &&
	    (gdp->bg_flags & cpu_to_le16(EXT4_BG_BLOCK_UNINIT))) {
		gdp->bg_flags &= cpu_to_le16(~EXT4_BG_BLOCK_UNINIT);
		ext4_free_group_clusters_set(sb, gdp,
					     ext4_free_clusters_after_init(sb,
						ac->ac_b_ex.fe_group, gdp));
	}