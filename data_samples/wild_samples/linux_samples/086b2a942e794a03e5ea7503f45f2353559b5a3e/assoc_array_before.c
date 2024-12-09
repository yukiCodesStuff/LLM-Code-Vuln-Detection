/* Generic associative array implementation.
 *
 * See Documentation/assoc_array.txt for information.
 *
 * Copyright (C) 2013 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version
 * 2 of the Licence, or (at your option) any later version.
 */
//#define DEBUG
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/assoc_array_priv.h>

/*
 * Iterate over an associative array.  The caller must hold the RCU read lock
 * or better.
 */
static int assoc_array_subtree_iterate(const struct assoc_array_ptr *root,
				       const struct assoc_array_ptr *stop,
				       int (*iterator)(const void *leaf,
						       void *iterator_data),
				       void *iterator_data)
{
	const struct assoc_array_shortcut *shortcut;
	const struct assoc_array_node *node;
	const struct assoc_array_ptr *cursor, *ptr, *parent;
	unsigned long has_meta;
	int slot, ret;

	cursor = root;

begin_node:
	if (assoc_array_ptr_is_shortcut(cursor)) {
		/* Descend through a shortcut */
		shortcut = assoc_array_ptr_to_shortcut(cursor);
		smp_read_barrier_depends();
		cursor = ACCESS_ONCE(shortcut->next_node);
	}

	node = assoc_array_ptr_to_node(cursor);
	smp_read_barrier_depends();
	slot = 0;

	/* We perform two passes of each node.
	 *
	 * The first pass does all the leaves in this node.  This means we
	 * don't miss any leaves if the node is split up by insertion whilst
	 * we're iterating over the branches rooted here (we may, however, see
	 * some leaves twice).
	 */
	has_meta = 0;
	for (; slot < ASSOC_ARRAY_FAN_OUT; slot++) {
		ptr = ACCESS_ONCE(node->slots[slot]);
		has_meta |= (unsigned long)ptr;
		if (ptr && assoc_array_ptr_is_leaf(ptr)) {
			/* We need a barrier between the read of the pointer
			 * and dereferencing the pointer - but only if we are
			 * actually going to dereference it.
			 */
			smp_read_barrier_depends();

			/* Invoke the callback */
			ret = iterator(assoc_array_ptr_to_leaf(ptr),
				       iterator_data);
			if (ret)
				return ret;
		}
	}

	/* The second pass attends to all the metadata pointers.  If we follow
	 * one of these we may find that we don't come back here, but rather go
	 * back to a replacement node with the leaves in a different layout.
	 *
	 * We are guaranteed to make progress, however, as the slot number for
	 * a particular portion of the key space cannot change - and we
	 * continue at the back pointer + 1.
	 */
	if (!(has_meta & ASSOC_ARRAY_PTR_META_TYPE))
		goto finished_node;
	slot = 0;

continue_node:
	node = assoc_array_ptr_to_node(cursor);
	smp_read_barrier_depends();

	for (; slot < ASSOC_ARRAY_FAN_OUT; slot++) {
		ptr = ACCESS_ONCE(node->slots[slot]);
		if (assoc_array_ptr_is_meta(ptr)) {
			cursor = ptr;
			goto begin_node;
		}
	}

finished_node:
	/* Move up to the parent (may need to skip back over a shortcut) */
	parent = ACCESS_ONCE(node->back_pointer);
	slot = node->parent_slot;
	if (parent == stop)
		return 0;

	if (assoc_array_ptr_is_shortcut(parent)) {
		shortcut = assoc_array_ptr_to_shortcut(parent);
		smp_read_barrier_depends();
		cursor = parent;
		parent = ACCESS_ONCE(shortcut->back_pointer);
		slot = shortcut->parent_slot;
		if (parent == stop)
			return 0;
	}

	/* Ascend to next slot in parent node */
	cursor = parent;
	slot++;
	goto continue_node;
}