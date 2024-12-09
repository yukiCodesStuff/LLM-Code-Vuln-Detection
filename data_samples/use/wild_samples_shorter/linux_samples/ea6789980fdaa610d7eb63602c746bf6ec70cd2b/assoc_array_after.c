		if ((edit->segment_cache[ASSOC_ARRAY_FAN_OUT] ^ base_seg) == 0)
			goto all_leaves_cluster_together;

		/* Otherwise all the old leaves cluster in the same slot, but
		 * the new leaf wants to go into a different slot - so we
		 * create a new node (n0) to hold the new leaf and a pointer to
		 * a new node (n1) holding all the old leaves.
		 *
		 * This can be done by falling through to the node splitting
		 * path.
		 */
		pr_devel("present leaves cluster but not new leaf\n");
	}

split_node:
	pr_devel("split node\n");

	/* We need to split the current node.  The node must contain anything
	 * from a single leaf (in the one leaf case, this leaf will cluster
	 * with the new leaf) and the rest meta-pointers, to all leaves, some
	 * of which may cluster.
	 *
	 * It won't contain the case in which all the current leaves plus the
	 * new leaves want to cluster in the same slot.
	 *
	 * We need to expel at least two leaves out of a set consisting of the
	 * leaves in the node and the new leaf.  The current meta pointers can
	 * just be copied as they shouldn't cluster with any of the leaves.
	 *
	 * We need a new node (n0) to replace the current one and a new node to
	 * take the expelled nodes (n1).
	 */
	pr_devel("<--%s() = ok [split node]\n", __func__);
	return true;

all_leaves_cluster_together:
	/* All the leaves, new and old, want to cluster together in this node
	 * in the same slot, so we have to replace this node with a shortcut to
	 * skip over the identical parts of the key and then place a pair of