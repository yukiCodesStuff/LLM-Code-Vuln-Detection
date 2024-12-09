{
	if (size > XEN_NETBK_MAX_HASH_MAPPING_SIZE)
		return XEN_NETIF_CTRL_STATUS_INVALID_PARAMETER;

	vif->hash.size = size;
	memset(vif->hash.mapping, 0, sizeof(u32) * size);

	return XEN_NETIF_CTRL_STATUS_SUCCESS;
}

u32 xenvif_set_hash_mapping(struct xenvif *vif, u32 gref, u32 len,
			    u32 off)
{
	u32 *mapping = &vif->hash.mapping[off];
	struct gnttab_copy copy_op = {
		.source.u.ref = gref,
		.source.domid = vif->domid,
		.dest.u.gmfn = virt_to_gfn(mapping),
		.dest.domid = DOMID_SELF,
		.dest.offset = xen_offset_in_page(mapping),
		.len = len * sizeof(u32),
		.flags = GNTCOPY_source_gref
	};

	if ((off + len > vif->hash.size) || copy_op.len > XEN_PAGE_SIZE)
		return XEN_NETIF_CTRL_STATUS_INVALID_PARAMETER;

	while (len-- != 0)
		if (mapping[off++] >= vif->num_queues)
			return XEN_NETIF_CTRL_STATUS_INVALID_PARAMETER;

	if (copy_op.len != 0) {
		gnttab_batch_copy(&copy_op, 1);

		if (copy_op.status != GNTST_okay)
			return XEN_NETIF_CTRL_STATUS_INVALID_PARAMETER;
	}

	return XEN_NETIF_CTRL_STATUS_SUCCESS;
}

#ifdef CONFIG_DEBUG_FS
void xenvif_dump_hash_info(struct xenvif *vif, struct seq_file *m)
{
	unsigned int i;

	switch (vif->hash.alg) {
	case XEN_NETIF_CTRL_HASH_ALGORITHM_TOEPLITZ:
		seq_puts(m, "Hash Algorithm: TOEPLITZ\n");
		break;

	case XEN_NETIF_CTRL_HASH_ALGORITHM_NONE:
		seq_puts(m, "Hash Algorithm: NONE\n");
		/* FALLTHRU */
	default:
		return;
	}

	if (vif->hash.flags) {
		seq_puts(m, "\nHash Flags:\n");

		if (vif->hash.flags & XEN_NETIF_CTRL_HASH_TYPE_IPV4)
			seq_puts(m, "- IPv4\n");
		if (vif->hash.flags & XEN_NETIF_CTRL_HASH_TYPE_IPV4_TCP)
			seq_puts(m, "- IPv4 + TCP\n");
		if (vif->hash.flags & XEN_NETIF_CTRL_HASH_TYPE_IPV6)
			seq_puts(m, "- IPv6\n");
		if (vif->hash.flags & XEN_NETIF_CTRL_HASH_TYPE_IPV6_TCP)
			seq_puts(m, "- IPv6 + TCP\n");
	}

	seq_puts(m, "\nHash Key:\n");

	for (i = 0; i < XEN_NETBK_MAX_HASH_KEY_SIZE; ) {
		unsigned int j, n;

		n = 8;
		if (i + n >= XEN_NETBK_MAX_HASH_KEY_SIZE)
			n = XEN_NETBK_MAX_HASH_KEY_SIZE - i;

		seq_printf(m, "[%2u - %2u]: ", i, i + n - 1);

		for (j = 0; j < n; j++, i++)
			seq_printf(m, "%02x ", vif->hash.key[i]);

		seq_puts(m, "\n");
	}

	if (vif->hash.size != 0) {
		seq_puts(m, "\nHash Mapping:\n");

		for (i = 0; i < vif->hash.size; ) {
			unsigned int j, n;

			n = 8;
			if (i + n >= vif->hash.size)
				n = vif->hash.size - i;

			seq_printf(m, "[%4u - %4u]: ", i, i + n - 1);

			for (j = 0; j < n; j++, i++)
				seq_printf(m, "%4u ", vif->hash.mapping[i]);

			seq_puts(m, "\n");
		}
	}
}
#endif /* CONFIG_DEBUG_FS */

void xenvif_init_hash(struct xenvif *vif)
{
	if (xenvif_hash_cache_size == 0)
		return;

	spin_lock_init(&vif->hash.cache.lock);
	INIT_LIST_HEAD(&vif->hash.cache.list);
}

void xenvif_deinit_hash(struct xenvif *vif)
{
	xenvif_flush_hash(vif);
}