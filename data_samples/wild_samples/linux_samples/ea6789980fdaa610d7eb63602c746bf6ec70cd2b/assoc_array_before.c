	if ((dissimilarity & ASSOC_ARRAY_FAN_MASK) == 0) {
		/* The old leaves all cluster in the same slot.  We will need
		 * to insert a shortcut if the new node wants to cluster with them.
		 */
		if ((edit->segment_cache[ASSOC_ARRAY_FAN_OUT] ^ base_seg) == 0)
			goto all_leaves_cluster_together;

		/* Otherwise we can just insert a new node ahead of the old
		 * one.
		 */
		goto present_leaves_cluster_but_not_new_leaf;
	}

split_node:
	pr_devel("split node\n");

	/* We need to split the current node; we know that the node doesn't
	 * simply contain a full set of leaves that cluster together (it
	 * contains meta pointers and/or non-clustering leaves).
	 *
	 * We need to expel at least two leaves out of a set consisting of the
	 * leaves in the node and the new leaf.
	 *
	 * We need a new node (n0) to replace the current one and a new node to
	 * take the expelled nodes (n1).
	 */
	edit->set[0].to = assoc_array_node_to_ptr(new_n0);
	new_n0->back_pointer = node->back_pointer;
	new_n0->parent_slot = node->parent_slot;
	new_n1->back_pointer = assoc_array_node_to_ptr(new_n0);
	new_n1->parent_slot = -1; /* Need to calculate this */

do_split_node:
	pr_devel("do_split_node\n");

	new_n0->nr_leaves_on_branch = node->nr_leaves_on_branch;
	new_n1->nr_leaves_on_branch = 0;

	/* Begin by finding two matching leaves.  There have to be at least two
	 * that match - even if there are meta pointers - because any leaf that
	 * would match a slot with a meta pointer in it must be somewhere
	 * behind that meta pointer and cannot be here.  Further, given N
	 * remaining leaf slots, we now have N+1 leaves to go in them.
	 */
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
		slot = edit->segment_cache[i];
		if (slot != 0xff)
			for (j = i + 1; j < ASSOC_ARRAY_FAN_OUT + 1; j++)
				if (edit->segment_cache[j] == slot)
					goto found_slot_for_multiple_occupancy;
	}
found_slot_for_multiple_occupancy:
	pr_devel("same slot: %x %x [%02x]\n", i, j, slot);
	BUG_ON(i >= ASSOC_ARRAY_FAN_OUT);
	BUG_ON(j >= ASSOC_ARRAY_FAN_OUT + 1);
	BUG_ON(slot >= ASSOC_ARRAY_FAN_OUT);

	new_n1->parent_slot = slot;

	/* Metadata pointers cannot change slot */
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++)
		if (assoc_array_ptr_is_meta(node->slots[i]))
			new_n0->slots[i] = node->slots[i];
		else
			new_n0->slots[i] = NULL;
	BUG_ON(new_n0->slots[slot] != NULL);
	new_n0->slots[slot] = assoc_array_node_to_ptr(new_n1);

	/* Filter the leaf pointers between the new nodes */
	free_slot = -1;
	next_slot = 0;
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
		if (assoc_array_ptr_is_meta(node->slots[i]))
			continue;
		if (edit->segment_cache[i] == slot) {
			new_n1->slots[next_slot++] = node->slots[i];
			new_n1->nr_leaves_on_branch++;
		} else {
			do {
				free_slot++;
			} while (new_n0->slots[free_slot] != NULL);
			new_n0->slots[free_slot] = node->slots[i];
		}
	}

	pr_devel("filtered: f=%x n=%x\n", free_slot, next_slot);

	if (edit->segment_cache[ASSOC_ARRAY_FAN_OUT] != slot) {
		do {
			free_slot++;
		} while (new_n0->slots[free_slot] != NULL);
		edit->leaf_p = &new_n0->slots[free_slot];
		edit->adjust_count_on = new_n0;
	} else {
		edit->leaf_p = &new_n1->slots[next_slot++];
		edit->adjust_count_on = new_n1;
	}

	BUG_ON(next_slot <= 1);

	edit->set_backpointers_to = assoc_array_node_to_ptr(new_n0);
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
		if (edit->segment_cache[i] == 0xff) {
			ptr = node->slots[i];
			BUG_ON(assoc_array_ptr_is_leaf(ptr));
			if (assoc_array_ptr_is_node(ptr)) {
				side = assoc_array_ptr_to_node(ptr);
				edit->set_backpointers[i] = &side->back_pointer;
			} else {
				shortcut = assoc_array_ptr_to_shortcut(ptr);
				edit->set_backpointers[i] = &shortcut->back_pointer;
			}
		}
	}

	ptr = node->back_pointer;
	if (!ptr)
		edit->set[0].ptr = &edit->array->root;
	else if (assoc_array_ptr_is_node(ptr))
		edit->set[0].ptr = &assoc_array_ptr_to_node(ptr)->slots[node->parent_slot];
	else
		edit->set[0].ptr = &assoc_array_ptr_to_shortcut(ptr)->next_node;
	edit->excised_meta[0] = assoc_array_node_to_ptr(node);
	pr_devel("<--%s() = ok [split node]\n", __func__);
	return true;

present_leaves_cluster_but_not_new_leaf:
	/* All the old leaves cluster in the same slot, but the new leaf wants
	 * to go into a different slot, so we create a new node to hold the new
	 * leaf and a pointer to a new node holding all the old leaves.
	 */
	pr_devel("present leaves cluster but not new leaf\n");

	new_n0->back_pointer = node->back_pointer;
	new_n0->parent_slot = node->parent_slot;
	new_n0->nr_leaves_on_branch = node->nr_leaves_on_branch;
	new_n1->back_pointer = assoc_array_node_to_ptr(new_n0);
	new_n1->parent_slot = edit->segment_cache[0];
	new_n1->nr_leaves_on_branch = node->nr_leaves_on_branch;
	edit->adjust_count_on = new_n0;

	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++)
		new_n1->slots[i] = node->slots[i];

	new_n0->slots[edit->segment_cache[0]] = assoc_array_node_to_ptr(new_n0);
	edit->leaf_p = &new_n0->slots[edit->segment_cache[ASSOC_ARRAY_FAN_OUT]];

	edit->set[0].ptr = &assoc_array_ptr_to_node(node->back_pointer)->slots[node->parent_slot];
	edit->set[0].to = assoc_array_node_to_ptr(new_n0);
	edit->excised_meta[0] = assoc_array_node_to_ptr(node);
	pr_devel("<--%s() = ok [insert node before]\n", __func__);
	return true;

