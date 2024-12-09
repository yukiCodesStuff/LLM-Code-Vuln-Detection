	u64 hdr;
	enum irdma_page_size page_size;

	if (!info->total_len && !info->all_memory)
		return -EINVAL;

	if (info->page_size == 0x40000000)
		page_size = IRDMA_PAGE_SIZE_1G;
	else if (info->page_size == 0x200000)
		page_size = IRDMA_PAGE_SIZE_2M;
	u8 addr_type;
	enum irdma_page_size page_size;

	if (!info->total_len && !info->all_memory)
		return -EINVAL;

	if (info->page_size == 0x40000000)
		page_size = IRDMA_PAGE_SIZE_1G;
	else if (info->page_size == 0x200000)
		page_size = IRDMA_PAGE_SIZE_2M;