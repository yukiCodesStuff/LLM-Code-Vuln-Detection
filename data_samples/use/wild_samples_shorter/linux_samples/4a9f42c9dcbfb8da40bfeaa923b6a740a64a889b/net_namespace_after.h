struct net_generic;
struct uevent_sock;
struct netns_ipvs;
struct bpf_prog;


#define NETDEV_HASHBITS    8
#define NETDEV_HASHENTRIES (1 << NETDEV_HASHBITS)
#endif
	struct net_generic __rcu	*gen;

	struct bpf_prog __rcu	*flow_dissector_prog;

	/* Note : following structs are cache line aligned */
#ifdef CONFIG_XFRM
	struct netns_xfrm	xfrm;
#endif