all_leaves_cluster_together:
	/* All the leaves, new and old, want to cluster together in this node
	 * in the same slot, so we have to replace this node with a shortcut to
	 * skip over the identical parts of the key and then place a pair of
	 * nodes, one inside the other, at the end of the shortcut and
	 * distribute the keys between them.
	 *
	 * Firstly we need to work out where the leaves start diverging as a
	 * bit position into their keys so that we know how big the shortcut
	 * needs to be.
	 *
	 * We only need to make a single pass of N of the N+1 leaves because if
	 * any keys differ between themselves at bit X then at least one of
	 * them must also differ with the base key at bit X or before.
	 */
	pr_devel("all leaves cluster together\n");
	diff = INT_MAX;
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
		int x = ops->diff_objects(assoc_array_ptr_to_leaf(node->slots[i]),
					  index_key);
		if (x < diff) {
			BUG_ON(x < 0);
			diff = x;
		}
	}
	BUG_ON(diff == INT_MAX);
	BUG_ON(diff < level + ASSOC_ARRAY_LEVEL_STEP);

	keylen = round_up(diff, ASSOC_ARRAY_KEY_CHUNK_SIZE);
	keylen >>= ASSOC_ARRAY_KEY_CHUNK_SHIFT;

	new_s0 = kzalloc(sizeof(struct assoc_array_shortcut) +
			 keylen * sizeof(unsigned long), GFP_KERNEL);
	if (!new_s0)
		return false;
	edit->new_meta[2] = assoc_array_shortcut_to_ptr(new_s0);

	edit->set[0].to = assoc_array_shortcut_to_ptr(new_s0);
	new_s0->back_pointer = node->back_pointer;
	new_s0->parent_slot = node->parent_slot;
	new_s0->next_node = assoc_array_node_to_ptr(new_n0);
	new_n0->back_pointer = assoc_array_shortcut_to_ptr(new_s0);
	new_n0->parent_slot = 0;
	new_n1->back_pointer = assoc_array_node_to_ptr(new_n0);
	new_n1->parent_slot = -1; /* Need to calculate this */

	new_s0->skip_to_level = level = diff & ~ASSOC_ARRAY_LEVEL_STEP_MASK;
	pr_devel("skip_to_level = %d [diff %d]\n", level, diff);
	BUG_ON(level <= 0);

	for (i = 0; i < keylen; i++)
		new_s0->index_key[i] =
			ops->get_key_chunk(index_key, i * ASSOC_ARRAY_KEY_CHUNK_SIZE);

	blank = ULONG_MAX << (level & ASSOC_ARRAY_KEY_CHUNK_MASK);
	pr_devel("blank off [%zu] %d: %lx\n", keylen - 1, level, blank);
	new_s0->index_key[keylen - 1] &= ~blank;

	/* This now reduces to a node splitting exercise for which we'll need
	 * to regenerate the disparity table.
	 */
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
		ptr = node->slots[i];
		base_seg = ops->get_object_key_chunk(assoc_array_ptr_to_leaf(ptr),
						     level);
		base_seg >>= level & ASSOC_ARRAY_KEY_CHUNK_MASK;
		edit->segment_cache[i] = base_seg & ASSOC_ARRAY_FAN_MASK;
	}

	base_seg = ops->get_key_chunk(index_key, level);
	base_seg >>= level & ASSOC_ARRAY_KEY_CHUNK_MASK;
	edit->segment_cache[ASSOC_ARRAY_FAN_OUT] = base_seg & ASSOC_ARRAY_FAN_MASK;
	goto do_split_node;
}

/*
 * Handle insertion into the middle of a shortcut.
 */
static bool assoc_array_insert_mid_shortcut(struct assoc_array_edit *edit,
					    const struct assoc_array_ops *ops,
					    struct assoc_array_walk_result *result)
{
	struct assoc_array_shortcut *shortcut, *new_s0, *new_s1;
	struct assoc_array_node *node, *new_n0, *side;
	unsigned long sc_segments, dissimilarity, blank;
	size_t keylen;
	int level, sc_level, diff;
	int sc_slot;

	shortcut	= result->wrong_shortcut.shortcut;
	level		= result->wrong_shortcut.level;
	sc_level	= result->wrong_shortcut.sc_level;
	sc_segments	= result->wrong_shortcut.sc_segments;
	dissimilarity	= result->wrong_shortcut.dissimilarity;

	pr_devel("-->%s(ix=%d dis=%lx scix=%d)\n",
		 __func__, level, dissimilarity, sc_level);

	/* We need to split a shortcut and insert a node between the two
	 * pieces.  Zero-length pieces will be dispensed with entirely.
	 *
	 * First of all, we need to find out in which level the first
	 * difference was.
	 */
	diff = __ffs(dissimilarity);
	diff &= ~ASSOC_ARRAY_LEVEL_STEP_MASK;
	diff += sc_level & ~ASSOC_ARRAY_KEY_CHUNK_MASK;
	pr_devel("diff=%d\n", diff);

	if (!shortcut->back_pointer) {
		edit->set[0].ptr = &edit->array->root;
	} else if (assoc_array_ptr_is_node(shortcut->back_pointer)) {
		node = assoc_array_ptr_to_node(shortcut->back_pointer);
		edit->set[0].ptr = &node->slots[shortcut->parent_slot];
	} else {
		BUG();
	}

