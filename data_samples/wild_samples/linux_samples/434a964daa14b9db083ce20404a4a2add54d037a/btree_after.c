	switch (id) {
	case HFS_EXT_CNID:
		hfs_inode_read_fork(tree->inode, mdb->drXTExtRec, mdb->drXTFlSize,
				    mdb->drXTFlSize, be32_to_cpu(mdb->drXTClpSiz));
		if (HFS_I(tree->inode)->alloc_blocks >
					HFS_I(tree->inode)->first_blocks) {
			printk(KERN_ERR "hfs: invalid btree extent records\n");
			unlock_new_inode(tree->inode);
			goto free_inode;
		}
		if (!HFS_I(tree->inode)->first_blocks) {
			printk(KERN_ERR "hfs: invalid btree extent records "
								"(0 size).\n");
			unlock_new_inode(tree->inode);
			goto free_inode;
		}

		tree->inode->i_mapping->a_ops = &hfs_btree_aops;
		break;
	default:
		BUG();
	}
	}
	unlock_new_inode(tree->inode);

	mapping = tree->inode->i_mapping;
	page = read_mapping_page(mapping, 0, NULL);
	if (IS_ERR(page))
		goto free_inode;

	/* Load the header */
	head = (struct hfs_btree_header_rec *)(kmap(page) + sizeof(struct hfs_bnode_desc));
	tree->root = be32_to_cpu(head->root);
	tree->leaf_count = be32_to_cpu(head->leaf_count);
	tree->leaf_head = be32_to_cpu(head->leaf_head);
	tree->leaf_tail = be32_to_cpu(head->leaf_tail);
	tree->node_count = be32_to_cpu(head->node_count);
	tree->free_nodes = be32_to_cpu(head->free_nodes);
	tree->attributes = be32_to_cpu(head->attributes);
	tree->node_size = be16_to_cpu(head->node_size);
	tree->max_key_len = be16_to_cpu(head->max_key_len);
	tree->depth = be16_to_cpu(head->depth);

	size = tree->node_size;
	if (!is_power_of_2(size))
		goto fail_page;
	if (!tree->node_count)
		goto fail_page;
	switch (id) {
	case HFS_EXT_CNID:
		if (tree->max_key_len != HFS_MAX_EXT_KEYLEN) {
			printk(KERN_ERR "hfs: invalid extent max_key_len %d\n",
				tree->max_key_len);
			goto fail_page;
		}
		break;
	case HFS_CAT_CNID:
		if (tree->max_key_len != HFS_MAX_CAT_KEYLEN) {
			printk(KERN_ERR "hfs: invalid catalog max_key_len %d\n",
				tree->max_key_len);
			goto fail_page;
		}
		break;
	default:
		BUG();
	}