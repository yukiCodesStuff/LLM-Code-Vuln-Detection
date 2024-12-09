	[BPF_MAP_TYPE_XSKMAP]           = "xskmap",
	[BPF_MAP_TYPE_SOCKHASH]		= "sockhash",
	[BPF_MAP_TYPE_CGROUP_STORAGE]	= "cgroup_storage",
	[BPF_MAP_TYPE_REUSEPORT_SOCKARRAY] = "reuseport_sockarray",
};

static bool map_is_per_cpu(__u32 type)
{
	if (fd < 0)
		return -1;

	key = malloc(info.key_size);
	value = alloc_value(&info);
	if (!key || !value) {
		p_err("mem alloc failed");
				} else {
					print_entry_plain(&info, key, value);
				}
			num_elems++;
		} else if (!map_is_map_of_maps(info.type) &&
			   !map_is_map_of_progs(info.type)) {
			if (json_output) {
				jsonw_name(json_wtr, "key");
				print_hex_data_json(key, info.key_size);
				jsonw_name(json_wtr, "value");
		}

		prev_key = key;
	}

	if (json_output)
		jsonw_end_array(json_wtr);