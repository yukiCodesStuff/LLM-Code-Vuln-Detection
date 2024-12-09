static const char * const map_type_name[] = {
	[BPF_MAP_TYPE_UNSPEC]		= "unspec",
	[BPF_MAP_TYPE_HASH]		= "hash",
	[BPF_MAP_TYPE_ARRAY]		= "array",
	[BPF_MAP_TYPE_PROG_ARRAY]	= "prog_array",
	[BPF_MAP_TYPE_PERF_EVENT_ARRAY]	= "perf_event_array",
	[BPF_MAP_TYPE_PERCPU_HASH]	= "percpu_hash",
	[BPF_MAP_TYPE_PERCPU_ARRAY]	= "percpu_array",
	[BPF_MAP_TYPE_STACK_TRACE]	= "stack_trace",
	[BPF_MAP_TYPE_CGROUP_ARRAY]	= "cgroup_array",
	[BPF_MAP_TYPE_LRU_HASH]		= "lru_hash",
	[BPF_MAP_TYPE_LRU_PERCPU_HASH]	= "lru_percpu_hash",
	[BPF_MAP_TYPE_LPM_TRIE]		= "lpm_trie",
	[BPF_MAP_TYPE_ARRAY_OF_MAPS]	= "array_of_maps",
	[BPF_MAP_TYPE_HASH_OF_MAPS]	= "hash_of_maps",
	[BPF_MAP_TYPE_DEVMAP]		= "devmap",
	[BPF_MAP_TYPE_SOCKMAP]		= "sockmap",
	[BPF_MAP_TYPE_CPUMAP]		= "cpumap",
	[BPF_MAP_TYPE_XSKMAP]           = "xskmap",
	[BPF_MAP_TYPE_SOCKHASH]		= "sockhash",
	[BPF_MAP_TYPE_CGROUP_STORAGE]	= "cgroup_storage",
	[BPF_MAP_TYPE_REUSEPORT_SOCKARRAY] = "reuseport_sockarray",
};

static bool map_is_per_cpu(__u32 type)
{
	return type == BPF_MAP_TYPE_PERCPU_HASH ||
	       type == BPF_MAP_TYPE_PERCPU_ARRAY ||
	       type == BPF_MAP_TYPE_LRU_PERCPU_HASH;
}
{
	struct bpf_map_info info = {};
	void *key, *value, *prev_key;
	unsigned int num_elems = 0;
	__u32 len = sizeof(info);
	json_writer_t *btf_wtr;
	struct btf *btf = NULL;
	int err;
	int fd;

	if (argc != 2)
		usage();

	fd = map_parse_fd_and_info(&argc, &argv, &info, &len);
	if (fd < 0)
		return -1;

	key = malloc(info.key_size);
	value = alloc_value(&info);
	if (!key || !value) {
		p_err("mem alloc failed");
		err = -1;
		goto exit_free;
	}
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
				jsonw_start_object(json_wtr);
				jsonw_string_field(json_wtr, "error",
						   "can't lookup element");
				jsonw_end_object(json_wtr);
			} else {
				p_info("can't lookup element with key: ");
				fprint_hex(stderr, key, info.key_size, " ");
				fprintf(stderr, "\n");
			}
		}
			} else {
				p_info("can't lookup element with key: ");
				fprint_hex(stderr, key, info.key_size, " ");
				fprintf(stderr, "\n");
			}
		}

		prev_key = key;
	}

	if (json_output)
		jsonw_end_array(json_wtr);
	else if (btf) {
		jsonw_end_array(btf_wtr);
		jsonw_destroy(&btf_wtr);
	} else {
		printf("Found %u element%s\n", num_elems,
		       num_elems != 1 ? "s" : "");
	}