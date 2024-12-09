	[BPF_MAP_TYPE_XSKMAP]           = "xskmap",
	[BPF_MAP_TYPE_SOCKHASH]		= "sockhash",
	[BPF_MAP_TYPE_CGROUP_STORAGE]	= "cgroup_storage",
};

static bool map_is_per_cpu(__u32 type)
{
	if (fd < 0)
		return -1;

	if (map_is_map_of_maps(info.type) || map_is_map_of_progs(info.type)) {
		p_err("Dumping maps of maps and program maps not supported");
		close(fd);
		return -1;
	}

	key = malloc(info.key_size);
	value = alloc_value(&info);
	if (!key || !value) {
		p_err("mem alloc failed");
				} else {
					print_entry_plain(&info, key, value);
				}
		} else {
			if (json_output) {
				jsonw_name(json_wtr, "key");
				print_hex_data_json(key, info.key_size);
				jsonw_name(json_wtr, "value");
		}

		prev_key = key;
		num_elems++;
	}

	if (json_output)
		jsonw_end_array(json_wtr);