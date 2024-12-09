		shortcut = assoc_array_ptr_to_shortcut(ptr);
		slot = shortcut->parent_slot;
		cursor = shortcut->back_pointer;
	} else {
		slot = node->parent_slot;
		cursor = ptr;
	}
	BUG_ON(!ptr);
	node = assoc_array_ptr_to_node(cursor);
	slot++;
	goto continue_node;
