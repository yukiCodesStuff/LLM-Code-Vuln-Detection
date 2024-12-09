	__be16 h_vlan_encapsulated_proto;
};

struct bpf_flow_keys {
	__be32 src;
	__be32 dst;
	union {
		__be32 ports;
}

struct globals {
	struct bpf_flow_keys flow;
};

struct bpf_map_def SEC("maps") percpu_map = {
	.type = BPF_MAP_TYPE_ARRAY,

struct bpf_map_def SEC("maps") hash_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(struct bpf_flow_keys),
	.value_size = sizeof(struct pair),
	.max_entries = 1024,
};

static void update_stats(struct __sk_buff *skb, struct globals *g)
{
	struct bpf_flow_keys key = g->flow;
	struct pair *value;

	value = bpf_map_lookup_elem(&hash_map, &key);
	if (value) {