	edit->excised_meta[0] = assoc_array_shortcut_to_ptr(shortcut);

	/* Create a new node now since we're going to need it anyway */
	new_n0 = kzalloc(sizeof(struct assoc_array_node), GFP_KERNEL);
	if (!new_n0)
		return false;
	edit->new_meta[0] = assoc_array_node_to_ptr(new_n0);
	edit->adjust_count_on = new_n0;

	/* Insert a new shortcut before the new node if this segment isn't of
	 * zero length - otherwise we just connect the new node directly to the
	 * parent.
	 */
	level += ASSOC_ARRAY_LEVEL_STEP;
	if (diff > level) {
		pr_devel("pre-shortcut %d...%d\n", level, diff);
		keylen = round_up(diff, ASSOC_ARRAY_KEY_CHUNK_SIZE);
		keylen >>= ASSOC_ARRAY_KEY_CHUNK_SHIFT;

		new_s0 = kzalloc(sizeof(struct assoc_array_shortcut) +
				 keylen * sizeof(unsigned long), GFP_KERNEL);
		if (!new_s0)
			return false;
		edit->new_meta[1] = assoc_array_shortcut_to_ptr(new_s0);
		edit->set[0].to = assoc_array_shortcut_to_ptr(new_s0);
		new_s0->back_pointer = shortcut->back_pointer;
		new_s0->parent_slot = shortcut->parent_slot;
		new_s0->next_node = assoc_array_node_to_ptr(new_n0);
		new_s0->skip_to_level = diff;

		new_n0->back_pointer = assoc_array_shortcut_to_ptr(new_s0);
		new_n0->parent_slot = 0;

		memcpy(new_s0->index_key, shortcut->index_key,
		       keylen * sizeof(unsigned long));

		blank = ULONG_MAX << (diff & ASSOC_ARRAY_KEY_CHUNK_MASK);
		pr_devel("blank off [%zu] %d: %lx\n", keylen - 1, diff, blank);
		new_s0->index_key[keylen - 1] &= ~blank;
	} else {
		pr_devel("no pre-shortcut\n");
		edit->set[0].to = assoc_array_node_to_ptr(new_n0);
		new_n0->back_pointer = shortcut->back_pointer;
		new_n0->parent_slot = shortcut->parent_slot;
	}

	side = assoc_array_ptr_to_node(shortcut->next_node);
	new_n0->nr_leaves_on_branch = side->nr_leaves_on_branch;

	/* We need to know which slot in the new node is going to take a
	 * metadata pointer.
	 */
	sc_slot = sc_segments >> (diff & ASSOC_ARRAY_KEY_CHUNK_MASK);
	sc_slot &= ASSOC_ARRAY_FAN_MASK;

	pr_devel("new slot %lx >> %d -> %d\n",
		 sc_segments, diff & ASSOC_ARRAY_KEY_CHUNK_MASK, sc_slot);

	/* Determine whether we need to follow the new node with a replacement
	 * for the current shortcut.  We could in theory reuse the current
	 * shortcut if its parent slot number doesn't change - but that's a
	 * 1-in-16 chance so not worth expending the code upon.
	 */
	level = diff + ASSOC_ARRAY_LEVEL_STEP;
	if (level < shortcut->skip_to_level) {
		pr_devel("post-shortcut %d...%d\n", level, shortcut->skip_to_level);
		keylen = round_up(shortcut->skip_to_level, ASSOC_ARRAY_KEY_CHUNK_SIZE);
		keylen >>= ASSOC_ARRAY_KEY_CHUNK_SHIFT;

		new_s1 = kzalloc(sizeof(struct assoc_array_shortcut) +
				 keylen * sizeof(unsigned long), GFP_KERNEL);
		if (!new_s1)
			return false;
		edit->new_meta[2] = assoc_array_shortcut_to_ptr(new_s1);

		new_s1->back_pointer = assoc_array_node_to_ptr(new_n0);
		new_s1->parent_slot = sc_slot;
		new_s1->next_node = shortcut->next_node;
		new_s1->skip_to_level = shortcut->skip_to_level;

		new_n0->slots[sc_slot] = assoc_array_shortcut_to_ptr(new_s1);

		memcpy(new_s1->index_key, shortcut->index_key,
		       keylen * sizeof(unsigned long));

		edit->set[1].ptr = &side->back_pointer;
		edit->set[1].to = assoc_array_shortcut_to_ptr(new_s1);
	} else {
		pr_devel("no post-shortcut\n");

		/* We don't have to replace the pointed-to node as long as we
		 * use memory barriers to make sure the parent slot number is
		 * changed before the back pointer (the parent slot number is
		 * irrelevant to the old parent shortcut).
		 */
		new_n0->slots[sc_slot] = shortcut->next_node;
		edit->set_parent_slot[0].p = &side->parent_slot;
		edit->set_parent_slot[0].to = sc_slot;
		edit->set[1].ptr = &side->back_pointer;
		edit->set[1].to = assoc_array_node_to_ptr(new_n0);
	}

	/* Install the new leaf in a spare slot in the new node. */
	if (sc_slot == 0)
		edit->leaf_p = &new_n0->slots[1];
	else
		edit->leaf_p = &new_n0->slots[0];

	pr_devel("<--%s() = ok [split shortcut]\n", __func__);
	return edit;
}

/**
 * assoc_array_insert - Script insertion of an object into an associative array
 * @array: The array to insert into.
 * @ops: The operations to use.
 * @index_key: The key to insert at.
 * @object: The object to insert.
 *
 * Precalculate and preallocate a script for the insertion or replacement of an
 * object in an associative array.  This results in an edit script that can
 * either be applied or cancelled.
 *
 * The function returns a pointer to an edit script or -ENOMEM.
 *
 * The caller should lock against other modifications and must continue to hold
 * the lock until assoc_array_apply_edit() has been called.
 *
 * Accesses to the tree may take place concurrently with this function,
 * provided they hold the RCU read lock.
 */
