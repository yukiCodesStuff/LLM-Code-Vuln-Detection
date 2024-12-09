	struct mei_cl_cb *cb, *next;

	list_for_each_entry_safe(cb, next, head, list) {
		if (cl == cb->cl)
			list_del_init(&cb->list);
	}
}

/**