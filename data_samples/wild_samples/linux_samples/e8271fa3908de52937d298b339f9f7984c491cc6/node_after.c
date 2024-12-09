{
	struct f2fs_inode *src, *dst;
	nid_t ino = ino_of_node(page);
	struct node_info old_ni, new_ni;
	struct page *ipage;

	get_node_info(sbi, ino, &old_ni);

	if (unlikely(old_ni.blk_addr != NULL_ADDR))
		return -EINVAL;

	ipage = grab_cache_page(NODE_MAPPING(sbi), ino);
	if (!ipage)
		return -ENOMEM;

	/* Should not use this inode  from free nid list */
	remove_free_nid(NM_I(sbi), ino);

	SetPageUptodate(ipage);
	fill_node_footer(ipage, ino, ino, 0, true);

	src = F2FS_INODE(page);
	dst = F2FS_INODE(ipage);

	memcpy(dst, src, (unsigned long)&src->i_ext - (unsigned long)src);
	dst->i_size = 0;
	dst->i_blocks = cpu_to_le64(1);
	dst->i_links = cpu_to_le32(1);
	dst->i_xattr_nid = 0;

	new_ni = old_ni;
	new_ni.ino = ino;

	if (unlikely(!inc_valid_node_count(sbi, NULL)))
		WARN_ON(1);
	set_node_addr(sbi, &new_ni, NEW_ADDR, false);
	inc_valid_inode_count(sbi);
	f2fs_put_page(ipage, 1);
	return 0;
}

/*
 * ra_sum_pages() merge contiguous pages into one bio and submit.
 * these pre-readed pages are linked in pages list.
 */
static int ra_sum_pages(struct f2fs_sb_info *sbi, struct list_head *pages,
				int start, int nrpages)
{
	struct page *page;
	int page_idx = start;
	struct f2fs_io_info fio = {
		.type = META,
		.rw = READ_SYNC | REQ_META | REQ_PRIO
	};

	for (; page_idx < start + nrpages; page_idx++) {
		/* alloc temporal page for read node summary info*/
		page = alloc_page(GFP_F2FS_ZERO);
		if (!page)
			break;

		lock_page(page);
		page->index = page_idx;
		list_add_tail(&page->lru, pages);
	}

	list_for_each_entry(page, pages, lru)
		f2fs_submit_page_mbio(sbi, page, page->index, &fio);

	f2fs_submit_merged_bio(sbi, META, READ);

	return page_idx - start;
}