struct assoc_array_edit *assoc_array_insert(struct assoc_array *array,
					    const struct assoc_array_ops *ops,
					    const void *index_key,
					    void *object)
{
	struct assoc_array_walk_result result;
	struct assoc_array_edit *edit;

	pr_devel("-->%s()\n", __func__);

	/* The leaf pointer we're given must not have the bottom bit set as we
	 * use those for type-marking the pointer.  NULL pointers are also not
	 * allowed as they indicate an empty slot but we have to allow them
	 * here as they can be updated later.
	 */
	BUG_ON(assoc_array_ptr_is_meta(object));

	edit = kzalloc(sizeof(struct assoc_array_edit), GFP_KERNEL);
	if (!edit)
		return ERR_PTR(-ENOMEM);
	edit->array = array;
	edit->ops = ops;
	edit->leaf = assoc_array_leaf_to_ptr(object);
	edit->adjust_count_by = 1;

	switch (assoc_array_walk(array, ops, index_key, &result)) {
	case assoc_array_walk_tree_empty:
		/* Allocate a root node if there isn't one yet */
		if (!assoc_array_insert_in_empty_tree(edit))
			goto enomem;
		return edit;

	case assoc_array_walk_found_terminal_node:
		/* We found a node that doesn't have a node/shortcut pointer in
		 * the slot corresponding to the index key that we have to
		 * follow.
		 */
		if (!assoc_array_insert_into_terminal_node(edit, ops, index_key,
							   &result))
			goto enomem;
		return edit;

	case assoc_array_walk_found_wrong_shortcut:
		/* We found a shortcut that didn't match our key in a slot we
		 * needed to follow.
		 */
		if (!assoc_array_insert_mid_shortcut(edit, ops, &result))
			goto enomem;
		return edit;
	}

enomem:
	/* Clean up after an out of memory error */
	pr_devel("enomem\n");
	assoc_array_cancel_edit(edit);
	return ERR_PTR(-ENOMEM);
}

/**
 * assoc_array_insert_set_object - Set the new object pointer in an edit script
 * @edit: The edit script to modify.
 * @object: The object pointer to set.
 *
 * Change the object to be inserted in an edit script.  The object pointed to
 * by the old object is not freed.  This must be done prior to applying the
 * script.
 */
void assoc_array_insert_set_object(struct assoc_array_edit *edit, void *object)
{
	BUG_ON(!object);
	edit->leaf = assoc_array_leaf_to_ptr(object);
}

struct assoc_array_delete_collapse_context {
	struct assoc_array_node	*node;
	const void		*skip_leaf;
	int			slot;
};

/*
 * Subtree collapse to node iterator.
 */
static int assoc_array_delete_collapse_iterator(const void *leaf,
						void *iterator_data)
{
	struct assoc_array_delete_collapse_context *collapse = iterator_data;

	if (leaf == collapse->skip_leaf)
		return 0;

	BUG_ON(collapse->slot >= ASSOC_ARRAY_FAN_OUT);

	collapse->node->slots[collapse->slot++] = assoc_array_leaf_to_ptr(leaf);
	return 0;
}

/**
 * assoc_array_delete - Script deletion of an object from an associative array
 * @array: The array to search.
 * @ops: The operations to use.
 * @index_key: The key to the object.
 *
 * Precalculate and preallocate a script for the deletion of an object from an
 * associative array.  This results in an edit script that can either be
 * applied or cancelled.
 *
 * The function returns a pointer to an edit script if the object was found,
 * NULL if the object was not found or -ENOMEM.
 *
 * The caller should lock against other modifications and must continue to hold
 * the lock until assoc_array_apply_edit() has been called.
 *
 * Accesses to the tree may take place concurrently with this function,
 * provided they hold the RCU read lock.
 */
