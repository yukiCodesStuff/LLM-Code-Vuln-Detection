struct pipe_inode_info;
struct iov_iter;
struct napi_struct;
struct bpf_prog;
union bpf_attr;

#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
struct nf_conntrack {
	atomic_t use;
			     const struct flow_dissector_key *key,
			     unsigned int key_count);

#ifdef CONFIG_NET
int skb_flow_dissector_bpf_prog_attach(const union bpf_attr *attr,
				       struct bpf_prog *prog);

int skb_flow_dissector_bpf_prog_detach(const union bpf_attr *attr);
#else
static inline int skb_flow_dissector_bpf_prog_attach(const union bpf_attr *attr,
						     struct bpf_prog *prog)
{
	return -EOPNOTSUPP;
}

static inline int skb_flow_dissector_bpf_prog_detach(const union bpf_attr *attr)
{
	return -EOPNOTSUPP;
}
#endif

bool __skb_flow_dissect(const struct sk_buff *skb,
			struct flow_dissector *flow_dissector,
			void *target_container,
			void *data, __be16 proto, int nhoff, int hlen,