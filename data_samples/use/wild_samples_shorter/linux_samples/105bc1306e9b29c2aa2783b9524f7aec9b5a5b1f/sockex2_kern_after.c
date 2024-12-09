	__be16 h_vlan_encapsulated_proto;
};

struct flow_key_record {
	__be32 src;
	__be32 dst;
	union {
		__be32 ports;
}

static inline __u64 parse_ip(struct __sk_buff *skb, __u64 nhoff, __u64 *ip_proto,
			     struct flow_key_record *flow)
{
	__u64 verlen;

	if (unlikely(ip_is_fragment(skb, nhoff)))
}

static inline __u64 parse_ipv6(struct __sk_buff *skb, __u64 nhoff, __u64 *ip_proto,
			       struct flow_key_record *flow)
{
	*ip_proto = load_byte(skb,
			      nhoff + offsetof(struct ipv6hdr, nexthdr));
	flow->src = ipv6_addr_hash(skb,
	return nhoff;
}

static inline bool flow_dissector(struct __sk_buff *skb,
				  struct flow_key_record *flow)
{
	__u64 nhoff = ETH_HLEN;
	__u64 ip_proto;
	__u64 proto = load_half(skb, 12);
SEC("socket2")
int bpf_prog2(struct __sk_buff *skb)
{
	struct flow_key_record flow = {};
	struct pair *value;
	u32 key;

	if (!flow_dissector(skb, &flow))