struct assoc_array_edit *assoc_array_delete(struct assoc_array *array,
					    const struct assoc_array_ops *ops,
					    const void *index_key)
{
	struct assoc_array_delete_collapse_context collapse;
	struct assoc_array_walk_result result;
	struct assoc_array_node *node, *new_n0;
	struct assoc_array_edit *edit;
	struct assoc_array_ptr *ptr;
	bool has_meta;
	int slot, i;

	pr_devel("-->%s()\n", __func__);

	edit = kzalloc(sizeof(struct assoc_array_edit), GFP_KERNEL);
	if (!edit)
		return ERR_PTR(-ENOMEM);
	edit->array = array;
	edit->ops = ops;
	edit->adjust_count_by = -1;

	switch (assoc_array_walk(array, ops, index_key, &result)) {
	case assoc_array_walk_found_terminal_node:
		/* We found a node that should contain the leaf we've been
		 * asked to remove - *if* it's in the tree.
		 */
		pr_devel("terminal_node\n");
		node = result.terminal_node.node;

		for (slot = 0; slot < ASSOC_ARRAY_FAN_OUT; slot++) {
			ptr = node->slots[slot];
			if (ptr &&
			    assoc_array_ptr_is_leaf(ptr) &&
			    ops->compare_object(assoc_array_ptr_to_leaf(ptr),
						index_key))
				goto found_leaf;
		}
	case assoc_array_walk_tree_empty:
	case assoc_array_walk_found_wrong_shortcut:
	default:
		assoc_array_cancel_edit(edit);
		pr_devel("not found\n");
		return NULL;
	}

found_leaf:
	BUG_ON(array->nr_leaves_on_tree <= 0);

	/* In the simplest form of deletion we just clear the slot and release
	 * the leaf after a suitable interval.
	 */
	edit->dead_leaf = node->slots[slot];
	edit->set[0].ptr = &node->slots[slot];
	edit->set[0].to = NULL;
	edit->adjust_count_on = node;

	/* If that concludes erasure of the last leaf, then delete the entire
	 * internal array.
	 */
	if (array->nr_leaves_on_tree == 1) {
		edit->set[1].ptr = &array->root;
		edit->set[1].to = NULL;
		edit->adjust_count_on = NULL;
		edit->excised_subtree = array->root;
		pr_devel("all gone\n");
		return edit;
	}

	/* However, we'd also like to clear up some metadata blocks if we
	 * possibly can.
	 *
	 * We go for a simple algorithm of: if this node has FAN_OUT or fewer
	 * leaves in it, then attempt to collapse it - and attempt to
	 * recursively collapse up the tree.
	 *
	 * We could also try and collapse in partially filled subtrees to take
	 * up space in this node.
	 */
	if (node->nr_leaves_on_branch <= ASSOC_ARRAY_FAN_OUT + 1) {
		struct assoc_array_node *parent, *grandparent;
		struct assoc_array_ptr *ptr;

		/* First of all, we need to know if this node has metadata so
		 * that we don't try collapsing if all the leaves are already
		 * here.
		 */
		has_meta = false;
		for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
			ptr = node->slots[i];
			if (assoc_array_ptr_is_meta(ptr)) {
				has_meta = true;
				break;
			}
		}

		pr_devel("leaves: %ld [m=%d]\n",
			 node->nr_leaves_on_branch - 1, has_meta);

		/* Look further up the tree to see if we can collapse this node
		 * into a more proximal node too.
		 */
		parent = node;
	collapse_up:
		pr_devel("collapse subtree: %ld\n", parent->nr_leaves_on_branch);

		ptr = parent->back_pointer;
		if (!ptr)
			goto do_collapse;
		if (assoc_array_ptr_is_shortcut(ptr)) {
			struct assoc_array_shortcut *s = assoc_array_ptr_to_shortcut(ptr);
			ptr = s->back_pointer;
			if (!ptr)
				goto do_collapse;
		}

		grandparent = assoc_array_ptr_to_node(ptr);
		if (grandparent->nr_leaves_on_branch <= ASSOC_ARRAY_FAN_OUT + 1) {
			parent = grandparent;
			goto collapse_up;
		}

	do_collapse:
		/* There's no point collapsing if the original node has no meta
		 * pointers to discard and if we didn't merge into one of that
		 * node's ancestry.
		 */
		if (has_meta || parent != node) {
			node = parent;

			/* Create a new node to collapse into */
			new_n0 = kzalloc(sizeof(struct assoc_array_node), GFP_KERNEL);
			if (!new_n0)
				goto enomem;
			edit->new_meta[0] = assoc_array_node_to_ptr(new_n0);

			new_n0->back_pointer = node->back_pointer;
			new_n0->parent_slot = node->parent_slot;
			new_n0->nr_leaves_on_branch = node->nr_leaves_on_branch;
			edit->adjust_count_on = new_n0;

			collapse.node = new_n0;
			collapse.skip_leaf = assoc_array_ptr_to_leaf(edit->dead_leaf);
			collapse.slot = 0;
			assoc_array_subtree_iterate(assoc_array_node_to_ptr(node),
						    node->back_pointer,
						    assoc_array_delete_collapse_iterator,
						    &collapse);
			pr_devel("collapsed %d,%lu\n", collapse.slot, new_n0->nr_leaves_on_branch);
			BUG_ON(collapse.slot != new_n0->nr_leaves_on_branch - 1);

			if (!node->back_pointer) {
				edit->set[1].ptr = &array->root;
			} else if (assoc_array_ptr_is_leaf(node->back_pointer)) {
				BUG();
			} else if (assoc_array_ptr_is_node(node->back_pointer)) {
				struct assoc_array_node *p =
					assoc_array_ptr_to_node(node->back_pointer);
				edit->set[1].ptr = &p->slots[node->parent_slot];
			} else if (assoc_array_ptr_is_shortcut(node->back_pointer)) {
				struct assoc_array_shortcut *s =
					assoc_array_ptr_to_shortcut(node->back_pointer);
				edit->set[1].ptr = &s->next_node;
			}
			edit->set[1].to = assoc_array_node_to_ptr(new_n0);
			edit->excised_subtree = assoc_array_node_to_ptr(node);
		}
	}

	return edit;

enomem:
	/* Clean up after an out of memory error */
	pr_devel("enomem\n");
	assoc_array_cancel_edit(edit);
	return ERR_PTR(-ENOMEM);
}

/**
 * assoc_array_clear - Script deletion of all objects from an associative array
 * @array: The array to clear.
 * @ops: The operations to use.
 *
 * Precalculate and preallocate a script for the deletion of all the objects
 * from an associative array.  This results in an edit script that can either
 * be applied or cancelled.
 *
 * The function returns a pointer to an edit script if there are objects to be
 * deleted, NULL if there are no objects in the array or -ENOMEM.
 *
 * The caller should lock against other modifications and must continue to hold
 * the lock until assoc_array_apply_edit() has been called.
 *
 * Accesses to the tree may take place concurrently with this function,
 * provided they hold the RCU read lock.
 */
struct assoc_array_edit *assoc_array_clear(struct assoc_array *array,
					   const struct assoc_array_ops *ops)
{
	struct assoc_array_edit *edit;

	pr_devel("-->%s()\n", __func__);

	if (!array->root)
		return NULL;

	edit = kzalloc(sizeof(struct assoc_array_edit), GFP_KERNEL);
	if (!edit)
		return ERR_PTR(-ENOMEM);
	edit->array = array;
	edit->ops = ops;
	edit->set[1].ptr = &array->root;
	edit->set[1].to = NULL;
	edit->excised_subtree = array->root;
	edit->ops_for_excised_subtree = ops;
	pr_devel("all gone\n");
	return edit;
}

/*
 * Handle the deferred destruction after an applied edit.
 */
