		fd_array_map_delete_elem(map, &i);
}

static void prog_array_map_seq_show_elem(struct bpf_map *map, void *key,
					 struct seq_file *m)
{
	void **elem, *ptr;
	u32 prog_id;

	rcu_read_lock();

	elem = array_map_lookup_elem(map, key);
	if (elem) {
		ptr = READ_ONCE(*elem);
		if (ptr) {
			seq_printf(m, "%u: ", *(u32 *)key);
			prog_id = prog_fd_array_sys_lookup_elem(ptr);
			btf_type_seq_show(map->btf, map->btf_value_type_id,
					  &prog_id, m);
			seq_puts(m, "\n");
		}
	}

	rcu_read_unlock();
}

const struct bpf_map_ops prog_array_map_ops = {
	.map_alloc_check = fd_array_map_alloc_check,
	.map_alloc = array_map_alloc,
	.map_free = fd_array_map_free,
	.map_fd_put_ptr = prog_fd_array_put_ptr,
	.map_fd_sys_lookup_elem = prog_fd_array_sys_lookup_elem,
	.map_release_uref = bpf_fd_array_map_clear,
	.map_seq_show_elem = prog_array_map_seq_show_elem,
};

static struct bpf_event_entry *bpf_event_entry_gen(struct file *perf_file,
						   struct file *map_file)