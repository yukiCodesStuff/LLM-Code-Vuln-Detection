	if (unlikely(!skb))
		return NULL;

	page = page_pool_dev_alloc_pages(queue->page_pool);
	if (unlikely(!page)) {
		kfree_skb(skb);
		return NULL;
	}