static void assoc_array_rcu_cleanup(struct rcu_head *head)
{
	struct assoc_array_edit *edit =
		container_of(head, struct assoc_array_edit, rcu);
	int i;

	pr_devel("-->%s()\n", __func__);

	if (edit->dead_leaf)
		edit->ops->free_object(assoc_array_ptr_to_leaf(edit->dead_leaf));
	for (i = 0; i < ARRAY_SIZE(edit->excised_meta); i++)
		if (edit->excised_meta[i])
			kfree(assoc_array_ptr_to_node(edit->excised_meta[i]));

	if (edit->excised_subtree) {
		BUG_ON(assoc_array_ptr_is_leaf(edit->excised_subtree));
		if (assoc_array_ptr_is_node(edit->excised_subtree)) {
			struct assoc_array_node *n =
				assoc_array_ptr_to_node(edit->excised_subtree);
			n->back_pointer = NULL;
		} else {
			struct assoc_array_shortcut *s =
				assoc_array_ptr_to_shortcut(edit->excised_subtree);
			s->back_pointer = NULL;
		}
		assoc_array_destroy_subtree(edit->excised_subtree,
					    edit->ops_for_excised_subtree);
	}

	kfree(edit);
}

/**
 * assoc_array_apply_edit - Apply an edit script to an associative array
 * @edit: The script to apply.
 *
 * Apply an edit script to an associative array to effect an insertion,
 * deletion or clearance.  As the edit script includes preallocated memory,
 * this is guaranteed not to fail.
 *
 * The edit script, dead objects and dead metadata will be scheduled for
 * destruction after an RCU grace period to permit those doing read-only
 * accesses on the array to continue to do so under the RCU read lock whilst
 * the edit is taking place.
 */
void assoc_array_apply_edit(struct assoc_array_edit *edit)
{
	struct assoc_array_shortcut *shortcut;
	struct assoc_array_node *node;
	struct assoc_array_ptr *ptr;
	int i;

	pr_devel("-->%s()\n", __func__);

	smp_wmb();
	if (edit->leaf_p)
		*edit->leaf_p = edit->leaf;

	smp_wmb();
	for (i = 0; i < ARRAY_SIZE(edit->set_parent_slot); i++)
		if (edit->set_parent_slot[i].p)
			*edit->set_parent_slot[i].p = edit->set_parent_slot[i].to;

	smp_wmb();
	for (i = 0; i < ARRAY_SIZE(edit->set_backpointers); i++)
		if (edit->set_backpointers[i])
			*edit->set_backpointers[i] = edit->set_backpointers_to;

	smp_wmb();
	for (i = 0; i < ARRAY_SIZE(edit->set); i++)
		if (edit->set[i].ptr)
			*edit->set[i].ptr = edit->set[i].to;

	if (edit->array->root == NULL) {
		edit->array->nr_leaves_on_tree = 0;
	} else if (edit->adjust_count_on) {
		node = edit->adjust_count_on;
		for (;;) {
			node->nr_leaves_on_branch += edit->adjust_count_by;

			ptr = node->back_pointer;
			if (!ptr)
				break;
			if (assoc_array_ptr_is_shortcut(ptr)) {
				shortcut = assoc_array_ptr_to_shortcut(ptr);
				ptr = shortcut->back_pointer;
				if (!ptr)
					break;
			}
			BUG_ON(!assoc_array_ptr_is_node(ptr));
			node = assoc_array_ptr_to_node(ptr);
		}

		edit->array->nr_leaves_on_tree += edit->adjust_count_by;
	}

	call_rcu(&edit->rcu, assoc_array_rcu_cleanup);
}

/**
 * assoc_array_cancel_edit - Discard an edit script.
 * @edit: The script to discard.
 *
 * Free an edit script and all the preallocated data it holds without making
 * any changes to the associative array it was intended for.
 *
 * NOTE!  In the case of an insertion script, this does _not_ release the leaf
 * that was to be inserted.  That is left to the caller.
 */
void assoc_array_cancel_edit(struct assoc_array_edit *edit)
{
	struct assoc_array_ptr *ptr;
	int i;

	pr_devel("-->%s()\n", __func__);

	/* Clean up after an out of memory error */
	for (i = 0; i < ARRAY_SIZE(edit->new_meta); i++) {
		ptr = edit->new_meta[i];
		if (ptr) {
			if (assoc_array_ptr_is_node(ptr))
				kfree(assoc_array_ptr_to_node(ptr));
			else
				kfree(assoc_array_ptr_to_shortcut(ptr));
		}
	}
	kfree(edit);
}

/**
 * assoc_array_gc - Garbage collect an associative array.
 * @array: The array to clean.
 * @ops: The operations to use.
 * @iterator: A callback function to pass judgement on each object.
 * @iterator_data: Private data for the callback function.
 *
 * Collect garbage from an associative array and pack down the internal tree to
 * save memory.
 *
 * The iterator function is asked to pass judgement upon each object in the
 * array.  If it returns false, the object is discard and if it returns true,
 * the object is kept.  If it returns true, it must increment the object's
 * usage count (or whatever it needs to do to retain it) before returning.
 *
 * This function returns 0 if successful or -ENOMEM if out of memory.  In the
 * latter case, the array is not changed.
 *
 * The caller should lock against other modifications and must continue to hold
 * the lock until assoc_array_apply_edit() has been called.
 *
 * Accesses to the tree may take place concurrently with this function,
 * provided they hold the RCU read lock.
 */
int assoc_array_gc(struct assoc_array *array,
		   const struct assoc_array_ops *ops,
		   bool (*iterator)(void *object, void *iterator_data),
		   void *iterator_data)
{
	struct assoc_array_shortcut *shortcut, *new_s;
	struct assoc_array_node *node, *new_n;
	struct assoc_array_edit *edit;
	struct assoc_array_ptr *cursor, *ptr;
	struct assoc_array_ptr *new_root, *new_parent, **new_ptr_pp;
	unsigned long nr_leaves_on_tree;
	int keylen, slot, nr_free, next_slot, i;

	pr_devel("-->%s()\n", __func__);

	if (!array->root)
		return 0;

	edit = kzalloc(sizeof(struct assoc_array_edit), GFP_KERNEL);
	if (!edit)
		return -ENOMEM;
	edit->array = array;
	edit->ops = ops;
	edit->ops_for_excised_subtree = ops;
	edit->set[0].ptr = &array->root;
	edit->excised_subtree = array->root;

	new_root = new_parent = NULL;
	new_ptr_pp = &new_root;
	cursor = array->root;

descend:
	/* If this point is a shortcut, then we need to duplicate it and
	 * advance the target cursor.
	 */
	if (assoc_array_ptr_is_shortcut(cursor)) {
		shortcut = assoc_array_ptr_to_shortcut(cursor);
		keylen = round_up(shortcut->skip_to_level, ASSOC_ARRAY_KEY_CHUNK_SIZE);
		keylen >>= ASSOC_ARRAY_KEY_CHUNK_SHIFT;
		new_s = kmalloc(sizeof(struct assoc_array_shortcut) +
				keylen * sizeof(unsigned long), GFP_KERNEL);
		if (!new_s)
			goto enomem;
		pr_devel("dup shortcut %p -> %p\n", shortcut, new_s);
		memcpy(new_s, shortcut, (sizeof(struct assoc_array_shortcut) +
					 keylen * sizeof(unsigned long)));
		new_s->back_pointer = new_parent;
		new_s->parent_slot = shortcut->parent_slot;
		*new_ptr_pp = new_parent = assoc_array_shortcut_to_ptr(new_s);
		new_ptr_pp = &new_s->next_node;
		cursor = shortcut->next_node;
	}

	/* Duplicate the node at this position */
	node = assoc_array_ptr_to_node(cursor);
	new_n = kzalloc(sizeof(struct assoc_array_node), GFP_KERNEL);
	if (!new_n)
		goto enomem;
	pr_devel("dup node %p -> %p\n", node, new_n);
	new_n->back_pointer = new_parent;
	new_n->parent_slot = node->parent_slot;
	*new_ptr_pp = new_parent = assoc_array_node_to_ptr(new_n);
	new_ptr_pp = NULL;
	slot = 0;

continue_node:
	/* Filter across any leaves and gc any subtrees */
	for (; slot < ASSOC_ARRAY_FAN_OUT; slot++) {
		ptr = node->slots[slot];
		if (!ptr)
			continue;

		if (assoc_array_ptr_is_leaf(ptr)) {
			if (iterator(assoc_array_ptr_to_leaf(ptr),
				     iterator_data))
				/* The iterator will have done any reference
				 * counting on the object for us.
				 */
				new_n->slots[slot] = ptr;
			continue;
		}

		new_ptr_pp = &new_n->slots[slot];
		cursor = ptr;
		goto descend;
	}

	pr_devel("-- compress node %p --\n", new_n);

	/* Count up the number of empty slots in this node and work out the
	 * subtree leaf count.
	 */
	new_n->nr_leaves_on_branch = 0;
	nr_free = 0;
	for (slot = 0; slot < ASSOC_ARRAY_FAN_OUT; slot++) {
		ptr = new_n->slots[slot];
		if (!ptr)
			nr_free++;
		else if (assoc_array_ptr_is_leaf(ptr))
			new_n->nr_leaves_on_branch++;
	}
	pr_devel("free=%d, leaves=%lu\n", nr_free, new_n->nr_leaves_on_branch);

	/* See what we can fold in */
	next_slot = 0;
	for (slot = 0; slot < ASSOC_ARRAY_FAN_OUT; slot++) {
		struct assoc_array_shortcut *s;
		struct assoc_array_node *child;

		ptr = new_n->slots[slot];
		if (!ptr || assoc_array_ptr_is_leaf(ptr))
			continue;

		s = NULL;
		if (assoc_array_ptr_is_shortcut(ptr)) {
			s = assoc_array_ptr_to_shortcut(ptr);
			ptr = s->next_node;
		}

		child = assoc_array_ptr_to_node(ptr);
		new_n->nr_leaves_on_branch += child->nr_leaves_on_branch;

		if (child->nr_leaves_on_branch <= nr_free + 1) {
			/* Fold the child node into this one */
			pr_devel("[%d] fold node %lu/%d [nx %d]\n",
				 slot, child->nr_leaves_on_branch, nr_free + 1,
				 next_slot);

			/* We would already have reaped an intervening shortcut
			 * on the way back up the tree.
			 */
			BUG_ON(s);

			new_n->slots[slot] = NULL;
			nr_free++;
			if (slot < next_slot)
				next_slot = slot;
			for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
				struct assoc_array_ptr *p = child->slots[i];
				if (!p)
					continue;
				BUG_ON(assoc_array_ptr_is_meta(p));
				while (new_n->slots[next_slot])
					next_slot++;
				BUG_ON(next_slot >= ASSOC_ARRAY_FAN_OUT);
				new_n->slots[next_slot++] = p;
				nr_free--;
			}
			kfree(child);
		} else {
			pr_devel("[%d] retain node %lu/%d [nx %d]\n",
				 slot, child->nr_leaves_on_branch, nr_free + 1,
				 next_slot);
		}
	}

	pr_devel("after: %lu\n", new_n->nr_leaves_on_branch);

	nr_leaves_on_tree = new_n->nr_leaves_on_branch;

	/* Excise this node if it is singly occupied by a shortcut */
	if (nr_free == ASSOC_ARRAY_FAN_OUT - 1) {
		for (slot = 0; slot < ASSOC_ARRAY_FAN_OUT; slot++)
			if ((ptr = new_n->slots[slot]))
				break;

		if (assoc_array_ptr_is_meta(ptr) &&
		    assoc_array_ptr_is_shortcut(ptr)) {
			pr_devel("excise node %p with 1 shortcut\n", new_n);
			new_s = assoc_array_ptr_to_shortcut(ptr);
			new_parent = new_n->back_pointer;
			slot = new_n->parent_slot;
			kfree(new_n);
			if (!new_parent) {
				new_s->back_pointer = NULL;
				new_s->parent_slot = 0;
				new_root = ptr;
				goto gc_complete;
			}

			if (assoc_array_ptr_is_shortcut(new_parent)) {
				/* We can discard any preceding shortcut also */
				struct assoc_array_shortcut *s =
					assoc_array_ptr_to_shortcut(new_parent);

				pr_devel("excise preceding shortcut\n");

				new_parent = new_s->back_pointer = s->back_pointer;
				slot = new_s->parent_slot = s->parent_slot;
				kfree(s);
				if (!new_parent) {
					new_s->back_pointer = NULL;
					new_s->parent_slot = 0;
					new_root = ptr;
					goto gc_complete;
				}
			}

			new_s->back_pointer = new_parent;
			new_s->parent_slot = slot;
			new_n = assoc_array_ptr_to_node(new_parent);
			new_n->slots[slot] = ptr;
			goto ascend_old_tree;
		}
	}

	/* Excise any shortcuts we might encounter that point to nodes that
	 * only contain leaves.
	 */
	ptr = new_n->back_pointer;
	if (!ptr)
		goto gc_complete;

	if (assoc_array_ptr_is_shortcut(ptr)) {
		new_s = assoc_array_ptr_to_shortcut(ptr);
		new_parent = new_s->back_pointer;
		slot = new_s->parent_slot;

		if (new_n->nr_leaves_on_branch <= ASSOC_ARRAY_FAN_OUT) {
			struct assoc_array_node *n;

			pr_devel("excise shortcut\n");
			new_n->back_pointer = new_parent;
			new_n->parent_slot = slot;
			kfree(new_s);
			if (!new_parent) {
				new_root = assoc_array_node_to_ptr(new_n);
				goto gc_complete;
			}

			n = assoc_array_ptr_to_node(new_parent);
			n->slots[slot] = assoc_array_node_to_ptr(new_n);
		}
	} else {
		new_parent = ptr;
	}
	new_n = assoc_array_ptr_to_node(new_parent);

ascend_old_tree:
	ptr = node->back_pointer;
	if (assoc_array_ptr_is_shortcut(ptr)) {
		shortcut = assoc_array_ptr_to_shortcut(ptr);
		slot = shortcut->parent_slot;
		cursor = shortcut->back_pointer;
		if (!cursor)
			goto gc_complete;
	} else {
		slot = node->parent_slot;
		cursor = ptr;
	}
	BUG_ON(!cursor);
	node = assoc_array_ptr_to_node(cursor);
	slot++;
	goto continue_node;

gc_complete:
	edit->set[0].to = new_root;
	assoc_array_apply_edit(edit);
	array->nr_leaves_on_tree = nr_leaves_on_tree;
	return 0;

enomem:
	pr_devel("enomem\n");
	assoc_array_destroy_subtree(new_root, edit->ops);
	kfree(edit);
	return -ENOMEM;
}
			} else {
				shortcut = assoc_array_ptr_to_shortcut(ptr);
				edit->set_backpointers[i] = &shortcut->back_pointer;
			}
		}
	}

	ptr = node->back_pointer;
	if (!ptr)
		edit->set[0].ptr = &edit->array->root;
	else if (assoc_array_ptr_is_node(ptr))
		edit->set[0].ptr = &assoc_array_ptr_to_node(ptr)->slots[node->parent_slot];
	else
		edit->set[0].ptr = &assoc_array_ptr_to_shortcut(ptr)->next_node;
	edit->excised_meta[0] = assoc_array_node_to_ptr(node);
	pr_devel("<--%s() = ok [split node]\n", __func__);
	return true;

present_leaves_cluster_but_not_new_leaf:
	/* All the old leaves cluster in the same slot, but the new leaf wants
	 * to go into a different slot, so we create a new node to hold the new
	 * leaf and a pointer to a new node holding all the old leaves.
	 */
	pr_devel("present leaves cluster but not new leaf\n");

	new_n0->back_pointer = node->back_pointer;
	new_n0->parent_slot = node->parent_slot;
	new_n0->nr_leaves_on_branch = node->nr_leaves_on_branch;
	new_n1->back_pointer = assoc_array_node_to_ptr(new_n0);
	new_n1->parent_slot = edit->segment_cache[0];
	new_n1->nr_leaves_on_branch = node->nr_leaves_on_branch;
	edit->adjust_count_on = new_n0;

	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++)
		new_n1->slots[i] = node->slots[i];

	new_n0->slots[edit->segment_cache[0]] = assoc_array_node_to_ptr(new_n0);
	edit->leaf_p = &new_n0->slots[edit->segment_cache[ASSOC_ARRAY_FAN_OUT]];

	edit->set[0].ptr = &assoc_array_ptr_to_node(node->back_pointer)->slots[node->parent_slot];
	edit->set[0].to = assoc_array_node_to_ptr(new_n0);
	edit->excised_meta[0] = assoc_array_node_to_ptr(node);
	pr_devel("<--%s() = ok [insert node before]\n", __func__);
	return true;

all_leaves_cluster_together:
	/* All the leaves, new and old, want to cluster together in this node
	 * in the same slot, so we have to replace this node with a shortcut to
	 * skip over the identical parts of the key and then place a pair of
	 * nodes, one inside the other, at the end of the shortcut and
	 * distribute the keys between them.
	 *
	 * Firstly we need to work out where the leaves start diverging as a
	 * bit position into their keys so that we know how big the shortcut
	 * needs to be.
	 *
	 * We only need to make a single pass of N of the N+1 leaves because if
	 * any keys differ between themselves at bit X then at least one of
	 * them must also differ with the base key at bit X or before.
	 */
	pr_devel("all leaves cluster together\n");
	diff = INT_MAX;
	for (i = 0; i < ASSOC_ARRAY_FAN_OUT; i++) {
		int x = ops->diff_objects(assoc_array_ptr_to_leaf(node->slots[i]),
					  index_key);
		if (x < diff) {
			BUG_ON(x < 0);
			diff = x;
		}
	}