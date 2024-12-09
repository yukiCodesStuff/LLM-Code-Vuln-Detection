struct napi_gro_cb {
	/* Virtual address of skb_shinfo(skb)->frags[0].page + offset. */
	void	*frag0;

	/* Length of frag0. */
	unsigned int frag0_len;

	/* This indicates where we are processing relative to skb->data. */
	int	data_offset;

	/* This is non-zero if the packet cannot be merged with the new skb. */
	u16	flush;

	/* Save the IP ID here and check when we get to the transport layer */
	u16	flush_id;

	/* Number of segments aggregated. */
	u16	count;

	/* Start offset for remote checksum offload */
	u16	gro_remcsum_start;

	/* jiffies when first packet was created/queued */
	unsigned long age;

	/* Used in ipv6_gro_receive() and foo-over-udp */
	u16	proto;

	/* This is non-zero if the packet may be of the same flow. */
	u8	same_flow:1;

	/* Used in tunnel GRO receive */
	u8	encap_mark:1;

	/* GRO checksum is valid */
	u8	csum_valid:1;

	/* Number of checksums via CHECKSUM_UNNECESSARY */
	u8	csum_cnt:3;

	/* Free the skb? */
	u8	free:2;
#define NAPI_GRO_FREE		  1
#define NAPI_GRO_FREE_STOLEN_HEAD 2

	/* Used in foo-over-udp, set in udp[46]_gro_receive */
	u8	is_ipv6:1;

	/* Used in GRE, set in fou/gue_gro_receive */
	u8	is_fou:1;

	/* Used to determine if flush_id can be ignored */
	u8	is_atomic:1;

	/* 5 bit hole */

	/* used to support CHECKSUM_COMPLETE for tunneling protocols */
	__wsum	csum;

	/* used in skb_gro_receive() slow path */
	struct sk_buff *last;
};

#define NAPI_GRO_CB(skb) ((struct napi_gro_cb *)(skb)->cb)

struct packet_type {
	__be16			type;	/* This is really htons(ether_type). */
	struct net_device	*dev;	/* NULL is wildcarded here	     */
	int			(*func) (struct sk_buff *,
					 struct net_device *,
					 struct packet_type *,
					 struct net_device *);
	bool			(*id_match)(struct packet_type *ptype,
					    struct sock *sk);
	void			*af_packet_priv;
	struct list_head	list;
};

struct offload_callbacks {
	struct sk_buff		*(*gso_segment)(struct sk_buff *skb,
						netdev_features_t features);
	struct sk_buff		**(*gro_receive)(struct sk_buff **head,
						 struct sk_buff *skb);
	int			(*gro_complete)(struct sk_buff *skb, int nhoff);
};

struct packet_offload {
	__be16			 type;	/* This is really htons(ether_type). */
	u16			 priority;
	struct offload_callbacks callbacks;
	struct list_head	 list;
};

/* often modified stats are per-CPU, other are shared (netdev->stats) */
struct pcpu_sw_netstats {
	u64     rx_packets;
	u64     rx_bytes;
	u64     tx_packets;
	u64     tx_bytes;
	struct u64_stats_sync   syncp;
};

#define __netdev_alloc_pcpu_stats(type, gfp)				\
({									\
	typeof(type) __percpu *pcpu_stats = alloc_percpu_gfp(type, gfp);\
	if (pcpu_stats)	{						\
		int __cpu;						\
		for_each_possible_cpu(__cpu) {				\
			typeof(type) *stat;				\
			stat = per_cpu_ptr(pcpu_stats, __cpu);		\
			u64_stats_init(&stat->syncp);			\
		}							\
	}								\
	pcpu_stats;							\
})

#define netdev_alloc_pcpu_stats(type)					\
	__netdev_alloc_pcpu_stats(type, GFP_KERNEL)

enum netdev_lag_tx_type {
	NETDEV_LAG_TX_TYPE_UNKNOWN,
	NETDEV_LAG_TX_TYPE_RANDOM,
	NETDEV_LAG_TX_TYPE_BROADCAST,
	NETDEV_LAG_TX_TYPE_ROUNDROBIN,
	NETDEV_LAG_TX_TYPE_ACTIVEBACKUP,
	NETDEV_LAG_TX_TYPE_HASH,
};

struct netdev_lag_upper_info {
	enum netdev_lag_tx_type tx_type;
};

struct netdev_lag_lower_state_info {
	u8 link_up : 1,
	   tx_enabled : 1;
};

#include <linux/notifier.h>

/* netdevice notifier chain. Please remember to update the rtnetlink
 * notification exclusion list in rtnetlink_event() when adding new
 * types.
 */
#define NETDEV_UP	0x0001	/* For now you can't veto a device up/down */
#define NETDEV_DOWN	0x0002
#define NETDEV_REBOOT	0x0003	/* Tell a protocol stack a network interface
				   detected a hardware crash and restarted
				   - we can use this eg to kick tcp sessions
				   once done */
#define NETDEV_CHANGE	0x0004	/* Notify device state change */
#define NETDEV_REGISTER 0x0005
#define NETDEV_UNREGISTER	0x0006
#define NETDEV_CHANGEMTU	0x0007 /* notify after mtu change happened */
#define NETDEV_CHANGEADDR	0x0008
#define NETDEV_GOING_DOWN	0x0009
#define NETDEV_CHANGENAME	0x000A
#define NETDEV_FEAT_CHANGE	0x000B
#define NETDEV_BONDING_FAILOVER 0x000C
#define NETDEV_PRE_UP		0x000D
#define NETDEV_PRE_TYPE_CHANGE	0x000E
#define NETDEV_POST_TYPE_CHANGE	0x000F
#define NETDEV_POST_INIT	0x0010
#define NETDEV_UNREGISTER_FINAL 0x0011
#define NETDEV_RELEASE		0x0012
#define NETDEV_NOTIFY_PEERS	0x0013
#define NETDEV_JOIN		0x0014
#define NETDEV_CHANGEUPPER	0x0015
#define NETDEV_RESEND_IGMP	0x0016
#define NETDEV_PRECHANGEMTU	0x0017 /* notify before mtu change happened */
#define NETDEV_CHANGEINFODATA	0x0018
#define NETDEV_BONDING_INFO	0x0019
#define NETDEV_PRECHANGEUPPER	0x001A
#define NETDEV_CHANGELOWERSTATE	0x001B
#define NETDEV_UDP_TUNNEL_PUSH_INFO	0x001C
#define NETDEV_CHANGE_TX_QUEUE_LEN	0x001E

int register_netdevice_notifier(struct notifier_block *nb);
int unregister_netdevice_notifier(struct notifier_block *nb);

struct netdev_notifier_info {
	struct net_device *dev;
};

struct netdev_notifier_change_info {
	struct netdev_notifier_info info; /* must be first */
	unsigned int flags_changed;
};

struct netdev_notifier_changeupper_info {
	struct netdev_notifier_info info; /* must be first */
	struct net_device *upper_dev; /* new upper dev */
	bool master; /* is upper dev master */
	bool linking; /* is the notification for link or unlink */
	void *upper_info; /* upper dev info */
};

struct netdev_notifier_changelowerstate_info {
	struct netdev_notifier_info info; /* must be first */
	void *lower_state_info; /* is lower dev state */
};

static inline void netdev_notifier_info_init(struct netdev_notifier_info *info,
					     struct net_device *dev)
{
	info->dev = dev;
}

static inline struct net_device *
netdev_notifier_info_to_dev(const struct netdev_notifier_info *info)
{
	return info->dev;
}

int call_netdevice_notifiers(unsigned long val, struct net_device *dev);


extern rwlock_t				dev_base_lock;		/* Device list lock */

#define for_each_netdev(net, d)		\
		list_for_each_entry(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_reverse(net, d)	\
		list_for_each_entry_reverse(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_rcu(net, d)		\
		list_for_each_entry_rcu(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_safe(net, d, n)	\
		list_for_each_entry_safe(d, n, &(net)->dev_base_head, dev_list)
#define for_each_netdev_continue(net, d)		\
		list_for_each_entry_continue(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_continue_rcu(net, d)		\
	list_for_each_entry_continue_rcu(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_in_bond_rcu(bond, slave)	\
		for_each_netdev_rcu(&init_net, slave)	\
			if (netdev_master_upper_dev_get_rcu(slave) == (bond))
#define net_device_entry(lh)	list_entry(lh, struct net_device, dev_list)

static inline struct net_device *next_net_device(struct net_device *dev)
{
	struct list_head *lh;
	struct net *net;

	net = dev_net(dev);
	lh = dev->dev_list.next;
	return lh == &net->dev_base_head ? NULL : net_device_entry(lh);
}

static inline struct net_device *next_net_device_rcu(struct net_device *dev)
{
	struct list_head *lh;
	struct net *net;

	net = dev_net(dev);
	lh = rcu_dereference(list_next_rcu(&dev->dev_list));
	return lh == &net->dev_base_head ? NULL : net_device_entry(lh);
}

static inline struct net_device *first_net_device(struct net *net)
{
	return list_empty(&net->dev_base_head) ? NULL :
		net_device_entry(net->dev_base_head.next);
}

static inline struct net_device *first_net_device_rcu(struct net *net)
{
	struct list_head *lh = rcu_dereference(list_next_rcu(&net->dev_base_head));

	return lh == &net->dev_base_head ? NULL : net_device_entry(lh);
}

int netdev_boot_setup_check(struct net_device *dev);
unsigned long netdev_boot_base(const char *prefix, int unit);
struct net_device *dev_getbyhwaddr_rcu(struct net *net, unsigned short type,
				       const char *hwaddr);
struct net_device *dev_getfirstbyhwtype(struct net *net, unsigned short type);
struct net_device *__dev_getfirstbyhwtype(struct net *net, unsigned short type);
void dev_add_pack(struct packet_type *pt);
void dev_remove_pack(struct packet_type *pt);
void __dev_remove_pack(struct packet_type *pt);
void dev_add_offload(struct packet_offload *po);
void dev_remove_offload(struct packet_offload *po);

int dev_get_iflink(const struct net_device *dev);
int dev_fill_metadata_dst(struct net_device *dev, struct sk_buff *skb);
struct net_device *__dev_get_by_flags(struct net *net, unsigned short flags,
				      unsigned short mask);
struct net_device *dev_get_by_name(struct net *net, const char *name);
struct net_device *dev_get_by_name_rcu(struct net *net, const char *name);
struct net_device *__dev_get_by_name(struct net *net, const char *name);
int dev_alloc_name(struct net_device *dev, const char *name);
int dev_open(struct net_device *dev);
int dev_close(struct net_device *dev);
int dev_close_many(struct list_head *head, bool unlink);
void dev_disable_lro(struct net_device *dev);
int dev_loopback_xmit(struct net *net, struct sock *sk, struct sk_buff *newskb);
int dev_queue_xmit(struct sk_buff *skb);
int dev_queue_xmit_accel(struct sk_buff *skb, void *accel_priv);
int register_netdevice(struct net_device *dev);
void unregister_netdevice_queue(struct net_device *dev, struct list_head *head);
void unregister_netdevice_many(struct list_head *head);
static inline void unregister_netdevice(struct net_device *dev)
{
	unregister_netdevice_queue(dev, NULL);
}

int netdev_refcnt_read(const struct net_device *dev);
void free_netdev(struct net_device *dev);
void netdev_freemem(struct net_device *dev);
void synchronize_net(void);
int init_dummy_netdev(struct net_device *dev);

DECLARE_PER_CPU(int, xmit_recursion);
#define XMIT_RECURSION_LIMIT	10

static inline int dev_recursion_level(void)
{
	return this_cpu_read(xmit_recursion);
}

struct net_device *dev_get_by_index(struct net *net, int ifindex);
struct net_device *__dev_get_by_index(struct net *net, int ifindex);
struct net_device *dev_get_by_index_rcu(struct net *net, int ifindex);
int netdev_get_name(struct net *net, char *name, int ifindex);
int dev_restart(struct net_device *dev);
int skb_gro_receive(struct sk_buff **head, struct sk_buff *skb);

static inline unsigned int skb_gro_offset(const struct sk_buff *skb)
{
	return NAPI_GRO_CB(skb)->data_offset;
}

static inline unsigned int skb_gro_len(const struct sk_buff *skb)
{
	return skb->len - NAPI_GRO_CB(skb)->data_offset;
}

static inline void skb_gro_pull(struct sk_buff *skb, unsigned int len)
{
	NAPI_GRO_CB(skb)->data_offset += len;
}

static inline void *skb_gro_header_fast(struct sk_buff *skb,
					unsigned int offset)
{
	return NAPI_GRO_CB(skb)->frag0 + offset;
}

static inline int skb_gro_header_hard(struct sk_buff *skb, unsigned int hlen)
{
	return NAPI_GRO_CB(skb)->frag0_len < hlen;
}

static inline void *skb_gro_header_slow(struct sk_buff *skb, unsigned int hlen,
					unsigned int offset)
{
	if (!pskb_may_pull(skb, hlen))
		return NULL;

	NAPI_GRO_CB(skb)->frag0 = NULL;
	NAPI_GRO_CB(skb)->frag0_len = 0;
	return skb->data + offset;
}

static inline void *skb_gro_network_header(struct sk_buff *skb)
{
	return (NAPI_GRO_CB(skb)->frag0 ?: skb->data) +
	       skb_network_offset(skb);
}

static inline void skb_gro_postpull_rcsum(struct sk_buff *skb,
					const void *start, unsigned int len)
{
	if (NAPI_GRO_CB(skb)->csum_valid)
		NAPI_GRO_CB(skb)->csum = csum_sub(NAPI_GRO_CB(skb)->csum,
						  csum_partial(start, len, 0));
}

/* GRO checksum functions. These are logical equivalents of the normal
 * checksum functions (in skbuff.h) except that they operate on the GRO
 * offsets and fields in sk_buff.
 */

__sum16 __skb_gro_checksum_complete(struct sk_buff *skb);

static inline bool skb_at_gro_remcsum_start(struct sk_buff *skb)
{
	return (NAPI_GRO_CB(skb)->gro_remcsum_start == skb_gro_offset(skb));
}

static inline bool __skb_gro_checksum_validate_needed(struct sk_buff *skb,
						      bool zero_okay,
						      __sum16 check)
{
	return ((skb->ip_summed != CHECKSUM_PARTIAL ||
		skb_checksum_start_offset(skb) <
		 skb_gro_offset(skb)) &&
		!skb_at_gro_remcsum_start(skb) &&
		NAPI_GRO_CB(skb)->csum_cnt == 0 &&
		(!zero_okay || check));
}

static inline __sum16 __skb_gro_checksum_validate_complete(struct sk_buff *skb,
							   __wsum psum)
{
	if (NAPI_GRO_CB(skb)->csum_valid &&
	    !csum_fold(csum_add(psum, NAPI_GRO_CB(skb)->csum)))
		return 0;

	NAPI_GRO_CB(skb)->csum = psum;

	return __skb_gro_checksum_complete(skb);
}

static inline void skb_gro_incr_csum_unnecessary(struct sk_buff *skb)
{
	if (NAPI_GRO_CB(skb)->csum_cnt > 0) {
		/* Consume a checksum from CHECKSUM_UNNECESSARY */
		NAPI_GRO_CB(skb)->csum_cnt--;
	} else {
		/* Update skb for CHECKSUM_UNNECESSARY and csum_level when we
		 * verified a new top level checksum or an encapsulated one
		 * during GRO. This saves work if we fallback to normal path.
		 */
		__skb_incr_checksum_unnecessary(skb);
	}
}

#define __skb_gro_checksum_validate(skb, proto, zero_okay, check,	\
				    compute_pseudo)			\
({									\
	__sum16 __ret = 0;						\
	if (__skb_gro_checksum_validate_needed(skb, zero_okay, check))	\
		__ret = __skb_gro_checksum_validate_complete(skb,	\
				compute_pseudo(skb, proto));		\
	if (__ret)							\
		__skb_mark_checksum_bad(skb);				\
	else								\
		skb_gro_incr_csum_unnecessary(skb);			\
	__ret;								\
})

#define skb_gro_checksum_validate(skb, proto, compute_pseudo)		\
	__skb_gro_checksum_validate(skb, proto, false, 0, compute_pseudo)

#define skb_gro_checksum_validate_zero_check(skb, proto, check,		\
					     compute_pseudo)		\
	__skb_gro_checksum_validate(skb, proto, true, check, compute_pseudo)

#define skb_gro_checksum_simple_validate(skb)				\
	__skb_gro_checksum_validate(skb, 0, false, 0, null_compute_pseudo)

static inline bool __skb_gro_checksum_convert_check(struct sk_buff *skb)
{
	return (NAPI_GRO_CB(skb)->csum_cnt == 0 &&
		!NAPI_GRO_CB(skb)->csum_valid);
}

static inline void __skb_gro_checksum_convert(struct sk_buff *skb,
					      __sum16 check, __wsum pseudo)
{
	NAPI_GRO_CB(skb)->csum = ~pseudo;
	NAPI_GRO_CB(skb)->csum_valid = 1;
}

#define skb_gro_checksum_try_convert(skb, proto, check, compute_pseudo)	\
do {									\
	if (__skb_gro_checksum_convert_check(skb))			\
		__skb_gro_checksum_convert(skb, check,			\
					   compute_pseudo(skb, proto));	\
} while (0)

struct gro_remcsum {
	int offset;
	__wsum delta;
};

static inline void skb_gro_remcsum_init(struct gro_remcsum *grc)
{
	grc->offset = 0;
	grc->delta = 0;
}

static inline void *skb_gro_remcsum_process(struct sk_buff *skb, void *ptr,
					    unsigned int off, size_t hdrlen,
					    int start, int offset,
					    struct gro_remcsum *grc,
					    bool nopartial)
{
	__wsum delta;
	size_t plen = hdrlen + max_t(size_t, offset + sizeof(u16), start);

	BUG_ON(!NAPI_GRO_CB(skb)->csum_valid);

	if (!nopartial) {
		NAPI_GRO_CB(skb)->gro_remcsum_start = off + hdrlen + start;
		return ptr;
	}

	ptr = skb_gro_header_fast(skb, off);
	if (skb_gro_header_hard(skb, off + plen)) {
		ptr = skb_gro_header_slow(skb, off + plen, off);
		if (!ptr)
			return NULL;
	}

	delta = remcsum_adjust(ptr + hdrlen, NAPI_GRO_CB(skb)->csum,
			       start, offset);

	/* Adjust skb->csum since we changed the packet */
	NAPI_GRO_CB(skb)->csum = csum_add(NAPI_GRO_CB(skb)->csum, delta);

	grc->offset = off + hdrlen + offset;
	grc->delta = delta;

	return ptr;
}

static inline void skb_gro_remcsum_cleanup(struct sk_buff *skb,
					   struct gro_remcsum *grc)
{
	void *ptr;
	size_t plen = grc->offset + sizeof(u16);

	if (!grc->delta)
		return;

	ptr = skb_gro_header_fast(skb, grc->offset);
	if (skb_gro_header_hard(skb, grc->offset + sizeof(u16))) {
		ptr = skb_gro_header_slow(skb, plen, grc->offset);
		if (!ptr)
			return;
	}

	remcsum_unadjust((__sum16 *)ptr, grc->delta);
}

struct skb_csum_offl_spec {
	__u16		ipv4_okay:1,
			ipv6_okay:1,
			encap_okay:1,
			ip_options_okay:1,
			ext_hdrs_okay:1,
			tcp_okay:1,
			udp_okay:1,
			sctp_okay:1,
			vlan_okay:1,
			no_encapped_ipv6:1,
			no_not_encapped:1;
};

bool __skb_csum_offload_chk(struct sk_buff *skb,
			    const struct skb_csum_offl_spec *spec,
			    bool *csum_encapped,
			    bool csum_help);

static inline bool skb_csum_offload_chk(struct sk_buff *skb,
					const struct skb_csum_offl_spec *spec,
					bool *csum_encapped,
					bool csum_help)
{
	if (skb->ip_summed != CHECKSUM_PARTIAL)
		return false;

	return __skb_csum_offload_chk(skb, spec, csum_encapped, csum_help);
}

static inline bool skb_csum_offload_chk_help(struct sk_buff *skb,
					     const struct skb_csum_offl_spec *spec)
{
	bool csum_encapped;

	return skb_csum_offload_chk(skb, spec, &csum_encapped, true);
}

static inline bool skb_csum_off_chk_help_cmn(struct sk_buff *skb)
{
	static const struct skb_csum_offl_spec csum_offl_spec = {
		.ipv4_okay = 1,
		.ip_options_okay = 1,
		.ipv6_okay = 1,
		.vlan_okay = 1,
		.tcp_okay = 1,
		.udp_okay = 1,
	};

	return skb_csum_offload_chk_help(skb, &csum_offl_spec);
}

static inline bool skb_csum_off_chk_help_cmn_v4_only(struct sk_buff *skb)
{
	static const struct skb_csum_offl_spec csum_offl_spec = {
		.ipv4_okay = 1,
		.ip_options_okay = 1,
		.tcp_okay = 1,
		.udp_okay = 1,
		.vlan_okay = 1,
	};

	return skb_csum_offload_chk_help(skb, &csum_offl_spec);
}

static inline int dev_hard_header(struct sk_buff *skb, struct net_device *dev,
				  unsigned short type,
				  const void *daddr, const void *saddr,
				  unsigned int len)
{
	if (!dev->header_ops || !dev->header_ops->create)
		return 0;

	return dev->header_ops->create(skb, dev, type, daddr, saddr, len);
}

static inline int dev_parse_header(const struct sk_buff *skb,
				   unsigned char *haddr)
{
	const struct net_device *dev = skb->dev;

	if (!dev->header_ops || !dev->header_ops->parse)
		return 0;
	return dev->header_ops->parse(skb, haddr);
}

/* ll_header must have at least hard_header_len allocated */
static inline bool dev_validate_header(const struct net_device *dev,
				       char *ll_header, int len)
{
	if (likely(len >= dev->hard_header_len))
		return true;

	if (capable(CAP_SYS_RAWIO)) {
		memset(ll_header + len, 0, dev->hard_header_len - len);
		return true;
	}

	if (dev->header_ops && dev->header_ops->validate)
		return dev->header_ops->validate(ll_header, len);

	return false;
}

typedef int gifconf_func_t(struct net_device * dev, char __user * bufptr, int len);
int register_gifconf(unsigned int family, gifconf_func_t *gifconf);
static inline int unregister_gifconf(unsigned int family)
{
	return register_gifconf(family, NULL);
}

#ifdef CONFIG_NET_FLOW_LIMIT
#define FLOW_LIMIT_HISTORY	(1 << 7)  /* must be ^2 and !overflow buckets */
struct sd_flow_limit {
	u64			count;
	unsigned int		num_buckets;
	unsigned int		history_head;
	u16			history[FLOW_LIMIT_HISTORY];
	u8			buckets[];
};

extern int netdev_flow_limit_table_len;
#endif /* CONFIG_NET_FLOW_LIMIT */

/*
 * Incoming packets are placed on per-CPU queues
 */
struct softnet_data {
	struct list_head	poll_list;
	struct sk_buff_head	process_queue;

	/* stats */
	unsigned int		processed;
	unsigned int		time_squeeze;
	unsigned int		received_rps;
#ifdef CONFIG_RPS
	struct softnet_data	*rps_ipi_list;
#endif
#ifdef CONFIG_NET_FLOW_LIMIT
	struct sd_flow_limit __rcu *flow_limit;
#endif
	struct Qdisc		*output_queue;
	struct Qdisc		**output_queue_tailp;
	struct sk_buff		*completion_queue;

#ifdef CONFIG_RPS
	/* input_queue_head should be written by cpu owning this struct,
	 * and only read by other cpus. Worth using a cache line.
	 */
	unsigned int		input_queue_head ____cacheline_aligned_in_smp;

	/* Elements below can be accessed between CPUs for RPS/RFS */
	struct call_single_data	csd ____cacheline_aligned_in_smp;
	struct softnet_data	*rps_ipi_next;
	unsigned int		cpu;
	unsigned int		input_queue_tail;
#endif
	unsigned int		dropped;
	struct sk_buff_head	input_pkt_queue;
	struct napi_struct	backlog;

};

static inline void input_queue_head_incr(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	sd->input_queue_head++;
#endif
}

static inline void input_queue_tail_incr_save(struct softnet_data *sd,
					      unsigned int *qtail)
{
#ifdef CONFIG_RPS
	*qtail = ++sd->input_queue_tail;
#endif
}

DECLARE_PER_CPU_ALIGNED(struct softnet_data, softnet_data);

void __netif_schedule(struct Qdisc *q);
void netif_schedule_queue(struct netdev_queue *txq);

static inline void netif_tx_schedule_all(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++)
		netif_schedule_queue(netdev_get_tx_queue(dev, i));
}

static __always_inline void netif_tx_start_queue(struct netdev_queue *dev_queue)
{
	clear_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
}

/**
 *	netif_start_queue - allow transmit
 *	@dev: network device
 *
 *	Allow upper layers to call the device hard_start_xmit routine.
 */
static inline void netif_start_queue(struct net_device *dev)
{
	netif_tx_start_queue(netdev_get_tx_queue(dev, 0));
}

static inline void netif_tx_start_all_queues(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);
		netif_tx_start_queue(txq);
	}
}

void netif_tx_wake_queue(struct netdev_queue *dev_queue);

/**
 *	netif_wake_queue - restart transmit
 *	@dev: network device
 *
 *	Allow upper layers to call the device hard_start_xmit routine.
 *	Used for flow control when transmit resources are available.
 */
static inline void netif_wake_queue(struct net_device *dev)
{
	netif_tx_wake_queue(netdev_get_tx_queue(dev, 0));
}

static inline void netif_tx_wake_all_queues(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);
		netif_tx_wake_queue(txq);
	}
}

static __always_inline void netif_tx_stop_queue(struct netdev_queue *dev_queue)
{
	set_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
}

/**
 *	netif_stop_queue - stop transmitted packets
 *	@dev: network device
 *
 *	Stop upper layers calling the device hard_start_xmit routine.
 *	Used for flow control when transmit resources are unavailable.
 */
static inline void netif_stop_queue(struct net_device *dev)
{
	netif_tx_stop_queue(netdev_get_tx_queue(dev, 0));
}

void netif_tx_stop_all_queues(struct net_device *dev);

static inline bool netif_tx_queue_stopped(const struct netdev_queue *dev_queue)
{
	return test_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
}

/**
 *	netif_queue_stopped - test if transmit queue is flowblocked
 *	@dev: network device
 *
 *	Test if transmit queue on device is currently unable to send.
 */
static inline bool netif_queue_stopped(const struct net_device *dev)
{
	return netif_tx_queue_stopped(netdev_get_tx_queue(dev, 0));
}

static inline bool netif_xmit_stopped(const struct netdev_queue *dev_queue)
{
	return dev_queue->state & QUEUE_STATE_ANY_XOFF;
}

static inline bool
netif_xmit_frozen_or_stopped(const struct netdev_queue *dev_queue)
{
	return dev_queue->state & QUEUE_STATE_ANY_XOFF_OR_FROZEN;
}

static inline bool
netif_xmit_frozen_or_drv_stopped(const struct netdev_queue *dev_queue)
{
	return dev_queue->state & QUEUE_STATE_DRV_XOFF_OR_FROZEN;
}

/**
 *	netdev_txq_bql_enqueue_prefetchw - prefetch bql data for write
 *	@dev_queue: pointer to transmit queue
 *
 * BQL enabled drivers might use this helper in their ndo_start_xmit(),
 * to give appropriate hint to the CPU.
 */
static inline void netdev_txq_bql_enqueue_prefetchw(struct netdev_queue *dev_queue)
{
#ifdef CONFIG_BQL
	prefetchw(&dev_queue->dql.num_queued);
#endif
}

/**
 *	netdev_txq_bql_complete_prefetchw - prefetch bql data for write
 *	@dev_queue: pointer to transmit queue
 *
 * BQL enabled drivers might use this helper in their TX completion path,
 * to give appropriate hint to the CPU.
 */
static inline void netdev_txq_bql_complete_prefetchw(struct netdev_queue *dev_queue)
{
#ifdef CONFIG_BQL
	prefetchw(&dev_queue->dql.limit);
#endif
}

static inline void netdev_tx_sent_queue(struct netdev_queue *dev_queue,
					unsigned int bytes)
{
#ifdef CONFIG_BQL
	dql_queued(&dev_queue->dql, bytes);

	if (likely(dql_avail(&dev_queue->dql) >= 0))
		return;

	set_bit(__QUEUE_STATE_STACK_XOFF, &dev_queue->state);

	/*
	 * The XOFF flag must be set before checking the dql_avail below,
	 * because in netdev_tx_completed_queue we update the dql_completed
	 * before checking the XOFF flag.
	 */
	smp_mb();

	/* check again in case another CPU has just made room avail */
	if (unlikely(dql_avail(&dev_queue->dql) >= 0))
		clear_bit(__QUEUE_STATE_STACK_XOFF, &dev_queue->state);
#endif
}

/**
 * 	netdev_sent_queue - report the number of bytes queued to hardware
 * 	@dev: network device
 * 	@bytes: number of bytes queued to the hardware device queue
 *
 * 	Report the number of bytes queued for sending/completion to the network
 * 	device hardware queue. @bytes should be a good approximation and should
 * 	exactly match netdev_completed_queue() @bytes
 */
static inline void netdev_sent_queue(struct net_device *dev, unsigned int bytes)
{
	netdev_tx_sent_queue(netdev_get_tx_queue(dev, 0), bytes);
}

static inline void netdev_tx_completed_queue(struct netdev_queue *dev_queue,
					     unsigned int pkts, unsigned int bytes)
{
#ifdef CONFIG_BQL
	if (unlikely(!bytes))
		return;

	dql_completed(&dev_queue->dql, bytes);

	/*
	 * Without the memory barrier there is a small possiblity that
	 * netdev_tx_sent_queue will miss the update and cause the queue to
	 * be stopped forever
	 */
	smp_mb();

	if (dql_avail(&dev_queue->dql) < 0)
		return;

	if (test_and_clear_bit(__QUEUE_STATE_STACK_XOFF, &dev_queue->state))
		netif_schedule_queue(dev_queue);
#endif
}

/**
 * 	netdev_completed_queue - report bytes and packets completed by device
 * 	@dev: network device
 * 	@pkts: actual number of packets sent over the medium
 * 	@bytes: actual number of bytes sent over the medium
 *
 * 	Report the number of bytes and packets transmitted by the network device
 * 	hardware queue over the physical medium, @bytes must exactly match the
 * 	@bytes amount passed to netdev_sent_queue()
 */
static inline void netdev_completed_queue(struct net_device *dev,
					  unsigned int pkts, unsigned int bytes)
{
	netdev_tx_completed_queue(netdev_get_tx_queue(dev, 0), pkts, bytes);
}

static inline void netdev_tx_reset_queue(struct netdev_queue *q)
{
#ifdef CONFIG_BQL
	clear_bit(__QUEUE_STATE_STACK_XOFF, &q->state);
	dql_reset(&q->dql);
#endif
}

/**
 * 	netdev_reset_queue - reset the packets and bytes count of a network device
 * 	@dev_queue: network device
 *
 * 	Reset the bytes and packet count of a network device and clear the
 * 	software flow control OFF bit for this network device
 */
static inline void netdev_reset_queue(struct net_device *dev_queue)
{
	netdev_tx_reset_queue(netdev_get_tx_queue(dev_queue, 0));
}

/**
 * 	netdev_cap_txqueue - check if selected tx queue exceeds device queues
 * 	@dev: network device
 * 	@queue_index: given tx queue index
 *
 * 	Returns 0 if given tx queue index >= number of device tx queues,
 * 	otherwise returns the originally passed tx queue index.
 */
static inline u16 netdev_cap_txqueue(struct net_device *dev, u16 queue_index)
{
	if (unlikely(queue_index >= dev->real_num_tx_queues)) {
		net_warn_ratelimited("%s selects TX queue %d, but real number of TX queues is %d\n",
				     dev->name, queue_index,
				     dev->real_num_tx_queues);
		return 0;
	}

	return queue_index;
}

/**
 *	netif_running - test if up
 *	@dev: network device
 *
 *	Test if the device has been brought up.
 */
static inline bool netif_running(const struct net_device *dev)
{
	return test_bit(__LINK_STATE_START, &dev->state);
}

/*
 * Routines to manage the subqueues on a device.  We only need start,
 * stop, and a check if it's stopped.  All other device management is
 * done at the overall netdevice level.
 * Also test the device if we're multiqueue.
 */

/**
 *	netif_start_subqueue - allow sending packets on subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Start individual transmit queue of a device with multiple transmit queues.
 */
static inline void netif_start_subqueue(struct net_device *dev, u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);

	netif_tx_start_queue(txq);
}

/**
 *	netif_stop_subqueue - stop sending packets on subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Stop individual transmit queue of a device with multiple transmit queues.
 */
static inline void netif_stop_subqueue(struct net_device *dev, u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);
	netif_tx_stop_queue(txq);
}

/**
 *	netif_subqueue_stopped - test status of subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Check individual transmit queue of a device with multiple transmit queues.
 */
static inline bool __netif_subqueue_stopped(const struct net_device *dev,
					    u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);

	return netif_tx_queue_stopped(txq);
}

static inline bool netif_subqueue_stopped(const struct net_device *dev,
					  struct sk_buff *skb)
{
	return __netif_subqueue_stopped(dev, skb_get_queue_mapping(skb));
}

void netif_wake_subqueue(struct net_device *dev, u16 queue_index);

#ifdef CONFIG_XPS
int netif_set_xps_queue(struct net_device *dev, const struct cpumask *mask,
			u16 index);
#else
static inline int netif_set_xps_queue(struct net_device *dev,
				      const struct cpumask *mask,
				      u16 index)
{
	return 0;
}
#endif

u16 __skb_tx_hash(const struct net_device *dev, struct sk_buff *skb,
		  unsigned int num_tx_queues);

/*
 * Returns a Tx hash for the given packet when dev->real_num_tx_queues is used
 * as a distribution range limit for the returned value.
 */
static inline u16 skb_tx_hash(const struct net_device *dev,
			      struct sk_buff *skb)
{
	return __skb_tx_hash(dev, skb, dev->real_num_tx_queues);
}

/**
 *	netif_is_multiqueue - test if device has multiple transmit queues
 *	@dev: network device
 *
 * Check if device has multiple transmit queues
 */
static inline bool netif_is_multiqueue(const struct net_device *dev)
{
	return dev->num_tx_queues > 1;
}

int netif_set_real_num_tx_queues(struct net_device *dev, unsigned int txq);

#ifdef CONFIG_SYSFS
int netif_set_real_num_rx_queues(struct net_device *dev, unsigned int rxq);
#else
static inline int netif_set_real_num_rx_queues(struct net_device *dev,
						unsigned int rxq)
{
	return 0;
}
#endif

#ifdef CONFIG_SYSFS
static inline unsigned int get_netdev_rx_queue_index(
		struct netdev_rx_queue *queue)
{
	struct net_device *dev = queue->dev;
	int index = queue - dev->_rx;

	BUG_ON(index >= dev->num_rx_queues);
	return index;
}
#endif

#define DEFAULT_MAX_NUM_RSS_QUEUES	(8)
int netif_get_num_default_rss_queues(void);

enum skb_free_reason {
	SKB_REASON_CONSUMED,
	SKB_REASON_DROPPED,
};

void __dev_kfree_skb_irq(struct sk_buff *skb, enum skb_free_reason reason);
void __dev_kfree_skb_any(struct sk_buff *skb, enum skb_free_reason reason);

/*
 * It is not allowed to call kfree_skb() or consume_skb() from hardware
 * interrupt context or with hardware interrupts being disabled.
 * (in_irq() || irqs_disabled())
 *
 * We provide four helpers that can be used in following contexts :
 *
 * dev_kfree_skb_irq(skb) when caller drops a packet from irq context,
 *  replacing kfree_skb(skb)
 *
 * dev_consume_skb_irq(skb) when caller consumes a packet from irq context.
 *  Typically used in place of consume_skb(skb) in TX completion path
 *
 * dev_kfree_skb_any(skb) when caller doesn't know its current irq context,
 *  replacing kfree_skb(skb)
 *
 * dev_consume_skb_any(skb) when caller doesn't know its current irq context,
 *  and consumed a packet. Used in place of consume_skb(skb)
 */
static inline void dev_kfree_skb_irq(struct sk_buff *skb)
{
	__dev_kfree_skb_irq(skb, SKB_REASON_DROPPED);
}

static inline void dev_consume_skb_irq(struct sk_buff *skb)
{
	__dev_kfree_skb_irq(skb, SKB_REASON_CONSUMED);
}

static inline void dev_kfree_skb_any(struct sk_buff *skb)
{
	__dev_kfree_skb_any(skb, SKB_REASON_DROPPED);
}

static inline void dev_consume_skb_any(struct sk_buff *skb)
{
	__dev_kfree_skb_any(skb, SKB_REASON_CONSUMED);
}

int netif_rx(struct sk_buff *skb);
int netif_rx_ni(struct sk_buff *skb);
int netif_receive_skb(struct sk_buff *skb);
gro_result_t napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb);
void napi_gro_flush(struct napi_struct *napi, bool flush_old);
struct sk_buff *napi_get_frags(struct napi_struct *napi);
gro_result_t napi_gro_frags(struct napi_struct *napi);
struct packet_offload *gro_find_receive_by_type(__be16 type);
struct packet_offload *gro_find_complete_by_type(__be16 type);

static inline void napi_free_frags(struct napi_struct *napi)
{
	kfree_skb(napi->skb);
	napi->skb = NULL;
}

bool netdev_is_rx_handler_busy(struct net_device *dev);
int netdev_rx_handler_register(struct net_device *dev,
			       rx_handler_func_t *rx_handler,
			       void *rx_handler_data);
void netdev_rx_handler_unregister(struct net_device *dev);

bool dev_valid_name(const char *name);
int dev_ioctl(struct net *net, unsigned int cmd, void __user *);
int dev_ethtool(struct net *net, struct ifreq *);
unsigned int dev_get_flags(const struct net_device *);
int __dev_change_flags(struct net_device *, unsigned int flags);
int dev_change_flags(struct net_device *, unsigned int);
void __dev_notify_flags(struct net_device *, unsigned int old_flags,
			unsigned int gchanges);
int dev_change_name(struct net_device *, const char *);
int dev_set_alias(struct net_device *, const char *, size_t);
int dev_change_net_namespace(struct net_device *, struct net *, const char *);
int dev_set_mtu(struct net_device *, int);
void dev_set_group(struct net_device *, int);
int dev_set_mac_address(struct net_device *, struct sockaddr *);
int dev_change_carrier(struct net_device *, bool new_carrier);
int dev_get_phys_port_id(struct net_device *dev,
			 struct netdev_phys_item_id *ppid);
int dev_get_phys_port_name(struct net_device *dev,
			   char *name, size_t len);
int dev_change_proto_down(struct net_device *dev, bool proto_down);
int dev_change_xdp_fd(struct net_device *dev, int fd);
struct sk_buff *validate_xmit_skb_list(struct sk_buff *skb, struct net_device *dev);
struct sk_buff *dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev,
				    struct netdev_queue *txq, int *ret);
int __dev_forward_skb(struct net_device *dev, struct sk_buff *skb);
int dev_forward_skb(struct net_device *dev, struct sk_buff *skb);
bool is_skb_forwardable(const struct net_device *dev,
			const struct sk_buff *skb);

void dev_queue_xmit_nit(struct sk_buff *skb, struct net_device *dev);

extern int		netdev_budget;

/* Called by rtnetlink.c:rtnl_unlock() */
void netdev_run_todo(void);

/**
 *	dev_put - release reference to device
 *	@dev: network device
 *
 * Release reference to device to allow it to be freed.
 */
static inline void dev_put(struct net_device *dev)
{
	this_cpu_dec(*dev->pcpu_refcnt);
}

/**
 *	dev_hold - get reference to device
 *	@dev: network device
 *
 * Hold reference to device to keep it from being freed.
 */
static inline void dev_hold(struct net_device *dev)
{
	this_cpu_inc(*dev->pcpu_refcnt);
}

/* Carrier loss detection, dial on demand. The functions netif_carrier_on
 * and _off may be called from IRQ context, but it is caller
 * who is responsible for serialization of these calls.
 *
 * The name carrier is inappropriate, these functions should really be
 * called netif_lowerlayer_*() because they represent the state of any
 * kind of lower layer not just hardware media.
 */

void linkwatch_init_dev(struct net_device *dev);
void linkwatch_fire_event(struct net_device *dev);
void linkwatch_forget_dev(struct net_device *dev);

/**
 *	netif_carrier_ok - test if carrier present
 *	@dev: network device
 *
 * Check if carrier is present on device
 */
static inline bool netif_carrier_ok(const struct net_device *dev)
{
	return !test_bit(__LINK_STATE_NOCARRIER, &dev->state);
}

unsigned long dev_trans_start(struct net_device *dev);

void __netdev_watchdog_up(struct net_device *dev);

void netif_carrier_on(struct net_device *dev);

void netif_carrier_off(struct net_device *dev);

/**
 *	netif_dormant_on - mark device as dormant.
 *	@dev: network device
 *
 * Mark device as dormant (as per RFC2863).
 *
 * The dormant state indicates that the relevant interface is not
 * actually in a condition to pass packets (i.e., it is not 'up') but is
 * in a "pending" state, waiting for some external event.  For "on-
 * demand" interfaces, this new state identifies the situation where the
 * interface is waiting for events to place it in the up state.
 */
static inline void netif_dormant_on(struct net_device *dev)
{
	if (!test_and_set_bit(__LINK_STATE_DORMANT, &dev->state))
		linkwatch_fire_event(dev);
}

/**
 *	netif_dormant_off - set device as not dormant.
 *	@dev: network device
 *
 * Device is not in dormant state.
 */
static inline void netif_dormant_off(struct net_device *dev)
{
	if (test_and_clear_bit(__LINK_STATE_DORMANT, &dev->state))
		linkwatch_fire_event(dev);
}

/**
 *	netif_dormant - test if carrier present
 *	@dev: network device
 *
 * Check if carrier is present on device
 */
static inline bool netif_dormant(const struct net_device *dev)
{
	return test_bit(__LINK_STATE_DORMANT, &dev->state);
}


/**
 *	netif_oper_up - test if device is operational
 *	@dev: network device
 *
 * Check if carrier is operational
 */
static inline bool netif_oper_up(const struct net_device *dev)
{
	return (dev->operstate == IF_OPER_UP ||
		dev->operstate == IF_OPER_UNKNOWN /* backward compat */);
}

/**
 *	netif_device_present - is device available or removed
 *	@dev: network device
 *
 * Check if device has not been removed from system.
 */
static inline bool netif_device_present(struct net_device *dev)
{
	return test_bit(__LINK_STATE_PRESENT, &dev->state);
}

void netif_device_detach(struct net_device *dev);

void netif_device_attach(struct net_device *dev);

/*
 * Network interface message level settings
 */

enum {
	NETIF_MSG_DRV		= 0x0001,
	NETIF_MSG_PROBE		= 0x0002,
	NETIF_MSG_LINK		= 0x0004,
	NETIF_MSG_TIMER		= 0x0008,
	NETIF_MSG_IFDOWN	= 0x0010,
	NETIF_MSG_IFUP		= 0x0020,
	NETIF_MSG_RX_ERR	= 0x0040,
	NETIF_MSG_TX_ERR	= 0x0080,
	NETIF_MSG_TX_QUEUED	= 0x0100,
	NETIF_MSG_INTR		= 0x0200,
	NETIF_MSG_TX_DONE	= 0x0400,
	NETIF_MSG_RX_STATUS	= 0x0800,
	NETIF_MSG_PKTDATA	= 0x1000,
	NETIF_MSG_HW		= 0x2000,
	NETIF_MSG_WOL		= 0x4000,
};

#define netif_msg_drv(p)	((p)->msg_enable & NETIF_MSG_DRV)
#define netif_msg_probe(p)	((p)->msg_enable & NETIF_MSG_PROBE)
#define netif_msg_link(p)	((p)->msg_enable & NETIF_MSG_LINK)
#define netif_msg_timer(p)	((p)->msg_enable & NETIF_MSG_TIMER)
#define netif_msg_ifdown(p)	((p)->msg_enable & NETIF_MSG_IFDOWN)
#define netif_msg_ifup(p)	((p)->msg_enable & NETIF_MSG_IFUP)
#define netif_msg_rx_err(p)	((p)->msg_enable & NETIF_MSG_RX_ERR)
#define netif_msg_tx_err(p)	((p)->msg_enable & NETIF_MSG_TX_ERR)
#define netif_msg_tx_queued(p)	((p)->msg_enable & NETIF_MSG_TX_QUEUED)
#define netif_msg_intr(p)	((p)->msg_enable & NETIF_MSG_INTR)
#define netif_msg_tx_done(p)	((p)->msg_enable & NETIF_MSG_TX_DONE)
#define netif_msg_rx_status(p)	((p)->msg_enable & NETIF_MSG_RX_STATUS)
#define netif_msg_pktdata(p)	((p)->msg_enable & NETIF_MSG_PKTDATA)
#define netif_msg_hw(p)		((p)->msg_enable & NETIF_MSG_HW)
#define netif_msg_wol(p)	((p)->msg_enable & NETIF_MSG_WOL)

static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{
	/* use default */
	if (debug_value < 0 || debug_value >= (sizeof(u32) * 8))
		return default_msg_enable_bits;
	if (debug_value == 0)	/* no output */
		return 0;
	/* set low N bits */
	return (1 << debug_value) - 1;
}

static inline void __netif_tx_lock(struct netdev_queue *txq, int cpu)
{
	spin_lock(&txq->_xmit_lock);
	txq->xmit_lock_owner = cpu;
}

static inline void __netif_tx_lock_bh(struct netdev_queue *txq)
{
	spin_lock_bh(&txq->_xmit_lock);
	txq->xmit_lock_owner = smp_processor_id();
}

static inline bool __netif_tx_trylock(struct netdev_queue *txq)
{
	bool ok = spin_trylock(&txq->_xmit_lock);
	if (likely(ok))
		txq->xmit_lock_owner = smp_processor_id();
	return ok;
}

static inline void __netif_tx_unlock(struct netdev_queue *txq)
{
	txq->xmit_lock_owner = -1;
	spin_unlock(&txq->_xmit_lock);
}

static inline void __netif_tx_unlock_bh(struct netdev_queue *txq)
{
	txq->xmit_lock_owner = -1;
	spin_unlock_bh(&txq->_xmit_lock);
}

static inline void txq_trans_update(struct netdev_queue *txq)
{
	if (txq->xmit_lock_owner != -1)
		txq->trans_start = jiffies;
}

/* legacy drivers only, netdev_start_xmit() sets txq->trans_start */
static inline void netif_trans_update(struct net_device *dev)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, 0);

	if (txq->trans_start != jiffies)
		txq->trans_start = jiffies;
}

/**
 *	netif_tx_lock - grab network device transmit lock
 *	@dev: network device
 *
 * Get network device transmit lock
 */
static inline void netif_tx_lock(struct net_device *dev)
{
	unsigned int i;
	int cpu;

	spin_lock(&dev->tx_global_lock);
	cpu = smp_processor_id();
	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);

		/* We are the only thread of execution doing a
		 * freeze, but we have to grab the _xmit_lock in
		 * order to synchronize with threads which are in
		 * the ->hard_start_xmit() handler and already
		 * checked the frozen bit.
		 */
		__netif_tx_lock(txq, cpu);
		set_bit(__QUEUE_STATE_FROZEN, &txq->state);
		__netif_tx_unlock(txq);
	}
}

static inline void netif_tx_lock_bh(struct net_device *dev)
{
	local_bh_disable();
	netif_tx_lock(dev);
}

static inline void netif_tx_unlock(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);

		/* No need to grab the _xmit_lock here.  If the
		 * queue is not stopped for another reason, we
		 * force a schedule.
		 */
		clear_bit(__QUEUE_STATE_FROZEN, &txq->state);
		netif_schedule_queue(txq);
	}
	spin_unlock(&dev->tx_global_lock);
}

static inline void netif_tx_unlock_bh(struct net_device *dev)
{
	netif_tx_unlock(dev);
	local_bh_enable();
}

#define HARD_TX_LOCK(dev, txq, cpu) {			\
	if ((dev->features & NETIF_F_LLTX) == 0) {	\
		__netif_tx_lock(txq, cpu);		\
	}						\
}

#define HARD_TX_TRYLOCK(dev, txq)			\
	(((dev->features & NETIF_F_LLTX) == 0) ?	\
		__netif_tx_trylock(txq) :		\
		true )

#define HARD_TX_UNLOCK(dev, txq) {			\
	if ((dev->features & NETIF_F_LLTX) == 0) {	\
		__netif_tx_unlock(txq);			\
	}						\
}

static inline void netif_tx_disable(struct net_device *dev)
{
	unsigned int i;
	int cpu;

	local_bh_disable();
	cpu = smp_processor_id();
	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);

		__netif_tx_lock(txq, cpu);
		netif_tx_stop_queue(txq);
		__netif_tx_unlock(txq);
	}
	local_bh_enable();
}

static inline void netif_addr_lock(struct net_device *dev)
{
	spin_lock(&dev->addr_list_lock);
}

static inline void netif_addr_lock_nested(struct net_device *dev)
{
	int subclass = SINGLE_DEPTH_NESTING;

	if (dev->netdev_ops->ndo_get_lock_subclass)
		subclass = dev->netdev_ops->ndo_get_lock_subclass(dev);

	spin_lock_nested(&dev->addr_list_lock, subclass);
}

static inline void netif_addr_lock_bh(struct net_device *dev)
{
	spin_lock_bh(&dev->addr_list_lock);
}

static inline void netif_addr_unlock(struct net_device *dev)
{
	spin_unlock(&dev->addr_list_lock);
}

static inline void netif_addr_unlock_bh(struct net_device *dev)
{
	spin_unlock_bh(&dev->addr_list_lock);
}

/*
 * dev_addrs walker. Should be used only for read access. Call with
 * rcu_read_lock held.
 */
#define for_each_dev_addr(dev, ha) \
		list_for_each_entry_rcu(ha, &dev->dev_addrs.list, list)

/* These functions live elsewhere (drivers/net/net_init.c, but related) */

void ether_setup(struct net_device *dev);

/* Support for loadable net-drivers */
struct net_device *alloc_netdev_mqs(int sizeof_priv, const char *name,
				    unsigned char name_assign_type,
				    void (*setup)(struct net_device *),
				    unsigned int txqs, unsigned int rxqs);
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
	alloc_netdev_mqs(sizeof_priv, name, name_assign_type, setup, 1, 1)

#define alloc_netdev_mq(sizeof_priv, name, name_assign_type, setup, count) \
	alloc_netdev_mqs(sizeof_priv, name, name_assign_type, setup, count, \
			 count)

int register_netdev(struct net_device *dev);
void unregister_netdev(struct net_device *dev);

/* General hardware address lists handling functions */
int __hw_addr_sync(struct netdev_hw_addr_list *to_list,
		   struct netdev_hw_addr_list *from_list, int addr_len);
void __hw_addr_unsync(struct netdev_hw_addr_list *to_list,
		      struct netdev_hw_addr_list *from_list, int addr_len);
int __hw_addr_sync_dev(struct netdev_hw_addr_list *list,
		       struct net_device *dev,
		       int (*sync)(struct net_device *, const unsigned char *),
		       int (*unsync)(struct net_device *,
				     const unsigned char *));
void __hw_addr_unsync_dev(struct netdev_hw_addr_list *list,
			  struct net_device *dev,
			  int (*unsync)(struct net_device *,
					const unsigned char *));
void __hw_addr_init(struct netdev_hw_addr_list *list);

/* Functions used for device addresses handling */
int dev_addr_add(struct net_device *dev, const unsigned char *addr,
		 unsigned char addr_type);
int dev_addr_del(struct net_device *dev, const unsigned char *addr,
		 unsigned char addr_type);
void dev_addr_flush(struct net_device *dev);
int dev_addr_init(struct net_device *dev);

/* Functions used for unicast addresses handling */
int dev_uc_add(struct net_device *dev, const unsigned char *addr);
int dev_uc_add_excl(struct net_device *dev, const unsigned char *addr);
int dev_uc_del(struct net_device *dev, const unsigned char *addr);
int dev_uc_sync(struct net_device *to, struct net_device *from);
int dev_uc_sync_multiple(struct net_device *to, struct net_device *from);
void dev_uc_unsync(struct net_device *to, struct net_device *from);
void dev_uc_flush(struct net_device *dev);
void dev_uc_init(struct net_device *dev);

/**
 *  __dev_uc_sync - Synchonize device's unicast list
 *  @dev:  device to sync
 *  @sync: function to call if address should be added
 *  @unsync: function to call if address should be removed
 *
 *  Add newly added addresses to the interface, and release
 *  addresses that have been deleted.
 */
static inline int __dev_uc_sync(struct net_device *dev,
				int (*sync)(struct net_device *,
					    const unsigned char *),
				int (*unsync)(struct net_device *,
					      const unsigned char *))
{
	return __hw_addr_sync_dev(&dev->uc, dev, sync, unsync);
}

/**
 *  __dev_uc_unsync - Remove synchronized addresses from device
 *  @dev:  device to sync
 *  @unsync: function to call if address should be removed
 *
 *  Remove all addresses that were added to the device by dev_uc_sync().
 */
static inline void __dev_uc_unsync(struct net_device *dev,
				   int (*unsync)(struct net_device *,
						 const unsigned char *))
{
	__hw_addr_unsync_dev(&dev->uc, dev, unsync);
}

/* Functions used for multicast addresses handling */
int dev_mc_add(struct net_device *dev, const unsigned char *addr);
int dev_mc_add_global(struct net_device *dev, const unsigned char *addr);
int dev_mc_add_excl(struct net_device *dev, const unsigned char *addr);
int dev_mc_del(struct net_device *dev, const unsigned char *addr);
int dev_mc_del_global(struct net_device *dev, const unsigned char *addr);
int dev_mc_sync(struct net_device *to, struct net_device *from);
int dev_mc_sync_multiple(struct net_device *to, struct net_device *from);
void dev_mc_unsync(struct net_device *to, struct net_device *from);
void dev_mc_flush(struct net_device *dev);
void dev_mc_init(struct net_device *dev);

/**
 *  __dev_mc_sync - Synchonize device's multicast list
 *  @dev:  device to sync
 *  @sync: function to call if address should be added
 *  @unsync: function to call if address should be removed
 *
 *  Add newly added addresses to the interface, and release
 *  addresses that have been deleted.
 */
static inline int __dev_mc_sync(struct net_device *dev,
				int (*sync)(struct net_device *,
					    const unsigned char *),
				int (*unsync)(struct net_device *,
					      const unsigned char *))
{
	return __hw_addr_sync_dev(&dev->mc, dev, sync, unsync);
}

/**
 *  __dev_mc_unsync - Remove synchronized addresses from device
 *  @dev:  device to sync
 *  @unsync: function to call if address should be removed
 *
 *  Remove all addresses that were added to the device by dev_mc_sync().
 */
static inline void __dev_mc_unsync(struct net_device *dev,
				   int (*unsync)(struct net_device *,
						 const unsigned char *))
{
	__hw_addr_unsync_dev(&dev->mc, dev, unsync);
}

/* Functions used for secondary unicast and multicast support */
void dev_set_rx_mode(struct net_device *dev);
void __dev_set_rx_mode(struct net_device *dev);
int dev_set_promiscuity(struct net_device *dev, int inc);
int dev_set_allmulti(struct net_device *dev, int inc);
void netdev_state_change(struct net_device *dev);
void netdev_notify_peers(struct net_device *dev);
void netdev_features_change(struct net_device *dev);
/* Load a device via the kmod */
void dev_load(struct net *net, const char *name);
struct rtnl_link_stats64 *dev_get_stats(struct net_device *dev,
					struct rtnl_link_stats64 *storage);
void netdev_stats_to_stats64(struct rtnl_link_stats64 *stats64,
			     const struct net_device_stats *netdev_stats);

extern int		netdev_max_backlog;
extern int		netdev_tstamp_prequeue;
extern int		weight_p;

bool netdev_has_upper_dev(struct net_device *dev, struct net_device *upper_dev);
struct net_device *netdev_upper_get_next_dev_rcu(struct net_device *dev,
						     struct list_head **iter);
struct net_device *netdev_all_upper_get_next_dev_rcu(struct net_device *dev,
						     struct list_head **iter);

/* iterate through upper list, must be called under RCU read lock */
#define netdev_for_each_upper_dev_rcu(dev, updev, iter) \
	for (iter = &(dev)->adj_list.upper, \
	     updev = netdev_upper_get_next_dev_rcu(dev, &(iter)); \
	     updev; \
	     updev = netdev_upper_get_next_dev_rcu(dev, &(iter)))

/* iterate through upper list, must be called under RCU read lock */
#define netdev_for_each_all_upper_dev_rcu(dev, updev, iter) \
	for (iter = &(dev)->all_adj_list.upper, \
	     updev = netdev_all_upper_get_next_dev_rcu(dev, &(iter)); \
	     updev; \
	     updev = netdev_all_upper_get_next_dev_rcu(dev, &(iter)))

void *netdev_lower_get_next_private(struct net_device *dev,
				    struct list_head **iter);
void *netdev_lower_get_next_private_rcu(struct net_device *dev,
					struct list_head **iter);

#define netdev_for_each_lower_private(dev, priv, iter) \
	for (iter = (dev)->adj_list.lower.next, \
	     priv = netdev_lower_get_next_private(dev, &(iter)); \
	     priv; \
	     priv = netdev_lower_get_next_private(dev, &(iter)))

#define netdev_for_each_lower_private_rcu(dev, priv, iter) \
	for (iter = &(dev)->adj_list.lower, \
	     priv = netdev_lower_get_next_private_rcu(dev, &(iter)); \
	     priv; \
	     priv = netdev_lower_get_next_private_rcu(dev, &(iter)))

void *netdev_lower_get_next(struct net_device *dev,
				struct list_head **iter);

#define netdev_for_each_lower_dev(dev, ldev, iter) \
	for (iter = (dev)->adj_list.lower.next, \
	     ldev = netdev_lower_get_next(dev, &(iter)); \
	     ldev; \
	     ldev = netdev_lower_get_next(dev, &(iter)))

struct net_device *netdev_all_lower_get_next(struct net_device *dev,
					     struct list_head **iter);
struct net_device *netdev_all_lower_get_next_rcu(struct net_device *dev,
						 struct list_head **iter);

#define netdev_for_each_all_lower_dev(dev, ldev, iter) \
	for (iter = (dev)->all_adj_list.lower.next, \
	     ldev = netdev_all_lower_get_next(dev, &(iter)); \
	     ldev; \
	     ldev = netdev_all_lower_get_next(dev, &(iter)))

#define netdev_for_each_all_lower_dev_rcu(dev, ldev, iter) \
	for (iter = &(dev)->all_adj_list.lower, \
	     ldev = netdev_all_lower_get_next_rcu(dev, &(iter)); \
	     ldev; \
	     ldev = netdev_all_lower_get_next_rcu(dev, &(iter)))

void *netdev_adjacent_get_private(struct list_head *adj_list);
void *netdev_lower_get_first_private_rcu(struct net_device *dev);
struct net_device *netdev_master_upper_dev_get(struct net_device *dev);
struct net_device *netdev_master_upper_dev_get_rcu(struct net_device *dev);
int netdev_upper_dev_link(struct net_device *dev, struct net_device *upper_dev);
int netdev_master_upper_dev_link(struct net_device *dev,
				 struct net_device *upper_dev,
				 void *upper_priv, void *upper_info);
void netdev_upper_dev_unlink(struct net_device *dev,
			     struct net_device *upper_dev);
void netdev_adjacent_rename_links(struct net_device *dev, char *oldname);
void *netdev_lower_dev_get_private(struct net_device *dev,
				   struct net_device *lower_dev);
void netdev_lower_state_changed(struct net_device *lower_dev,
				void *lower_state_info);
int netdev_default_l2upper_neigh_construct(struct net_device *dev,
					   struct neighbour *n);
void netdev_default_l2upper_neigh_destroy(struct net_device *dev,
					  struct neighbour *n);

/* RSS keys are 40 or 52 bytes long */
#define NETDEV_RSS_KEY_LEN 52
extern u8 netdev_rss_key[NETDEV_RSS_KEY_LEN] __read_mostly;
void netdev_rss_key_fill(void *buffer, size_t len);

int dev_get_nest_level(struct net_device *dev);
int skb_checksum_help(struct sk_buff *skb);
struct sk_buff *__skb_gso_segment(struct sk_buff *skb,
				  netdev_features_t features, bool tx_path);
struct sk_buff *skb_mac_gso_segment(struct sk_buff *skb,
				    netdev_features_t features);

struct netdev_bonding_info {
	ifslave	slave;
	ifbond	master;
};

struct netdev_notifier_bonding_info {
	struct netdev_notifier_info info; /* must be first */
	struct netdev_bonding_info  bonding_info;
};

void netdev_bonding_info_change(struct net_device *dev,
				struct netdev_bonding_info *bonding_info);

static inline
struct sk_buff *skb_gso_segment(struct sk_buff *skb, netdev_features_t features)
{
	return __skb_gso_segment(skb, features, true);
}
__be16 skb_network_protocol(struct sk_buff *skb, int *depth);

static inline bool can_checksum_protocol(netdev_features_t features,
					 __be16 protocol)
{
	if (protocol == htons(ETH_P_FCOE))
		return !!(features & NETIF_F_FCOE_CRC);

	/* Assume this is an IP checksum (not SCTP CRC) */

	if (features & NETIF_F_HW_CSUM) {
		/* Can checksum everything */
		return true;
	}

	switch (protocol) {
	case htons(ETH_P_IP):
		return !!(features & NETIF_F_IP_CSUM);
	case htons(ETH_P_IPV6):
		return !!(features & NETIF_F_IPV6_CSUM);
	default:
		return false;
	}
}

/* Map an ethertype into IP protocol if possible */
static inline int eproto_to_ipproto(int eproto)
{
	switch (eproto) {
	case htons(ETH_P_IP):
		return IPPROTO_IP;
	case htons(ETH_P_IPV6):
		return IPPROTO_IPV6;
	default:
		return -1;
	}
}

#ifdef CONFIG_BUG
void netdev_rx_csum_fault(struct net_device *dev);
#else
static inline void netdev_rx_csum_fault(struct net_device *dev)
{
}
#endif
/* rx skb timestamps */
void net_enable_timestamp(void);
void net_disable_timestamp(void);

#ifdef CONFIG_PROC_FS
int __init dev_proc_init(void);
#else
#define dev_proc_init() 0
#endif

static inline netdev_tx_t __netdev_start_xmit(const struct net_device_ops *ops,
					      struct sk_buff *skb, struct net_device *dev,
					      bool more)
{
	skb->xmit_more = more ? 1 : 0;
	return ops->ndo_start_xmit(skb, dev);
}

static inline netdev_tx_t netdev_start_xmit(struct sk_buff *skb, struct net_device *dev,
					    struct netdev_queue *txq, bool more)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	int rc;

	rc = __netdev_start_xmit(ops, skb, dev, more);
	if (rc == NETDEV_TX_OK)
		txq_trans_update(txq);

	return rc;
}

int netdev_class_create_file_ns(struct class_attribute *class_attr,
				const void *ns);
void netdev_class_remove_file_ns(struct class_attribute *class_attr,
				 const void *ns);

static inline int netdev_class_create_file(struct class_attribute *class_attr)
{
	return netdev_class_create_file_ns(class_attr, NULL);
}

static inline void netdev_class_remove_file(struct class_attribute *class_attr)
{
	netdev_class_remove_file_ns(class_attr, NULL);
}

extern struct kobj_ns_type_operations net_ns_type_operations;

const char *netdev_drivername(const struct net_device *dev);

void linkwatch_run_queue(void);

static inline netdev_features_t netdev_intersect_features(netdev_features_t f1,
							  netdev_features_t f2)
{
	if ((f1 ^ f2) & NETIF_F_HW_CSUM) {
		if (f1 & NETIF_F_HW_CSUM)
			f1 |= (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM);
		else
			f2 |= (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM);
	}

	return f1 & f2;
}

static inline netdev_features_t netdev_get_wanted_features(
	struct net_device *dev)
{
	return (dev->features & ~dev->hw_features) | dev->wanted_features;
}
netdev_features_t netdev_increment_features(netdev_features_t all,
	netdev_features_t one, netdev_features_t mask);

/* Allow TSO being used on stacked device :
 * Performing the GSO segmentation before last device
 * is a performance improvement.
 */
static inline netdev_features_t netdev_add_tso_features(netdev_features_t features,
							netdev_features_t mask)
{
	return netdev_increment_features(features, NETIF_F_ALL_TSO, mask);
}

int __netdev_update_features(struct net_device *dev);
void netdev_update_features(struct net_device *dev);
void netdev_change_features(struct net_device *dev);

void netif_stacked_transfer_operstate(const struct net_device *rootdev,
					struct net_device *dev);

netdev_features_t passthru_features_check(struct sk_buff *skb,
					  struct net_device *dev,
					  netdev_features_t features);
netdev_features_t netif_skb_features(struct sk_buff *skb);

static inline bool net_gso_ok(netdev_features_t features, int gso_type)
{
	netdev_features_t feature = (netdev_features_t)gso_type << NETIF_F_GSO_SHIFT;

	/* check flags correspondence */
	BUILD_BUG_ON(SKB_GSO_TCPV4   != (NETIF_F_TSO >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_UDP     != (NETIF_F_UFO >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_DODGY   != (NETIF_F_GSO_ROBUST >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TCP_ECN != (NETIF_F_TSO_ECN >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TCP_FIXEDID != (NETIF_F_TSO_MANGLEID >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TCPV6   != (NETIF_F_TSO6 >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_FCOE    != (NETIF_F_FSO >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_GRE     != (NETIF_F_GSO_GRE >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_GRE_CSUM != (NETIF_F_GSO_GRE_CSUM >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_IPXIP4  != (NETIF_F_GSO_IPXIP4 >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_IPXIP6  != (NETIF_F_GSO_IPXIP6 >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_UDP_TUNNEL != (NETIF_F_GSO_UDP_TUNNEL >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_UDP_TUNNEL_CSUM != (NETIF_F_GSO_UDP_TUNNEL_CSUM >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_PARTIAL != (NETIF_F_GSO_PARTIAL >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TUNNEL_REMCSUM != (NETIF_F_GSO_TUNNEL_REMCSUM >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_SCTP    != (NETIF_F_GSO_SCTP >> NETIF_F_GSO_SHIFT));

	return (features & feature) == feature;
}

static inline bool skb_gso_ok(struct sk_buff *skb, netdev_features_t features)
{
	return net_gso_ok(features, skb_shinfo(skb)->gso_type) &&
	       (!skb_has_frag_list(skb) || (features & NETIF_F_FRAGLIST));
}

static inline bool netif_needs_gso(struct sk_buff *skb,
				   netdev_features_t features)
{
	return skb_is_gso(skb) && (!skb_gso_ok(skb, features) ||
		unlikely((skb->ip_summed != CHECKSUM_PARTIAL) &&
			 (skb->ip_summed != CHECKSUM_UNNECESSARY)));
}

static inline void netif_set_gso_max_size(struct net_device *dev,
					  unsigned int size)
{
	dev->gso_max_size = size;
}

static inline void skb_gso_error_unwind(struct sk_buff *skb, __be16 protocol,
					int pulled_hlen, u16 mac_offset,
					int mac_len)
{
	skb->protocol = protocol;
	skb->encapsulation = 1;
	skb_push(skb, pulled_hlen);
	skb_reset_transport_header(skb);
	skb->mac_header = mac_offset;
	skb->network_header = skb->mac_header + mac_len;
	skb->mac_len = mac_len;
}

static inline bool netif_is_macsec(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MACSEC;
}

static inline bool netif_is_macvlan(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MACVLAN;
}

static inline bool netif_is_macvlan_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MACVLAN_PORT;
}

static inline bool netif_is_ipvlan(const struct net_device *dev)
{
	return dev->priv_flags & IFF_IPVLAN_SLAVE;
}

static inline bool netif_is_ipvlan_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_IPVLAN_MASTER;
}

static inline bool netif_is_bond_master(const struct net_device *dev)
{
	return dev->flags & IFF_MASTER && dev->priv_flags & IFF_BONDING;
}

static inline bool netif_is_bond_slave(const struct net_device *dev)
{
	return dev->flags & IFF_SLAVE && dev->priv_flags & IFF_BONDING;
}

static inline bool netif_supports_nofcs(struct net_device *dev)
{
	return dev->priv_flags & IFF_SUPP_NOFCS;
}

static inline bool netif_is_l3_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_L3MDEV_MASTER;
}

static inline bool netif_is_l3_slave(const struct net_device *dev)
{
	return dev->priv_flags & IFF_L3MDEV_SLAVE;
}

static inline bool netif_is_bridge_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_EBRIDGE;
}

static inline bool netif_is_bridge_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_BRIDGE_PORT;
}

static inline bool netif_is_ovs_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_OPENVSWITCH;
}

static inline bool netif_is_team_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_TEAM;
}

static inline bool netif_is_team_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_TEAM_PORT;
}

static inline bool netif_is_lag_master(const struct net_device *dev)
{
	return netif_is_bond_master(dev) || netif_is_team_master(dev);
}

static inline bool netif_is_lag_port(const struct net_device *dev)
{
	return netif_is_bond_slave(dev) || netif_is_team_port(dev);
}

static inline bool netif_is_rxfh_configured(const struct net_device *dev)
{
	return dev->priv_flags & IFF_RXFH_CONFIGURED;
}

/* This device needs to keep skb dst for qdisc enqueue or ndo_start_xmit() */
static inline void netif_keep_dst(struct net_device *dev)
{
	dev->priv_flags &= ~(IFF_XMIT_DST_RELEASE | IFF_XMIT_DST_RELEASE_PERM);
}

/* return true if dev can't cope with mtu frames that need vlan tag insertion */
static inline bool netif_reduces_vlan_mtu(struct net_device *dev)
{
	/* TODO: reserve and use an additional IFF bit, if we get more users */
	return dev->priv_flags & IFF_MACSEC;
}

extern struct pernet_operations __net_initdata loopback_net_ops;

/* Logging, debugging and troubleshooting/diagnostic helpers. */

/* netdev_printk helpers, similar to dev_printk */

static inline const char *netdev_name(const struct net_device *dev)
{
	if (!dev->name[0] || strchr(dev->name, '%'))
		return "(unnamed net_device)";
	return dev->name;
}

static inline const char *netdev_reg_state(const struct net_device *dev)
{
	switch (dev->reg_state) {
	case NETREG_UNINITIALIZED: return " (uninitialized)";
	case NETREG_REGISTERED: return "";
	case NETREG_UNREGISTERING: return " (unregistering)";
	case NETREG_UNREGISTERED: return " (unregistered)";
	case NETREG_RELEASED: return " (released)";
	case NETREG_DUMMY: return " (dummy)";
	}

	WARN_ONCE(1, "%s: unknown reg_state %d\n", dev->name, dev->reg_state);
	return " (unknown)";
}

__printf(3, 4)
void netdev_printk(const char *level, const struct net_device *dev,
		   const char *format, ...);
__printf(2, 3)
void netdev_emerg(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_alert(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_crit(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_err(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_warn(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_notice(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_info(const struct net_device *dev, const char *format, ...);

#define MODULE_ALIAS_NETDEV(device) \
	MODULE_ALIAS("netdev-" device)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define netdev_dbg(__dev, format, args...)			\
do {								\
	dynamic_netdev_dbg(__dev, format, ##args);		\
} while (0)
#elif defined(DEBUG)
#define netdev_dbg(__dev, format, args...)			\
	netdev_printk(KERN_DEBUG, __dev, format, ##args)
#else
#define netdev_dbg(__dev, format, args...)			\
({								\
	if (0)							\
		netdev_printk(KERN_DEBUG, __dev, format, ##args); \
})
#endif

#if defined(VERBOSE_DEBUG)
#define netdev_vdbg	netdev_dbg
#else

#define netdev_vdbg(dev, format, args...)			\
({								\
	if (0)							\
		netdev_printk(KERN_DEBUG, dev, format, ##args);	\
	0;							\
})
#endif

/*
 * netdev_WARN() acts like dev_printk(), but with the key difference
 * of using a WARN/WARN_ON to get the message out, including the
 * file/line information and a backtrace.
 */
#define netdev_WARN(dev, format, args...)			\
	WARN(1, "netdevice: %s%s\n" format, netdev_name(dev),	\
	     netdev_reg_state(dev), ##args)

/* netif printk helpers, similar to netdev_printk */

#define netif_printk(priv, type, level, dev, fmt, args...)	\
do {					  			\
	if (netif_msg_##type(priv))				\
		netdev_printk(level, (dev), fmt, ##args);	\
} while (0)

#define netif_level(level, priv, type, dev, fmt, args...)	\
do {								\
	if (netif_msg_##type(priv))				\
		netdev_##level(dev, fmt, ##args);		\
} while (0)

#define netif_emerg(priv, type, dev, fmt, args...)		\
	netif_level(emerg, priv, type, dev, fmt, ##args)
#define netif_alert(priv, type, dev, fmt, args...)		\
	netif_level(alert, priv, type, dev, fmt, ##args)
#define netif_crit(priv, type, dev, fmt, args...)		\
	netif_level(crit, priv, type, dev, fmt, ##args)
#define netif_err(priv, type, dev, fmt, args...)		\
	netif_level(err, priv, type, dev, fmt, ##args)
#define netif_warn(priv, type, dev, fmt, args...)		\
	netif_level(warn, priv, type, dev, fmt, ##args)
#define netif_notice(priv, type, dev, fmt, args...)		\
	netif_level(notice, priv, type, dev, fmt, ##args)
#define netif_info(priv, type, dev, fmt, args...)		\
	netif_level(info, priv, type, dev, fmt, ##args)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define netif_dbg(priv, type, netdev, format, args...)		\
do {								\
	if (netif_msg_##type(priv))				\
		dynamic_netdev_dbg(netdev, format, ##args);	\
} while (0)
#elif defined(DEBUG)
#define netif_dbg(priv, type, dev, format, args...)		\
	netif_printk(priv, type, KERN_DEBUG, dev, format, ##args)
#else
#define netif_dbg(priv, type, dev, format, args...)			\
({									\
	if (0)								\
		netif_printk(priv, type, KERN_DEBUG, dev, format, ##args); \
	0;								\
})
#endif

#if defined(VERBOSE_DEBUG)
#define netif_vdbg	netif_dbg
#else
#define netif_vdbg(priv, type, dev, format, args...)		\
({								\
	if (0)							\
		netif_printk(priv, type, KERN_DEBUG, dev, format, ##args); \
	0;							\
})
#endif

/*
 *	The list of packet types we will receive (as opposed to discard)
 *	and the routines to invoke.
 *
 *	Why 16. Because with 16 the only overlap we get on a hash of the
 *	low nibble of the protocol value is RARP/SNAP/X.25.
 *
 *      NOTE:  That is no longer true with the addition of VLAN tags.  Not
 *             sure which should go first, but I bet it won't make much
 *             difference if we are running VLANs.  The good news is that
 *             this protocol won't be in the list unless compiled in, so
 *             the average user (w/out VLANs) will not be adversely affected.
 *             --BLG
 *
 *		0800	IP
 *		8100    802.1Q VLAN
 *		0001	802.3
 *		0002	AX.25
 *		0004	802.2
 *		8035	RARP
 *		0005	SNAP
 *		0805	X.25
 *		0806	ARP
 *		8137	IPX
 *		0009	Localtalk
 *		86DD	IPv6
 */
#define PTYPE_HASH_SIZE	(16)
#define PTYPE_HASH_MASK	(PTYPE_HASH_SIZE - 1)

#endif	/* _LINUX_NETDEVICE_H */
struct napi_gro_cb {
	/* Virtual address of skb_shinfo(skb)->frags[0].page + offset. */
	void	*frag0;

	/* Length of frag0. */
	unsigned int frag0_len;

	/* This indicates where we are processing relative to skb->data. */
	int	data_offset;

	/* This is non-zero if the packet cannot be merged with the new skb. */
	u16	flush;

	/* Save the IP ID here and check when we get to the transport layer */
	u16	flush_id;

	/* Number of segments aggregated. */
	u16	count;

	/* Start offset for remote checksum offload */
	u16	gro_remcsum_start;

	/* jiffies when first packet was created/queued */
	unsigned long age;

	/* Used in ipv6_gro_receive() and foo-over-udp */
	u16	proto;

	/* This is non-zero if the packet may be of the same flow. */
	u8	same_flow:1;

	/* Used in tunnel GRO receive */
	u8	encap_mark:1;

	/* GRO checksum is valid */
	u8	csum_valid:1;

	/* Number of checksums via CHECKSUM_UNNECESSARY */
	u8	csum_cnt:3;

	/* Free the skb? */
	u8	free:2;
#define NAPI_GRO_FREE		  1
#define NAPI_GRO_FREE_STOLEN_HEAD 2

	/* Used in foo-over-udp, set in udp[46]_gro_receive */
	u8	is_ipv6:1;

	/* Used in GRE, set in fou/gue_gro_receive */
	u8	is_fou:1;

	/* Used to determine if flush_id can be ignored */
	u8	is_atomic:1;

	/* 5 bit hole */

	/* used to support CHECKSUM_COMPLETE for tunneling protocols */
	__wsum	csum;

	/* used in skb_gro_receive() slow path */
	struct sk_buff *last;
};

#define NAPI_GRO_CB(skb) ((struct napi_gro_cb *)(skb)->cb)

struct packet_type {
	__be16			type;	/* This is really htons(ether_type). */
	struct net_device	*dev;	/* NULL is wildcarded here	     */
	int			(*func) (struct sk_buff *,
					 struct net_device *,
					 struct packet_type *,
					 struct net_device *);
	bool			(*id_match)(struct packet_type *ptype,
					    struct sock *sk);
	void			*af_packet_priv;
	struct list_head	list;
};

struct offload_callbacks {
	struct sk_buff		*(*gso_segment)(struct sk_buff *skb,
						netdev_features_t features);
	struct sk_buff		**(*gro_receive)(struct sk_buff **head,
						 struct sk_buff *skb);
	int			(*gro_complete)(struct sk_buff *skb, int nhoff);
};

struct packet_offload {
	__be16			 type;	/* This is really htons(ether_type). */
	u16			 priority;
	struct offload_callbacks callbacks;
	struct list_head	 list;
};

/* often modified stats are per-CPU, other are shared (netdev->stats) */
struct pcpu_sw_netstats {
	u64     rx_packets;
	u64     rx_bytes;
	u64     tx_packets;
	u64     tx_bytes;
	struct u64_stats_sync   syncp;
};

#define __netdev_alloc_pcpu_stats(type, gfp)				\
({									\
	typeof(type) __percpu *pcpu_stats = alloc_percpu_gfp(type, gfp);\
	if (pcpu_stats)	{						\
		int __cpu;						\
		for_each_possible_cpu(__cpu) {				\
			typeof(type) *stat;				\
			stat = per_cpu_ptr(pcpu_stats, __cpu);		\
			u64_stats_init(&stat->syncp);			\
		}							\
	}								\
	pcpu_stats;							\
})

#define netdev_alloc_pcpu_stats(type)					\
	__netdev_alloc_pcpu_stats(type, GFP_KERNEL)

enum netdev_lag_tx_type {
	NETDEV_LAG_TX_TYPE_UNKNOWN,
	NETDEV_LAG_TX_TYPE_RANDOM,
	NETDEV_LAG_TX_TYPE_BROADCAST,
	NETDEV_LAG_TX_TYPE_ROUNDROBIN,
	NETDEV_LAG_TX_TYPE_ACTIVEBACKUP,
	NETDEV_LAG_TX_TYPE_HASH,
};

struct netdev_lag_upper_info {
	enum netdev_lag_tx_type tx_type;
};

struct netdev_lag_lower_state_info {
	u8 link_up : 1,
	   tx_enabled : 1;
};

#include <linux/notifier.h>

/* netdevice notifier chain. Please remember to update the rtnetlink
 * notification exclusion list in rtnetlink_event() when adding new
 * types.
 */
#define NETDEV_UP	0x0001	/* For now you can't veto a device up/down */
#define NETDEV_DOWN	0x0002
#define NETDEV_REBOOT	0x0003	/* Tell a protocol stack a network interface
				   detected a hardware crash and restarted
				   - we can use this eg to kick tcp sessions
				   once done */
#define NETDEV_CHANGE	0x0004	/* Notify device state change */
#define NETDEV_REGISTER 0x0005
#define NETDEV_UNREGISTER	0x0006
#define NETDEV_CHANGEMTU	0x0007 /* notify after mtu change happened */
#define NETDEV_CHANGEADDR	0x0008
#define NETDEV_GOING_DOWN	0x0009
#define NETDEV_CHANGENAME	0x000A
#define NETDEV_FEAT_CHANGE	0x000B
#define NETDEV_BONDING_FAILOVER 0x000C
#define NETDEV_PRE_UP		0x000D
#define NETDEV_PRE_TYPE_CHANGE	0x000E
#define NETDEV_POST_TYPE_CHANGE	0x000F
#define NETDEV_POST_INIT	0x0010
#define NETDEV_UNREGISTER_FINAL 0x0011
#define NETDEV_RELEASE		0x0012
#define NETDEV_NOTIFY_PEERS	0x0013
#define NETDEV_JOIN		0x0014
#define NETDEV_CHANGEUPPER	0x0015
#define NETDEV_RESEND_IGMP	0x0016
#define NETDEV_PRECHANGEMTU	0x0017 /* notify before mtu change happened */
#define NETDEV_CHANGEINFODATA	0x0018
#define NETDEV_BONDING_INFO	0x0019
#define NETDEV_PRECHANGEUPPER	0x001A
#define NETDEV_CHANGELOWERSTATE	0x001B
#define NETDEV_UDP_TUNNEL_PUSH_INFO	0x001C
#define NETDEV_CHANGE_TX_QUEUE_LEN	0x001E

int register_netdevice_notifier(struct notifier_block *nb);
int unregister_netdevice_notifier(struct notifier_block *nb);

struct netdev_notifier_info {
	struct net_device *dev;
};

struct netdev_notifier_change_info {
	struct netdev_notifier_info info; /* must be first */
	unsigned int flags_changed;
};

struct netdev_notifier_changeupper_info {
	struct netdev_notifier_info info; /* must be first */
	struct net_device *upper_dev; /* new upper dev */
	bool master; /* is upper dev master */
	bool linking; /* is the notification for link or unlink */
	void *upper_info; /* upper dev info */
};

struct netdev_notifier_changelowerstate_info {
	struct netdev_notifier_info info; /* must be first */
	void *lower_state_info; /* is lower dev state */
};

static inline void netdev_notifier_info_init(struct netdev_notifier_info *info,
					     struct net_device *dev)
{
	info->dev = dev;
}

static inline struct net_device *
netdev_notifier_info_to_dev(const struct netdev_notifier_info *info)
{
	return info->dev;
}

int call_netdevice_notifiers(unsigned long val, struct net_device *dev);


extern rwlock_t				dev_base_lock;		/* Device list lock */

#define for_each_netdev(net, d)		\
		list_for_each_entry(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_reverse(net, d)	\
		list_for_each_entry_reverse(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_rcu(net, d)		\
		list_for_each_entry_rcu(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_safe(net, d, n)	\
		list_for_each_entry_safe(d, n, &(net)->dev_base_head, dev_list)
#define for_each_netdev_continue(net, d)		\
		list_for_each_entry_continue(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_continue_rcu(net, d)		\
	list_for_each_entry_continue_rcu(d, &(net)->dev_base_head, dev_list)
#define for_each_netdev_in_bond_rcu(bond, slave)	\
		for_each_netdev_rcu(&init_net, slave)	\
			if (netdev_master_upper_dev_get_rcu(slave) == (bond))
#define net_device_entry(lh)	list_entry(lh, struct net_device, dev_list)

static inline struct net_device *next_net_device(struct net_device *dev)
{
	struct list_head *lh;
	struct net *net;

	net = dev_net(dev);
	lh = dev->dev_list.next;
	return lh == &net->dev_base_head ? NULL : net_device_entry(lh);
}

static inline struct net_device *next_net_device_rcu(struct net_device *dev)
{
	struct list_head *lh;
	struct net *net;

	net = dev_net(dev);
	lh = rcu_dereference(list_next_rcu(&dev->dev_list));
	return lh == &net->dev_base_head ? NULL : net_device_entry(lh);
}

static inline struct net_device *first_net_device(struct net *net)
{
	return list_empty(&net->dev_base_head) ? NULL :
		net_device_entry(net->dev_base_head.next);
}

static inline struct net_device *first_net_device_rcu(struct net *net)
{
	struct list_head *lh = rcu_dereference(list_next_rcu(&net->dev_base_head));

	return lh == &net->dev_base_head ? NULL : net_device_entry(lh);
}

int netdev_boot_setup_check(struct net_device *dev);
unsigned long netdev_boot_base(const char *prefix, int unit);
struct net_device *dev_getbyhwaddr_rcu(struct net *net, unsigned short type,
				       const char *hwaddr);
struct net_device *dev_getfirstbyhwtype(struct net *net, unsigned short type);
struct net_device *__dev_getfirstbyhwtype(struct net *net, unsigned short type);
void dev_add_pack(struct packet_type *pt);
void dev_remove_pack(struct packet_type *pt);
void __dev_remove_pack(struct packet_type *pt);
void dev_add_offload(struct packet_offload *po);
void dev_remove_offload(struct packet_offload *po);

int dev_get_iflink(const struct net_device *dev);
int dev_fill_metadata_dst(struct net_device *dev, struct sk_buff *skb);
struct net_device *__dev_get_by_flags(struct net *net, unsigned short flags,
				      unsigned short mask);
struct net_device *dev_get_by_name(struct net *net, const char *name);
struct net_device *dev_get_by_name_rcu(struct net *net, const char *name);
struct net_device *__dev_get_by_name(struct net *net, const char *name);
int dev_alloc_name(struct net_device *dev, const char *name);
int dev_open(struct net_device *dev);
int dev_close(struct net_device *dev);
int dev_close_many(struct list_head *head, bool unlink);
void dev_disable_lro(struct net_device *dev);
int dev_loopback_xmit(struct net *net, struct sock *sk, struct sk_buff *newskb);
int dev_queue_xmit(struct sk_buff *skb);
int dev_queue_xmit_accel(struct sk_buff *skb, void *accel_priv);
int register_netdevice(struct net_device *dev);
void unregister_netdevice_queue(struct net_device *dev, struct list_head *head);
void unregister_netdevice_many(struct list_head *head);
static inline void unregister_netdevice(struct net_device *dev)
{
	unregister_netdevice_queue(dev, NULL);
}

int netdev_refcnt_read(const struct net_device *dev);
void free_netdev(struct net_device *dev);
void netdev_freemem(struct net_device *dev);
void synchronize_net(void);
int init_dummy_netdev(struct net_device *dev);

DECLARE_PER_CPU(int, xmit_recursion);
#define XMIT_RECURSION_LIMIT	10

static inline int dev_recursion_level(void)
{
	return this_cpu_read(xmit_recursion);
}

struct net_device *dev_get_by_index(struct net *net, int ifindex);
struct net_device *__dev_get_by_index(struct net *net, int ifindex);
struct net_device *dev_get_by_index_rcu(struct net *net, int ifindex);
int netdev_get_name(struct net *net, char *name, int ifindex);
int dev_restart(struct net_device *dev);
int skb_gro_receive(struct sk_buff **head, struct sk_buff *skb);

static inline unsigned int skb_gro_offset(const struct sk_buff *skb)
{
	return NAPI_GRO_CB(skb)->data_offset;
}

static inline unsigned int skb_gro_len(const struct sk_buff *skb)
{
	return skb->len - NAPI_GRO_CB(skb)->data_offset;
}

static inline void skb_gro_pull(struct sk_buff *skb, unsigned int len)
{
	NAPI_GRO_CB(skb)->data_offset += len;
}

static inline void *skb_gro_header_fast(struct sk_buff *skb,
					unsigned int offset)
{
	return NAPI_GRO_CB(skb)->frag0 + offset;
}

static inline int skb_gro_header_hard(struct sk_buff *skb, unsigned int hlen)
{
	return NAPI_GRO_CB(skb)->frag0_len < hlen;
}

static inline void *skb_gro_header_slow(struct sk_buff *skb, unsigned int hlen,
					unsigned int offset)
{
	if (!pskb_may_pull(skb, hlen))
		return NULL;

	NAPI_GRO_CB(skb)->frag0 = NULL;
	NAPI_GRO_CB(skb)->frag0_len = 0;
	return skb->data + offset;
}

static inline void *skb_gro_network_header(struct sk_buff *skb)
{
	return (NAPI_GRO_CB(skb)->frag0 ?: skb->data) +
	       skb_network_offset(skb);
}

static inline void skb_gro_postpull_rcsum(struct sk_buff *skb,
					const void *start, unsigned int len)
{
	if (NAPI_GRO_CB(skb)->csum_valid)
		NAPI_GRO_CB(skb)->csum = csum_sub(NAPI_GRO_CB(skb)->csum,
						  csum_partial(start, len, 0));
}

/* GRO checksum functions. These are logical equivalents of the normal
 * checksum functions (in skbuff.h) except that they operate on the GRO
 * offsets and fields in sk_buff.
 */

__sum16 __skb_gro_checksum_complete(struct sk_buff *skb);

static inline bool skb_at_gro_remcsum_start(struct sk_buff *skb)
{
	return (NAPI_GRO_CB(skb)->gro_remcsum_start == skb_gro_offset(skb));
}

static inline bool __skb_gro_checksum_validate_needed(struct sk_buff *skb,
						      bool zero_okay,
						      __sum16 check)
{
	return ((skb->ip_summed != CHECKSUM_PARTIAL ||
		skb_checksum_start_offset(skb) <
		 skb_gro_offset(skb)) &&
		!skb_at_gro_remcsum_start(skb) &&
		NAPI_GRO_CB(skb)->csum_cnt == 0 &&
		(!zero_okay || check));
}

static inline __sum16 __skb_gro_checksum_validate_complete(struct sk_buff *skb,
							   __wsum psum)
{
	if (NAPI_GRO_CB(skb)->csum_valid &&
	    !csum_fold(csum_add(psum, NAPI_GRO_CB(skb)->csum)))
		return 0;

	NAPI_GRO_CB(skb)->csum = psum;

	return __skb_gro_checksum_complete(skb);
}

static inline void skb_gro_incr_csum_unnecessary(struct sk_buff *skb)
{
	if (NAPI_GRO_CB(skb)->csum_cnt > 0) {
		/* Consume a checksum from CHECKSUM_UNNECESSARY */
		NAPI_GRO_CB(skb)->csum_cnt--;
	} else {
		/* Update skb for CHECKSUM_UNNECESSARY and csum_level when we
		 * verified a new top level checksum or an encapsulated one
		 * during GRO. This saves work if we fallback to normal path.
		 */
		__skb_incr_checksum_unnecessary(skb);
	}
}

#define __skb_gro_checksum_validate(skb, proto, zero_okay, check,	\
				    compute_pseudo)			\
({									\
	__sum16 __ret = 0;						\
	if (__skb_gro_checksum_validate_needed(skb, zero_okay, check))	\
		__ret = __skb_gro_checksum_validate_complete(skb,	\
				compute_pseudo(skb, proto));		\
	if (__ret)							\
		__skb_mark_checksum_bad(skb);				\
	else								\
		skb_gro_incr_csum_unnecessary(skb);			\
	__ret;								\
})

#define skb_gro_checksum_validate(skb, proto, compute_pseudo)		\
	__skb_gro_checksum_validate(skb, proto, false, 0, compute_pseudo)

#define skb_gro_checksum_validate_zero_check(skb, proto, check,		\
					     compute_pseudo)		\
	__skb_gro_checksum_validate(skb, proto, true, check, compute_pseudo)

#define skb_gro_checksum_simple_validate(skb)				\
	__skb_gro_checksum_validate(skb, 0, false, 0, null_compute_pseudo)

static inline bool __skb_gro_checksum_convert_check(struct sk_buff *skb)
{
	return (NAPI_GRO_CB(skb)->csum_cnt == 0 &&
		!NAPI_GRO_CB(skb)->csum_valid);
}

static inline void __skb_gro_checksum_convert(struct sk_buff *skb,
					      __sum16 check, __wsum pseudo)
{
	NAPI_GRO_CB(skb)->csum = ~pseudo;
	NAPI_GRO_CB(skb)->csum_valid = 1;
}

#define skb_gro_checksum_try_convert(skb, proto, check, compute_pseudo)	\
do {									\
	if (__skb_gro_checksum_convert_check(skb))			\
		__skb_gro_checksum_convert(skb, check,			\
					   compute_pseudo(skb, proto));	\
} while (0)

struct gro_remcsum {
	int offset;
	__wsum delta;
};

static inline void skb_gro_remcsum_init(struct gro_remcsum *grc)
{
	grc->offset = 0;
	grc->delta = 0;
}

static inline void *skb_gro_remcsum_process(struct sk_buff *skb, void *ptr,
					    unsigned int off, size_t hdrlen,
					    int start, int offset,
					    struct gro_remcsum *grc,
					    bool nopartial)
{
	__wsum delta;
	size_t plen = hdrlen + max_t(size_t, offset + sizeof(u16), start);

	BUG_ON(!NAPI_GRO_CB(skb)->csum_valid);

	if (!nopartial) {
		NAPI_GRO_CB(skb)->gro_remcsum_start = off + hdrlen + start;
		return ptr;
	}

	ptr = skb_gro_header_fast(skb, off);
	if (skb_gro_header_hard(skb, off + plen)) {
		ptr = skb_gro_header_slow(skb, off + plen, off);
		if (!ptr)
			return NULL;
	}

	delta = remcsum_adjust(ptr + hdrlen, NAPI_GRO_CB(skb)->csum,
			       start, offset);

	/* Adjust skb->csum since we changed the packet */
	NAPI_GRO_CB(skb)->csum = csum_add(NAPI_GRO_CB(skb)->csum, delta);

	grc->offset = off + hdrlen + offset;
	grc->delta = delta;

	return ptr;
}

static inline void skb_gro_remcsum_cleanup(struct sk_buff *skb,
					   struct gro_remcsum *grc)
{
	void *ptr;
	size_t plen = grc->offset + sizeof(u16);

	if (!grc->delta)
		return;

	ptr = skb_gro_header_fast(skb, grc->offset);
	if (skb_gro_header_hard(skb, grc->offset + sizeof(u16))) {
		ptr = skb_gro_header_slow(skb, plen, grc->offset);
		if (!ptr)
			return;
	}

	remcsum_unadjust((__sum16 *)ptr, grc->delta);
}

struct skb_csum_offl_spec {
	__u16		ipv4_okay:1,
			ipv6_okay:1,
			encap_okay:1,
			ip_options_okay:1,
			ext_hdrs_okay:1,
			tcp_okay:1,
			udp_okay:1,
			sctp_okay:1,
			vlan_okay:1,
			no_encapped_ipv6:1,
			no_not_encapped:1;
};

bool __skb_csum_offload_chk(struct sk_buff *skb,
			    const struct skb_csum_offl_spec *spec,
			    bool *csum_encapped,
			    bool csum_help);

static inline bool skb_csum_offload_chk(struct sk_buff *skb,
					const struct skb_csum_offl_spec *spec,
					bool *csum_encapped,
					bool csum_help)
{
	if (skb->ip_summed != CHECKSUM_PARTIAL)
		return false;

	return __skb_csum_offload_chk(skb, spec, csum_encapped, csum_help);
}

static inline bool skb_csum_offload_chk_help(struct sk_buff *skb,
					     const struct skb_csum_offl_spec *spec)
{
	bool csum_encapped;

	return skb_csum_offload_chk(skb, spec, &csum_encapped, true);
}

static inline bool skb_csum_off_chk_help_cmn(struct sk_buff *skb)
{
	static const struct skb_csum_offl_spec csum_offl_spec = {
		.ipv4_okay = 1,
		.ip_options_okay = 1,
		.ipv6_okay = 1,
		.vlan_okay = 1,
		.tcp_okay = 1,
		.udp_okay = 1,
	};

	return skb_csum_offload_chk_help(skb, &csum_offl_spec);
}

static inline bool skb_csum_off_chk_help_cmn_v4_only(struct sk_buff *skb)
{
	static const struct skb_csum_offl_spec csum_offl_spec = {
		.ipv4_okay = 1,
		.ip_options_okay = 1,
		.tcp_okay = 1,
		.udp_okay = 1,
		.vlan_okay = 1,
	};

	return skb_csum_offload_chk_help(skb, &csum_offl_spec);
}

static inline int dev_hard_header(struct sk_buff *skb, struct net_device *dev,
				  unsigned short type,
				  const void *daddr, const void *saddr,
				  unsigned int len)
{
	if (!dev->header_ops || !dev->header_ops->create)
		return 0;

	return dev->header_ops->create(skb, dev, type, daddr, saddr, len);
}

static inline int dev_parse_header(const struct sk_buff *skb,
				   unsigned char *haddr)
{
	const struct net_device *dev = skb->dev;

	if (!dev->header_ops || !dev->header_ops->parse)
		return 0;
	return dev->header_ops->parse(skb, haddr);
}

/* ll_header must have at least hard_header_len allocated */
static inline bool dev_validate_header(const struct net_device *dev,
				       char *ll_header, int len)
{
	if (likely(len >= dev->hard_header_len))
		return true;

	if (capable(CAP_SYS_RAWIO)) {
		memset(ll_header + len, 0, dev->hard_header_len - len);
		return true;
	}

	if (dev->header_ops && dev->header_ops->validate)
		return dev->header_ops->validate(ll_header, len);

	return false;
}

typedef int gifconf_func_t(struct net_device * dev, char __user * bufptr, int len);
int register_gifconf(unsigned int family, gifconf_func_t *gifconf);
static inline int unregister_gifconf(unsigned int family)
{
	return register_gifconf(family, NULL);
}

#ifdef CONFIG_NET_FLOW_LIMIT
#define FLOW_LIMIT_HISTORY	(1 << 7)  /* must be ^2 and !overflow buckets */
struct sd_flow_limit {
	u64			count;
	unsigned int		num_buckets;
	unsigned int		history_head;
	u16			history[FLOW_LIMIT_HISTORY];
	u8			buckets[];
};

extern int netdev_flow_limit_table_len;
#endif /* CONFIG_NET_FLOW_LIMIT */

/*
 * Incoming packets are placed on per-CPU queues
 */
struct softnet_data {
	struct list_head	poll_list;
	struct sk_buff_head	process_queue;

	/* stats */
	unsigned int		processed;
	unsigned int		time_squeeze;
	unsigned int		received_rps;
#ifdef CONFIG_RPS
	struct softnet_data	*rps_ipi_list;
#endif
#ifdef CONFIG_NET_FLOW_LIMIT
	struct sd_flow_limit __rcu *flow_limit;
#endif
	struct Qdisc		*output_queue;
	struct Qdisc		**output_queue_tailp;
	struct sk_buff		*completion_queue;

#ifdef CONFIG_RPS
	/* input_queue_head should be written by cpu owning this struct,
	 * and only read by other cpus. Worth using a cache line.
	 */
	unsigned int		input_queue_head ____cacheline_aligned_in_smp;

	/* Elements below can be accessed between CPUs for RPS/RFS */
	struct call_single_data	csd ____cacheline_aligned_in_smp;
	struct softnet_data	*rps_ipi_next;
	unsigned int		cpu;
	unsigned int		input_queue_tail;
#endif
	unsigned int		dropped;
	struct sk_buff_head	input_pkt_queue;
	struct napi_struct	backlog;

};

static inline void input_queue_head_incr(struct softnet_data *sd)
{
#ifdef CONFIG_RPS
	sd->input_queue_head++;
#endif
}

static inline void input_queue_tail_incr_save(struct softnet_data *sd,
					      unsigned int *qtail)
{
#ifdef CONFIG_RPS
	*qtail = ++sd->input_queue_tail;
#endif
}

DECLARE_PER_CPU_ALIGNED(struct softnet_data, softnet_data);

void __netif_schedule(struct Qdisc *q);
void netif_schedule_queue(struct netdev_queue *txq);

static inline void netif_tx_schedule_all(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++)
		netif_schedule_queue(netdev_get_tx_queue(dev, i));
}

static __always_inline void netif_tx_start_queue(struct netdev_queue *dev_queue)
{
	clear_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
}

/**
 *	netif_start_queue - allow transmit
 *	@dev: network device
 *
 *	Allow upper layers to call the device hard_start_xmit routine.
 */
static inline void netif_start_queue(struct net_device *dev)
{
	netif_tx_start_queue(netdev_get_tx_queue(dev, 0));
}

static inline void netif_tx_start_all_queues(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);
		netif_tx_start_queue(txq);
	}
}

void netif_tx_wake_queue(struct netdev_queue *dev_queue);

/**
 *	netif_wake_queue - restart transmit
 *	@dev: network device
 *
 *	Allow upper layers to call the device hard_start_xmit routine.
 *	Used for flow control when transmit resources are available.
 */
static inline void netif_wake_queue(struct net_device *dev)
{
	netif_tx_wake_queue(netdev_get_tx_queue(dev, 0));
}

static inline void netif_tx_wake_all_queues(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);
		netif_tx_wake_queue(txq);
	}
}

static __always_inline void netif_tx_stop_queue(struct netdev_queue *dev_queue)
{
	set_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
}

/**
 *	netif_stop_queue - stop transmitted packets
 *	@dev: network device
 *
 *	Stop upper layers calling the device hard_start_xmit routine.
 *	Used for flow control when transmit resources are unavailable.
 */
static inline void netif_stop_queue(struct net_device *dev)
{
	netif_tx_stop_queue(netdev_get_tx_queue(dev, 0));
}

void netif_tx_stop_all_queues(struct net_device *dev);

static inline bool netif_tx_queue_stopped(const struct netdev_queue *dev_queue)
{
	return test_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
}

/**
 *	netif_queue_stopped - test if transmit queue is flowblocked
 *	@dev: network device
 *
 *	Test if transmit queue on device is currently unable to send.
 */
static inline bool netif_queue_stopped(const struct net_device *dev)
{
	return netif_tx_queue_stopped(netdev_get_tx_queue(dev, 0));
}

static inline bool netif_xmit_stopped(const struct netdev_queue *dev_queue)
{
	return dev_queue->state & QUEUE_STATE_ANY_XOFF;
}

static inline bool
netif_xmit_frozen_or_stopped(const struct netdev_queue *dev_queue)
{
	return dev_queue->state & QUEUE_STATE_ANY_XOFF_OR_FROZEN;
}

static inline bool
netif_xmit_frozen_or_drv_stopped(const struct netdev_queue *dev_queue)
{
	return dev_queue->state & QUEUE_STATE_DRV_XOFF_OR_FROZEN;
}

/**
 *	netdev_txq_bql_enqueue_prefetchw - prefetch bql data for write
 *	@dev_queue: pointer to transmit queue
 *
 * BQL enabled drivers might use this helper in their ndo_start_xmit(),
 * to give appropriate hint to the CPU.
 */
static inline void netdev_txq_bql_enqueue_prefetchw(struct netdev_queue *dev_queue)
{
#ifdef CONFIG_BQL
	prefetchw(&dev_queue->dql.num_queued);
#endif
}

/**
 *	netdev_txq_bql_complete_prefetchw - prefetch bql data for write
 *	@dev_queue: pointer to transmit queue
 *
 * BQL enabled drivers might use this helper in their TX completion path,
 * to give appropriate hint to the CPU.
 */
static inline void netdev_txq_bql_complete_prefetchw(struct netdev_queue *dev_queue)
{
#ifdef CONFIG_BQL
	prefetchw(&dev_queue->dql.limit);
#endif
}

static inline void netdev_tx_sent_queue(struct netdev_queue *dev_queue,
					unsigned int bytes)
{
#ifdef CONFIG_BQL
	dql_queued(&dev_queue->dql, bytes);

	if (likely(dql_avail(&dev_queue->dql) >= 0))
		return;

	set_bit(__QUEUE_STATE_STACK_XOFF, &dev_queue->state);

	/*
	 * The XOFF flag must be set before checking the dql_avail below,
	 * because in netdev_tx_completed_queue we update the dql_completed
	 * before checking the XOFF flag.
	 */
	smp_mb();

	/* check again in case another CPU has just made room avail */
	if (unlikely(dql_avail(&dev_queue->dql) >= 0))
		clear_bit(__QUEUE_STATE_STACK_XOFF, &dev_queue->state);
#endif
}

/**
 * 	netdev_sent_queue - report the number of bytes queued to hardware
 * 	@dev: network device
 * 	@bytes: number of bytes queued to the hardware device queue
 *
 * 	Report the number of bytes queued for sending/completion to the network
 * 	device hardware queue. @bytes should be a good approximation and should
 * 	exactly match netdev_completed_queue() @bytes
 */
static inline void netdev_sent_queue(struct net_device *dev, unsigned int bytes)
{
	netdev_tx_sent_queue(netdev_get_tx_queue(dev, 0), bytes);
}

static inline void netdev_tx_completed_queue(struct netdev_queue *dev_queue,
					     unsigned int pkts, unsigned int bytes)
{
#ifdef CONFIG_BQL
	if (unlikely(!bytes))
		return;

	dql_completed(&dev_queue->dql, bytes);

	/*
	 * Without the memory barrier there is a small possiblity that
	 * netdev_tx_sent_queue will miss the update and cause the queue to
	 * be stopped forever
	 */
	smp_mb();

	if (dql_avail(&dev_queue->dql) < 0)
		return;

	if (test_and_clear_bit(__QUEUE_STATE_STACK_XOFF, &dev_queue->state))
		netif_schedule_queue(dev_queue);
#endif
}

/**
 * 	netdev_completed_queue - report bytes and packets completed by device
 * 	@dev: network device
 * 	@pkts: actual number of packets sent over the medium
 * 	@bytes: actual number of bytes sent over the medium
 *
 * 	Report the number of bytes and packets transmitted by the network device
 * 	hardware queue over the physical medium, @bytes must exactly match the
 * 	@bytes amount passed to netdev_sent_queue()
 */
static inline void netdev_completed_queue(struct net_device *dev,
					  unsigned int pkts, unsigned int bytes)
{
	netdev_tx_completed_queue(netdev_get_tx_queue(dev, 0), pkts, bytes);
}

static inline void netdev_tx_reset_queue(struct netdev_queue *q)
{
#ifdef CONFIG_BQL
	clear_bit(__QUEUE_STATE_STACK_XOFF, &q->state);
	dql_reset(&q->dql);
#endif
}

/**
 * 	netdev_reset_queue - reset the packets and bytes count of a network device
 * 	@dev_queue: network device
 *
 * 	Reset the bytes and packet count of a network device and clear the
 * 	software flow control OFF bit for this network device
 */
static inline void netdev_reset_queue(struct net_device *dev_queue)
{
	netdev_tx_reset_queue(netdev_get_tx_queue(dev_queue, 0));
}

/**
 * 	netdev_cap_txqueue - check if selected tx queue exceeds device queues
 * 	@dev: network device
 * 	@queue_index: given tx queue index
 *
 * 	Returns 0 if given tx queue index >= number of device tx queues,
 * 	otherwise returns the originally passed tx queue index.
 */
static inline u16 netdev_cap_txqueue(struct net_device *dev, u16 queue_index)
{
	if (unlikely(queue_index >= dev->real_num_tx_queues)) {
		net_warn_ratelimited("%s selects TX queue %d, but real number of TX queues is %d\n",
				     dev->name, queue_index,
				     dev->real_num_tx_queues);
		return 0;
	}

	return queue_index;
}

/**
 *	netif_running - test if up
 *	@dev: network device
 *
 *	Test if the device has been brought up.
 */
static inline bool netif_running(const struct net_device *dev)
{
	return test_bit(__LINK_STATE_START, &dev->state);
}

/*
 * Routines to manage the subqueues on a device.  We only need start,
 * stop, and a check if it's stopped.  All other device management is
 * done at the overall netdevice level.
 * Also test the device if we're multiqueue.
 */

/**
 *	netif_start_subqueue - allow sending packets on subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Start individual transmit queue of a device with multiple transmit queues.
 */
static inline void netif_start_subqueue(struct net_device *dev, u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);

	netif_tx_start_queue(txq);
}

/**
 *	netif_stop_subqueue - stop sending packets on subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Stop individual transmit queue of a device with multiple transmit queues.
 */
static inline void netif_stop_subqueue(struct net_device *dev, u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);
	netif_tx_stop_queue(txq);
}

/**
 *	netif_subqueue_stopped - test status of subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Check individual transmit queue of a device with multiple transmit queues.
 */
static inline bool __netif_subqueue_stopped(const struct net_device *dev,
					    u16 queue_index)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue_index);

	return netif_tx_queue_stopped(txq);
}

static inline bool netif_subqueue_stopped(const struct net_device *dev,
					  struct sk_buff *skb)
{
	return __netif_subqueue_stopped(dev, skb_get_queue_mapping(skb));
}

void netif_wake_subqueue(struct net_device *dev, u16 queue_index);

#ifdef CONFIG_XPS
int netif_set_xps_queue(struct net_device *dev, const struct cpumask *mask,
			u16 index);
#else
static inline int netif_set_xps_queue(struct net_device *dev,
				      const struct cpumask *mask,
				      u16 index)
{
	return 0;
}
#endif

u16 __skb_tx_hash(const struct net_device *dev, struct sk_buff *skb,
		  unsigned int num_tx_queues);

/*
 * Returns a Tx hash for the given packet when dev->real_num_tx_queues is used
 * as a distribution range limit for the returned value.
 */
static inline u16 skb_tx_hash(const struct net_device *dev,
			      struct sk_buff *skb)
{
	return __skb_tx_hash(dev, skb, dev->real_num_tx_queues);
}

/**
 *	netif_is_multiqueue - test if device has multiple transmit queues
 *	@dev: network device
 *
 * Check if device has multiple transmit queues
 */
static inline bool netif_is_multiqueue(const struct net_device *dev)
{
	return dev->num_tx_queues > 1;
}

int netif_set_real_num_tx_queues(struct net_device *dev, unsigned int txq);

#ifdef CONFIG_SYSFS
int netif_set_real_num_rx_queues(struct net_device *dev, unsigned int rxq);
#else
static inline int netif_set_real_num_rx_queues(struct net_device *dev,
						unsigned int rxq)
{
	return 0;
}
#endif

#ifdef CONFIG_SYSFS
static inline unsigned int get_netdev_rx_queue_index(
		struct netdev_rx_queue *queue)
{
	struct net_device *dev = queue->dev;
	int index = queue - dev->_rx;

	BUG_ON(index >= dev->num_rx_queues);
	return index;
}
#endif

#define DEFAULT_MAX_NUM_RSS_QUEUES	(8)
int netif_get_num_default_rss_queues(void);

enum skb_free_reason {
	SKB_REASON_CONSUMED,
	SKB_REASON_DROPPED,
};

void __dev_kfree_skb_irq(struct sk_buff *skb, enum skb_free_reason reason);
void __dev_kfree_skb_any(struct sk_buff *skb, enum skb_free_reason reason);

/*
 * It is not allowed to call kfree_skb() or consume_skb() from hardware
 * interrupt context or with hardware interrupts being disabled.
 * (in_irq() || irqs_disabled())
 *
 * We provide four helpers that can be used in following contexts :
 *
 * dev_kfree_skb_irq(skb) when caller drops a packet from irq context,
 *  replacing kfree_skb(skb)
 *
 * dev_consume_skb_irq(skb) when caller consumes a packet from irq context.
 *  Typically used in place of consume_skb(skb) in TX completion path
 *
 * dev_kfree_skb_any(skb) when caller doesn't know its current irq context,
 *  replacing kfree_skb(skb)
 *
 * dev_consume_skb_any(skb) when caller doesn't know its current irq context,
 *  and consumed a packet. Used in place of consume_skb(skb)
 */
static inline void dev_kfree_skb_irq(struct sk_buff *skb)
{
	__dev_kfree_skb_irq(skb, SKB_REASON_DROPPED);
}

static inline void dev_consume_skb_irq(struct sk_buff *skb)
{
	__dev_kfree_skb_irq(skb, SKB_REASON_CONSUMED);
}

static inline void dev_kfree_skb_any(struct sk_buff *skb)
{
	__dev_kfree_skb_any(skb, SKB_REASON_DROPPED);
}

static inline void dev_consume_skb_any(struct sk_buff *skb)
{
	__dev_kfree_skb_any(skb, SKB_REASON_CONSUMED);
}

int netif_rx(struct sk_buff *skb);
int netif_rx_ni(struct sk_buff *skb);
int netif_receive_skb(struct sk_buff *skb);
gro_result_t napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb);
void napi_gro_flush(struct napi_struct *napi, bool flush_old);
struct sk_buff *napi_get_frags(struct napi_struct *napi);
gro_result_t napi_gro_frags(struct napi_struct *napi);
struct packet_offload *gro_find_receive_by_type(__be16 type);
struct packet_offload *gro_find_complete_by_type(__be16 type);

static inline void napi_free_frags(struct napi_struct *napi)
{
	kfree_skb(napi->skb);
	napi->skb = NULL;
}

bool netdev_is_rx_handler_busy(struct net_device *dev);
int netdev_rx_handler_register(struct net_device *dev,
			       rx_handler_func_t *rx_handler,
			       void *rx_handler_data);
void netdev_rx_handler_unregister(struct net_device *dev);

bool dev_valid_name(const char *name);
int dev_ioctl(struct net *net, unsigned int cmd, void __user *);
int dev_ethtool(struct net *net, struct ifreq *);
unsigned int dev_get_flags(const struct net_device *);
int __dev_change_flags(struct net_device *, unsigned int flags);
int dev_change_flags(struct net_device *, unsigned int);
void __dev_notify_flags(struct net_device *, unsigned int old_flags,
			unsigned int gchanges);
int dev_change_name(struct net_device *, const char *);
int dev_set_alias(struct net_device *, const char *, size_t);
int dev_change_net_namespace(struct net_device *, struct net *, const char *);
int dev_set_mtu(struct net_device *, int);
void dev_set_group(struct net_device *, int);
int dev_set_mac_address(struct net_device *, struct sockaddr *);
int dev_change_carrier(struct net_device *, bool new_carrier);
int dev_get_phys_port_id(struct net_device *dev,
			 struct netdev_phys_item_id *ppid);
int dev_get_phys_port_name(struct net_device *dev,
			   char *name, size_t len);
int dev_change_proto_down(struct net_device *dev, bool proto_down);
int dev_change_xdp_fd(struct net_device *dev, int fd);
struct sk_buff *validate_xmit_skb_list(struct sk_buff *skb, struct net_device *dev);
struct sk_buff *dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev,
				    struct netdev_queue *txq, int *ret);
int __dev_forward_skb(struct net_device *dev, struct sk_buff *skb);
int dev_forward_skb(struct net_device *dev, struct sk_buff *skb);
bool is_skb_forwardable(const struct net_device *dev,
			const struct sk_buff *skb);

void dev_queue_xmit_nit(struct sk_buff *skb, struct net_device *dev);

extern int		netdev_budget;

/* Called by rtnetlink.c:rtnl_unlock() */
void netdev_run_todo(void);

/**
 *	dev_put - release reference to device
 *	@dev: network device
 *
 * Release reference to device to allow it to be freed.
 */
static inline void dev_put(struct net_device *dev)
{
	this_cpu_dec(*dev->pcpu_refcnt);
}

/**
 *	dev_hold - get reference to device
 *	@dev: network device
 *
 * Hold reference to device to keep it from being freed.
 */
static inline void dev_hold(struct net_device *dev)
{
	this_cpu_inc(*dev->pcpu_refcnt);
}

/* Carrier loss detection, dial on demand. The functions netif_carrier_on
 * and _off may be called from IRQ context, but it is caller
 * who is responsible for serialization of these calls.
 *
 * The name carrier is inappropriate, these functions should really be
 * called netif_lowerlayer_*() because they represent the state of any
 * kind of lower layer not just hardware media.
 */

void linkwatch_init_dev(struct net_device *dev);
void linkwatch_fire_event(struct net_device *dev);
void linkwatch_forget_dev(struct net_device *dev);

/**
 *	netif_carrier_ok - test if carrier present
 *	@dev: network device
 *
 * Check if carrier is present on device
 */
static inline bool netif_carrier_ok(const struct net_device *dev)
{
	return !test_bit(__LINK_STATE_NOCARRIER, &dev->state);
}

unsigned long dev_trans_start(struct net_device *dev);

void __netdev_watchdog_up(struct net_device *dev);

void netif_carrier_on(struct net_device *dev);

void netif_carrier_off(struct net_device *dev);

/**
 *	netif_dormant_on - mark device as dormant.
 *	@dev: network device
 *
 * Mark device as dormant (as per RFC2863).
 *
 * The dormant state indicates that the relevant interface is not
 * actually in a condition to pass packets (i.e., it is not 'up') but is
 * in a "pending" state, waiting for some external event.  For "on-
 * demand" interfaces, this new state identifies the situation where the
 * interface is waiting for events to place it in the up state.
 */
static inline void netif_dormant_on(struct net_device *dev)
{
	if (!test_and_set_bit(__LINK_STATE_DORMANT, &dev->state))
		linkwatch_fire_event(dev);
}

/**
 *	netif_dormant_off - set device as not dormant.
 *	@dev: network device
 *
 * Device is not in dormant state.
 */
static inline void netif_dormant_off(struct net_device *dev)
{
	if (test_and_clear_bit(__LINK_STATE_DORMANT, &dev->state))
		linkwatch_fire_event(dev);
}

/**
 *	netif_dormant - test if carrier present
 *	@dev: network device
 *
 * Check if carrier is present on device
 */
static inline bool netif_dormant(const struct net_device *dev)
{
	return test_bit(__LINK_STATE_DORMANT, &dev->state);
}


/**
 *	netif_oper_up - test if device is operational
 *	@dev: network device
 *
 * Check if carrier is operational
 */
static inline bool netif_oper_up(const struct net_device *dev)
{
	return (dev->operstate == IF_OPER_UP ||
		dev->operstate == IF_OPER_UNKNOWN /* backward compat */);
}

/**
 *	netif_device_present - is device available or removed
 *	@dev: network device
 *
 * Check if device has not been removed from system.
 */
static inline bool netif_device_present(struct net_device *dev)
{
	return test_bit(__LINK_STATE_PRESENT, &dev->state);
}

void netif_device_detach(struct net_device *dev);

void netif_device_attach(struct net_device *dev);

/*
 * Network interface message level settings
 */

enum {
	NETIF_MSG_DRV		= 0x0001,
	NETIF_MSG_PROBE		= 0x0002,
	NETIF_MSG_LINK		= 0x0004,
	NETIF_MSG_TIMER		= 0x0008,
	NETIF_MSG_IFDOWN	= 0x0010,
	NETIF_MSG_IFUP		= 0x0020,
	NETIF_MSG_RX_ERR	= 0x0040,
	NETIF_MSG_TX_ERR	= 0x0080,
	NETIF_MSG_TX_QUEUED	= 0x0100,
	NETIF_MSG_INTR		= 0x0200,
	NETIF_MSG_TX_DONE	= 0x0400,
	NETIF_MSG_RX_STATUS	= 0x0800,
	NETIF_MSG_PKTDATA	= 0x1000,
	NETIF_MSG_HW		= 0x2000,
	NETIF_MSG_WOL		= 0x4000,
};

#define netif_msg_drv(p)	((p)->msg_enable & NETIF_MSG_DRV)
#define netif_msg_probe(p)	((p)->msg_enable & NETIF_MSG_PROBE)
#define netif_msg_link(p)	((p)->msg_enable & NETIF_MSG_LINK)
#define netif_msg_timer(p)	((p)->msg_enable & NETIF_MSG_TIMER)
#define netif_msg_ifdown(p)	((p)->msg_enable & NETIF_MSG_IFDOWN)
#define netif_msg_ifup(p)	((p)->msg_enable & NETIF_MSG_IFUP)
#define netif_msg_rx_err(p)	((p)->msg_enable & NETIF_MSG_RX_ERR)
#define netif_msg_tx_err(p)	((p)->msg_enable & NETIF_MSG_TX_ERR)
#define netif_msg_tx_queued(p)	((p)->msg_enable & NETIF_MSG_TX_QUEUED)
#define netif_msg_intr(p)	((p)->msg_enable & NETIF_MSG_INTR)
#define netif_msg_tx_done(p)	((p)->msg_enable & NETIF_MSG_TX_DONE)
#define netif_msg_rx_status(p)	((p)->msg_enable & NETIF_MSG_RX_STATUS)
#define netif_msg_pktdata(p)	((p)->msg_enable & NETIF_MSG_PKTDATA)
#define netif_msg_hw(p)		((p)->msg_enable & NETIF_MSG_HW)
#define netif_msg_wol(p)	((p)->msg_enable & NETIF_MSG_WOL)

static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{
	/* use default */
	if (debug_value < 0 || debug_value >= (sizeof(u32) * 8))
		return default_msg_enable_bits;
	if (debug_value == 0)	/* no output */
		return 0;
	/* set low N bits */
	return (1 << debug_value) - 1;
}

static inline void __netif_tx_lock(struct netdev_queue *txq, int cpu)
{
	spin_lock(&txq->_xmit_lock);
	txq->xmit_lock_owner = cpu;
}

static inline void __netif_tx_lock_bh(struct netdev_queue *txq)
{
	spin_lock_bh(&txq->_xmit_lock);
	txq->xmit_lock_owner = smp_processor_id();
}

static inline bool __netif_tx_trylock(struct netdev_queue *txq)
{
	bool ok = spin_trylock(&txq->_xmit_lock);
	if (likely(ok))
		txq->xmit_lock_owner = smp_processor_id();
	return ok;
}

static inline void __netif_tx_unlock(struct netdev_queue *txq)
{
	txq->xmit_lock_owner = -1;
	spin_unlock(&txq->_xmit_lock);
}

static inline void __netif_tx_unlock_bh(struct netdev_queue *txq)
{
	txq->xmit_lock_owner = -1;
	spin_unlock_bh(&txq->_xmit_lock);
}

static inline void txq_trans_update(struct netdev_queue *txq)
{
	if (txq->xmit_lock_owner != -1)
		txq->trans_start = jiffies;
}

/* legacy drivers only, netdev_start_xmit() sets txq->trans_start */
static inline void netif_trans_update(struct net_device *dev)
{
	struct netdev_queue *txq = netdev_get_tx_queue(dev, 0);

	if (txq->trans_start != jiffies)
		txq->trans_start = jiffies;
}

/**
 *	netif_tx_lock - grab network device transmit lock
 *	@dev: network device
 *
 * Get network device transmit lock
 */
static inline void netif_tx_lock(struct net_device *dev)
{
	unsigned int i;
	int cpu;

	spin_lock(&dev->tx_global_lock);
	cpu = smp_processor_id();
	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);

		/* We are the only thread of execution doing a
		 * freeze, but we have to grab the _xmit_lock in
		 * order to synchronize with threads which are in
		 * the ->hard_start_xmit() handler and already
		 * checked the frozen bit.
		 */
		__netif_tx_lock(txq, cpu);
		set_bit(__QUEUE_STATE_FROZEN, &txq->state);
		__netif_tx_unlock(txq);
	}
}

static inline void netif_tx_lock_bh(struct net_device *dev)
{
	local_bh_disable();
	netif_tx_lock(dev);
}

static inline void netif_tx_unlock(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);

		/* No need to grab the _xmit_lock here.  If the
		 * queue is not stopped for another reason, we
		 * force a schedule.
		 */
		clear_bit(__QUEUE_STATE_FROZEN, &txq->state);
		netif_schedule_queue(txq);
	}
	spin_unlock(&dev->tx_global_lock);
}

static inline void netif_tx_unlock_bh(struct net_device *dev)
{
	netif_tx_unlock(dev);
	local_bh_enable();
}

#define HARD_TX_LOCK(dev, txq, cpu) {			\
	if ((dev->features & NETIF_F_LLTX) == 0) {	\
		__netif_tx_lock(txq, cpu);		\
	}						\
}

#define HARD_TX_TRYLOCK(dev, txq)			\
	(((dev->features & NETIF_F_LLTX) == 0) ?	\
		__netif_tx_trylock(txq) :		\
		true )

#define HARD_TX_UNLOCK(dev, txq) {			\
	if ((dev->features & NETIF_F_LLTX) == 0) {	\
		__netif_tx_unlock(txq);			\
	}						\
}

static inline void netif_tx_disable(struct net_device *dev)
{
	unsigned int i;
	int cpu;

	local_bh_disable();
	cpu = smp_processor_id();
	for (i = 0; i < dev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(dev, i);

		__netif_tx_lock(txq, cpu);
		netif_tx_stop_queue(txq);
		__netif_tx_unlock(txq);
	}
	local_bh_enable();
}

static inline void netif_addr_lock(struct net_device *dev)
{
	spin_lock(&dev->addr_list_lock);
}

static inline void netif_addr_lock_nested(struct net_device *dev)
{
	int subclass = SINGLE_DEPTH_NESTING;

	if (dev->netdev_ops->ndo_get_lock_subclass)
		subclass = dev->netdev_ops->ndo_get_lock_subclass(dev);

	spin_lock_nested(&dev->addr_list_lock, subclass);
}

static inline void netif_addr_lock_bh(struct net_device *dev)
{
	spin_lock_bh(&dev->addr_list_lock);
}

static inline void netif_addr_unlock(struct net_device *dev)
{
	spin_unlock(&dev->addr_list_lock);
}

static inline void netif_addr_unlock_bh(struct net_device *dev)
{
	spin_unlock_bh(&dev->addr_list_lock);
}

/*
 * dev_addrs walker. Should be used only for read access. Call with
 * rcu_read_lock held.
 */
#define for_each_dev_addr(dev, ha) \
		list_for_each_entry_rcu(ha, &dev->dev_addrs.list, list)

/* These functions live elsewhere (drivers/net/net_init.c, but related) */

void ether_setup(struct net_device *dev);

/* Support for loadable net-drivers */
struct net_device *alloc_netdev_mqs(int sizeof_priv, const char *name,
				    unsigned char name_assign_type,
				    void (*setup)(struct net_device *),
				    unsigned int txqs, unsigned int rxqs);
#define alloc_netdev(sizeof_priv, name, name_assign_type, setup) \
	alloc_netdev_mqs(sizeof_priv, name, name_assign_type, setup, 1, 1)

#define alloc_netdev_mq(sizeof_priv, name, name_assign_type, setup, count) \
	alloc_netdev_mqs(sizeof_priv, name, name_assign_type, setup, count, \
			 count)

int register_netdev(struct net_device *dev);
void unregister_netdev(struct net_device *dev);

/* General hardware address lists handling functions */
int __hw_addr_sync(struct netdev_hw_addr_list *to_list,
		   struct netdev_hw_addr_list *from_list, int addr_len);
void __hw_addr_unsync(struct netdev_hw_addr_list *to_list,
		      struct netdev_hw_addr_list *from_list, int addr_len);
int __hw_addr_sync_dev(struct netdev_hw_addr_list *list,
		       struct net_device *dev,
		       int (*sync)(struct net_device *, const unsigned char *),
		       int (*unsync)(struct net_device *,
				     const unsigned char *));
void __hw_addr_unsync_dev(struct netdev_hw_addr_list *list,
			  struct net_device *dev,
			  int (*unsync)(struct net_device *,
					const unsigned char *));
void __hw_addr_init(struct netdev_hw_addr_list *list);

/* Functions used for device addresses handling */
int dev_addr_add(struct net_device *dev, const unsigned char *addr,
		 unsigned char addr_type);
int dev_addr_del(struct net_device *dev, const unsigned char *addr,
		 unsigned char addr_type);
void dev_addr_flush(struct net_device *dev);
int dev_addr_init(struct net_device *dev);

/* Functions used for unicast addresses handling */
int dev_uc_add(struct net_device *dev, const unsigned char *addr);
int dev_uc_add_excl(struct net_device *dev, const unsigned char *addr);
int dev_uc_del(struct net_device *dev, const unsigned char *addr);
int dev_uc_sync(struct net_device *to, struct net_device *from);
int dev_uc_sync_multiple(struct net_device *to, struct net_device *from);
void dev_uc_unsync(struct net_device *to, struct net_device *from);
void dev_uc_flush(struct net_device *dev);
void dev_uc_init(struct net_device *dev);

/**
 *  __dev_uc_sync - Synchonize device's unicast list
 *  @dev:  device to sync
 *  @sync: function to call if address should be added
 *  @unsync: function to call if address should be removed
 *
 *  Add newly added addresses to the interface, and release
 *  addresses that have been deleted.
 */
static inline int __dev_uc_sync(struct net_device *dev,
				int (*sync)(struct net_device *,
					    const unsigned char *),
				int (*unsync)(struct net_device *,
					      const unsigned char *))
{
	return __hw_addr_sync_dev(&dev->uc, dev, sync, unsync);
}

/**
 *  __dev_uc_unsync - Remove synchronized addresses from device
 *  @dev:  device to sync
 *  @unsync: function to call if address should be removed
 *
 *  Remove all addresses that were added to the device by dev_uc_sync().
 */
static inline void __dev_uc_unsync(struct net_device *dev,
				   int (*unsync)(struct net_device *,
						 const unsigned char *))
{
	__hw_addr_unsync_dev(&dev->uc, dev, unsync);
}

/* Functions used for multicast addresses handling */
int dev_mc_add(struct net_device *dev, const unsigned char *addr);
int dev_mc_add_global(struct net_device *dev, const unsigned char *addr);
int dev_mc_add_excl(struct net_device *dev, const unsigned char *addr);
int dev_mc_del(struct net_device *dev, const unsigned char *addr);
int dev_mc_del_global(struct net_device *dev, const unsigned char *addr);
int dev_mc_sync(struct net_device *to, struct net_device *from);
int dev_mc_sync_multiple(struct net_device *to, struct net_device *from);
void dev_mc_unsync(struct net_device *to, struct net_device *from);
void dev_mc_flush(struct net_device *dev);
void dev_mc_init(struct net_device *dev);

/**
 *  __dev_mc_sync - Synchonize device's multicast list
 *  @dev:  device to sync
 *  @sync: function to call if address should be added
 *  @unsync: function to call if address should be removed
 *
 *  Add newly added addresses to the interface, and release
 *  addresses that have been deleted.
 */
static inline int __dev_mc_sync(struct net_device *dev,
				int (*sync)(struct net_device *,
					    const unsigned char *),
				int (*unsync)(struct net_device *,
					      const unsigned char *))
{
	return __hw_addr_sync_dev(&dev->mc, dev, sync, unsync);
}

/**
 *  __dev_mc_unsync - Remove synchronized addresses from device
 *  @dev:  device to sync
 *  @unsync: function to call if address should be removed
 *
 *  Remove all addresses that were added to the device by dev_mc_sync().
 */
static inline void __dev_mc_unsync(struct net_device *dev,
				   int (*unsync)(struct net_device *,
						 const unsigned char *))
{
	__hw_addr_unsync_dev(&dev->mc, dev, unsync);
}

/* Functions used for secondary unicast and multicast support */
void dev_set_rx_mode(struct net_device *dev);
void __dev_set_rx_mode(struct net_device *dev);
int dev_set_promiscuity(struct net_device *dev, int inc);
int dev_set_allmulti(struct net_device *dev, int inc);
void netdev_state_change(struct net_device *dev);
void netdev_notify_peers(struct net_device *dev);
void netdev_features_change(struct net_device *dev);
/* Load a device via the kmod */
void dev_load(struct net *net, const char *name);
struct rtnl_link_stats64 *dev_get_stats(struct net_device *dev,
					struct rtnl_link_stats64 *storage);
void netdev_stats_to_stats64(struct rtnl_link_stats64 *stats64,
			     const struct net_device_stats *netdev_stats);

extern int		netdev_max_backlog;
extern int		netdev_tstamp_prequeue;
extern int		weight_p;

bool netdev_has_upper_dev(struct net_device *dev, struct net_device *upper_dev);
struct net_device *netdev_upper_get_next_dev_rcu(struct net_device *dev,
						     struct list_head **iter);
struct net_device *netdev_all_upper_get_next_dev_rcu(struct net_device *dev,
						     struct list_head **iter);

/* iterate through upper list, must be called under RCU read lock */
#define netdev_for_each_upper_dev_rcu(dev, updev, iter) \
	for (iter = &(dev)->adj_list.upper, \
	     updev = netdev_upper_get_next_dev_rcu(dev, &(iter)); \
	     updev; \
	     updev = netdev_upper_get_next_dev_rcu(dev, &(iter)))

/* iterate through upper list, must be called under RCU read lock */
#define netdev_for_each_all_upper_dev_rcu(dev, updev, iter) \
	for (iter = &(dev)->all_adj_list.upper, \
	     updev = netdev_all_upper_get_next_dev_rcu(dev, &(iter)); \
	     updev; \
	     updev = netdev_all_upper_get_next_dev_rcu(dev, &(iter)))

void *netdev_lower_get_next_private(struct net_device *dev,
				    struct list_head **iter);
void *netdev_lower_get_next_private_rcu(struct net_device *dev,
					struct list_head **iter);

#define netdev_for_each_lower_private(dev, priv, iter) \
	for (iter = (dev)->adj_list.lower.next, \
	     priv = netdev_lower_get_next_private(dev, &(iter)); \
	     priv; \
	     priv = netdev_lower_get_next_private(dev, &(iter)))

#define netdev_for_each_lower_private_rcu(dev, priv, iter) \
	for (iter = &(dev)->adj_list.lower, \
	     priv = netdev_lower_get_next_private_rcu(dev, &(iter)); \
	     priv; \
	     priv = netdev_lower_get_next_private_rcu(dev, &(iter)))

void *netdev_lower_get_next(struct net_device *dev,
				struct list_head **iter);

#define netdev_for_each_lower_dev(dev, ldev, iter) \
	for (iter = (dev)->adj_list.lower.next, \
	     ldev = netdev_lower_get_next(dev, &(iter)); \
	     ldev; \
	     ldev = netdev_lower_get_next(dev, &(iter)))

struct net_device *netdev_all_lower_get_next(struct net_device *dev,
					     struct list_head **iter);
struct net_device *netdev_all_lower_get_next_rcu(struct net_device *dev,
						 struct list_head **iter);

#define netdev_for_each_all_lower_dev(dev, ldev, iter) \
	for (iter = (dev)->all_adj_list.lower.next, \
	     ldev = netdev_all_lower_get_next(dev, &(iter)); \
	     ldev; \
	     ldev = netdev_all_lower_get_next(dev, &(iter)))

#define netdev_for_each_all_lower_dev_rcu(dev, ldev, iter) \
	for (iter = &(dev)->all_adj_list.lower, \
	     ldev = netdev_all_lower_get_next_rcu(dev, &(iter)); \
	     ldev; \
	     ldev = netdev_all_lower_get_next_rcu(dev, &(iter)))

void *netdev_adjacent_get_private(struct list_head *adj_list);
void *netdev_lower_get_first_private_rcu(struct net_device *dev);
struct net_device *netdev_master_upper_dev_get(struct net_device *dev);
struct net_device *netdev_master_upper_dev_get_rcu(struct net_device *dev);
int netdev_upper_dev_link(struct net_device *dev, struct net_device *upper_dev);
int netdev_master_upper_dev_link(struct net_device *dev,
				 struct net_device *upper_dev,
				 void *upper_priv, void *upper_info);
void netdev_upper_dev_unlink(struct net_device *dev,
			     struct net_device *upper_dev);
void netdev_adjacent_rename_links(struct net_device *dev, char *oldname);
void *netdev_lower_dev_get_private(struct net_device *dev,
				   struct net_device *lower_dev);
void netdev_lower_state_changed(struct net_device *lower_dev,
				void *lower_state_info);
int netdev_default_l2upper_neigh_construct(struct net_device *dev,
					   struct neighbour *n);
void netdev_default_l2upper_neigh_destroy(struct net_device *dev,
					  struct neighbour *n);

/* RSS keys are 40 or 52 bytes long */
#define NETDEV_RSS_KEY_LEN 52
extern u8 netdev_rss_key[NETDEV_RSS_KEY_LEN] __read_mostly;
void netdev_rss_key_fill(void *buffer, size_t len);

int dev_get_nest_level(struct net_device *dev);
int skb_checksum_help(struct sk_buff *skb);
struct sk_buff *__skb_gso_segment(struct sk_buff *skb,
				  netdev_features_t features, bool tx_path);
struct sk_buff *skb_mac_gso_segment(struct sk_buff *skb,
				    netdev_features_t features);

struct netdev_bonding_info {
	ifslave	slave;
	ifbond	master;
};

struct netdev_notifier_bonding_info {
	struct netdev_notifier_info info; /* must be first */
	struct netdev_bonding_info  bonding_info;
};

void netdev_bonding_info_change(struct net_device *dev,
				struct netdev_bonding_info *bonding_info);

static inline
struct sk_buff *skb_gso_segment(struct sk_buff *skb, netdev_features_t features)
{
	return __skb_gso_segment(skb, features, true);
}
__be16 skb_network_protocol(struct sk_buff *skb, int *depth);

static inline bool can_checksum_protocol(netdev_features_t features,
					 __be16 protocol)
{
	if (protocol == htons(ETH_P_FCOE))
		return !!(features & NETIF_F_FCOE_CRC);

	/* Assume this is an IP checksum (not SCTP CRC) */

	if (features & NETIF_F_HW_CSUM) {
		/* Can checksum everything */
		return true;
	}

	switch (protocol) {
	case htons(ETH_P_IP):
		return !!(features & NETIF_F_IP_CSUM);
	case htons(ETH_P_IPV6):
		return !!(features & NETIF_F_IPV6_CSUM);
	default:
		return false;
	}
}

/* Map an ethertype into IP protocol if possible */
static inline int eproto_to_ipproto(int eproto)
{
	switch (eproto) {
	case htons(ETH_P_IP):
		return IPPROTO_IP;
	case htons(ETH_P_IPV6):
		return IPPROTO_IPV6;
	default:
		return -1;
	}
}

#ifdef CONFIG_BUG
void netdev_rx_csum_fault(struct net_device *dev);
#else
static inline void netdev_rx_csum_fault(struct net_device *dev)
{
}
#endif
/* rx skb timestamps */
void net_enable_timestamp(void);
void net_disable_timestamp(void);

#ifdef CONFIG_PROC_FS
int __init dev_proc_init(void);
#else
#define dev_proc_init() 0
#endif

static inline netdev_tx_t __netdev_start_xmit(const struct net_device_ops *ops,
					      struct sk_buff *skb, struct net_device *dev,
					      bool more)
{
	skb->xmit_more = more ? 1 : 0;
	return ops->ndo_start_xmit(skb, dev);
}

static inline netdev_tx_t netdev_start_xmit(struct sk_buff *skb, struct net_device *dev,
					    struct netdev_queue *txq, bool more)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	int rc;

	rc = __netdev_start_xmit(ops, skb, dev, more);
	if (rc == NETDEV_TX_OK)
		txq_trans_update(txq);

	return rc;
}

int netdev_class_create_file_ns(struct class_attribute *class_attr,
				const void *ns);
void netdev_class_remove_file_ns(struct class_attribute *class_attr,
				 const void *ns);

static inline int netdev_class_create_file(struct class_attribute *class_attr)
{
	return netdev_class_create_file_ns(class_attr, NULL);
}

static inline void netdev_class_remove_file(struct class_attribute *class_attr)
{
	netdev_class_remove_file_ns(class_attr, NULL);
}

extern struct kobj_ns_type_operations net_ns_type_operations;

const char *netdev_drivername(const struct net_device *dev);

void linkwatch_run_queue(void);

static inline netdev_features_t netdev_intersect_features(netdev_features_t f1,
							  netdev_features_t f2)
{
	if ((f1 ^ f2) & NETIF_F_HW_CSUM) {
		if (f1 & NETIF_F_HW_CSUM)
			f1 |= (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM);
		else
			f2 |= (NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM);
	}

	return f1 & f2;
}

static inline netdev_features_t netdev_get_wanted_features(
	struct net_device *dev)
{
	return (dev->features & ~dev->hw_features) | dev->wanted_features;
}
netdev_features_t netdev_increment_features(netdev_features_t all,
	netdev_features_t one, netdev_features_t mask);

/* Allow TSO being used on stacked device :
 * Performing the GSO segmentation before last device
 * is a performance improvement.
 */
static inline netdev_features_t netdev_add_tso_features(netdev_features_t features,
							netdev_features_t mask)
{
	return netdev_increment_features(features, NETIF_F_ALL_TSO, mask);
}

int __netdev_update_features(struct net_device *dev);
void netdev_update_features(struct net_device *dev);
void netdev_change_features(struct net_device *dev);

void netif_stacked_transfer_operstate(const struct net_device *rootdev,
					struct net_device *dev);

netdev_features_t passthru_features_check(struct sk_buff *skb,
					  struct net_device *dev,
					  netdev_features_t features);
netdev_features_t netif_skb_features(struct sk_buff *skb);

static inline bool net_gso_ok(netdev_features_t features, int gso_type)
{
	netdev_features_t feature = (netdev_features_t)gso_type << NETIF_F_GSO_SHIFT;

	/* check flags correspondence */
	BUILD_BUG_ON(SKB_GSO_TCPV4   != (NETIF_F_TSO >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_UDP     != (NETIF_F_UFO >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_DODGY   != (NETIF_F_GSO_ROBUST >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TCP_ECN != (NETIF_F_TSO_ECN >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TCP_FIXEDID != (NETIF_F_TSO_MANGLEID >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TCPV6   != (NETIF_F_TSO6 >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_FCOE    != (NETIF_F_FSO >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_GRE     != (NETIF_F_GSO_GRE >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_GRE_CSUM != (NETIF_F_GSO_GRE_CSUM >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_IPXIP4  != (NETIF_F_GSO_IPXIP4 >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_IPXIP6  != (NETIF_F_GSO_IPXIP6 >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_UDP_TUNNEL != (NETIF_F_GSO_UDP_TUNNEL >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_UDP_TUNNEL_CSUM != (NETIF_F_GSO_UDP_TUNNEL_CSUM >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_PARTIAL != (NETIF_F_GSO_PARTIAL >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_TUNNEL_REMCSUM != (NETIF_F_GSO_TUNNEL_REMCSUM >> NETIF_F_GSO_SHIFT));
	BUILD_BUG_ON(SKB_GSO_SCTP    != (NETIF_F_GSO_SCTP >> NETIF_F_GSO_SHIFT));

	return (features & feature) == feature;
}

static inline bool skb_gso_ok(struct sk_buff *skb, netdev_features_t features)
{
	return net_gso_ok(features, skb_shinfo(skb)->gso_type) &&
	       (!skb_has_frag_list(skb) || (features & NETIF_F_FRAGLIST));
}

static inline bool netif_needs_gso(struct sk_buff *skb,
				   netdev_features_t features)
{
	return skb_is_gso(skb) && (!skb_gso_ok(skb, features) ||
		unlikely((skb->ip_summed != CHECKSUM_PARTIAL) &&
			 (skb->ip_summed != CHECKSUM_UNNECESSARY)));
}

static inline void netif_set_gso_max_size(struct net_device *dev,
					  unsigned int size)
{
	dev->gso_max_size = size;
}

static inline void skb_gso_error_unwind(struct sk_buff *skb, __be16 protocol,
					int pulled_hlen, u16 mac_offset,
					int mac_len)
{
	skb->protocol = protocol;
	skb->encapsulation = 1;
	skb_push(skb, pulled_hlen);
	skb_reset_transport_header(skb);
	skb->mac_header = mac_offset;
	skb->network_header = skb->mac_header + mac_len;
	skb->mac_len = mac_len;
}

static inline bool netif_is_macsec(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MACSEC;
}

static inline bool netif_is_macvlan(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MACVLAN;
}

static inline bool netif_is_macvlan_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_MACVLAN_PORT;
}

static inline bool netif_is_ipvlan(const struct net_device *dev)
{
	return dev->priv_flags & IFF_IPVLAN_SLAVE;
}

static inline bool netif_is_ipvlan_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_IPVLAN_MASTER;
}

static inline bool netif_is_bond_master(const struct net_device *dev)
{
	return dev->flags & IFF_MASTER && dev->priv_flags & IFF_BONDING;
}

static inline bool netif_is_bond_slave(const struct net_device *dev)
{
	return dev->flags & IFF_SLAVE && dev->priv_flags & IFF_BONDING;
}

static inline bool netif_supports_nofcs(struct net_device *dev)
{
	return dev->priv_flags & IFF_SUPP_NOFCS;
}

static inline bool netif_is_l3_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_L3MDEV_MASTER;
}

static inline bool netif_is_l3_slave(const struct net_device *dev)
{
	return dev->priv_flags & IFF_L3MDEV_SLAVE;
}

static inline bool netif_is_bridge_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_EBRIDGE;
}

static inline bool netif_is_bridge_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_BRIDGE_PORT;
}

static inline bool netif_is_ovs_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_OPENVSWITCH;
}

static inline bool netif_is_team_master(const struct net_device *dev)
{
	return dev->priv_flags & IFF_TEAM;
}

static inline bool netif_is_team_port(const struct net_device *dev)
{
	return dev->priv_flags & IFF_TEAM_PORT;
}

static inline bool netif_is_lag_master(const struct net_device *dev)
{
	return netif_is_bond_master(dev) || netif_is_team_master(dev);
}

static inline bool netif_is_lag_port(const struct net_device *dev)
{
	return netif_is_bond_slave(dev) || netif_is_team_port(dev);
}

static inline bool netif_is_rxfh_configured(const struct net_device *dev)
{
	return dev->priv_flags & IFF_RXFH_CONFIGURED;
}

/* This device needs to keep skb dst for qdisc enqueue or ndo_start_xmit() */
static inline void netif_keep_dst(struct net_device *dev)
{
	dev->priv_flags &= ~(IFF_XMIT_DST_RELEASE | IFF_XMIT_DST_RELEASE_PERM);
}

/* return true if dev can't cope with mtu frames that need vlan tag insertion */
static inline bool netif_reduces_vlan_mtu(struct net_device *dev)
{
	/* TODO: reserve and use an additional IFF bit, if we get more users */
	return dev->priv_flags & IFF_MACSEC;
}

extern struct pernet_operations __net_initdata loopback_net_ops;

/* Logging, debugging and troubleshooting/diagnostic helpers. */

/* netdev_printk helpers, similar to dev_printk */

static inline const char *netdev_name(const struct net_device *dev)
{
	if (!dev->name[0] || strchr(dev->name, '%'))
		return "(unnamed net_device)";
	return dev->name;
}

static inline const char *netdev_reg_state(const struct net_device *dev)
{
	switch (dev->reg_state) {
	case NETREG_UNINITIALIZED: return " (uninitialized)";
	case NETREG_REGISTERED: return "";
	case NETREG_UNREGISTERING: return " (unregistering)";
	case NETREG_UNREGISTERED: return " (unregistered)";
	case NETREG_RELEASED: return " (released)";
	case NETREG_DUMMY: return " (dummy)";
	}

	WARN_ONCE(1, "%s: unknown reg_state %d\n", dev->name, dev->reg_state);
	return " (unknown)";
}

__printf(3, 4)
void netdev_printk(const char *level, const struct net_device *dev,
		   const char *format, ...);
__printf(2, 3)
void netdev_emerg(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_alert(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_crit(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_err(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_warn(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_notice(const struct net_device *dev, const char *format, ...);
__printf(2, 3)
void netdev_info(const struct net_device *dev, const char *format, ...);

#define MODULE_ALIAS_NETDEV(device) \
	MODULE_ALIAS("netdev-" device)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define netdev_dbg(__dev, format, args...)			\
do {								\
	dynamic_netdev_dbg(__dev, format, ##args);		\
} while (0)
#elif defined(DEBUG)
#define netdev_dbg(__dev, format, args...)			\
	netdev_printk(KERN_DEBUG, __dev, format, ##args)
#else
#define netdev_dbg(__dev, format, args...)			\
({								\
	if (0)							\
		netdev_printk(KERN_DEBUG, __dev, format, ##args); \
})
#endif

#if defined(VERBOSE_DEBUG)
#define netdev_vdbg	netdev_dbg
#else

#define netdev_vdbg(dev, format, args...)			\
({								\
	if (0)							\
		netdev_printk(KERN_DEBUG, dev, format, ##args);	\
	0;							\
})
#endif

/*
 * netdev_WARN() acts like dev_printk(), but with the key difference
 * of using a WARN/WARN_ON to get the message out, including the
 * file/line information and a backtrace.
 */
#define netdev_WARN(dev, format, args...)			\
	WARN(1, "netdevice: %s%s\n" format, netdev_name(dev),	\
	     netdev_reg_state(dev), ##args)

/* netif printk helpers, similar to netdev_printk */

#define netif_printk(priv, type, level, dev, fmt, args...)	\
do {					  			\
	if (netif_msg_##type(priv))				\
		netdev_printk(level, (dev), fmt, ##args);	\
} while (0)

#define netif_level(level, priv, type, dev, fmt, args...)	\
do {								\
	if (netif_msg_##type(priv))				\
		netdev_##level(dev, fmt, ##args);		\
} while (0)

#define netif_emerg(priv, type, dev, fmt, args...)		\
	netif_level(emerg, priv, type, dev, fmt, ##args)
#define netif_alert(priv, type, dev, fmt, args...)		\
	netif_level(alert, priv, type, dev, fmt, ##args)
#define netif_crit(priv, type, dev, fmt, args...)		\
	netif_level(crit, priv, type, dev, fmt, ##args)
#define netif_err(priv, type, dev, fmt, args...)		\
	netif_level(err, priv, type, dev, fmt, ##args)
#define netif_warn(priv, type, dev, fmt, args...)		\
	netif_level(warn, priv, type, dev, fmt, ##args)
#define netif_notice(priv, type, dev, fmt, args...)		\
	netif_level(notice, priv, type, dev, fmt, ##args)
#define netif_info(priv, type, dev, fmt, args...)		\
	netif_level(info, priv, type, dev, fmt, ##args)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define netif_dbg(priv, type, netdev, format, args...)		\
do {								\
	if (netif_msg_##type(priv))				\
		dynamic_netdev_dbg(netdev, format, ##args);	\
} while (0)
#elif defined(DEBUG)
#define netif_dbg(priv, type, dev, format, args...)		\
	netif_printk(priv, type, KERN_DEBUG, dev, format, ##args)
#else
#define netif_dbg(priv, type, dev, format, args...)			\
({									\
	if (0)								\
		netif_printk(priv, type, KERN_DEBUG, dev, format, ##args); \
	0;								\
})
#endif

#if defined(VERBOSE_DEBUG)
#define netif_vdbg	netif_dbg
#else
#define netif_vdbg(priv, type, dev, format, args...)		\
({								\
	if (0)							\
		netif_printk(priv, type, KERN_DEBUG, dev, format, ##args); \
	0;							\
})
#endif

/*
 *	The list of packet types we will receive (as opposed to discard)
 *	and the routines to invoke.
 *
 *	Why 16. Because with 16 the only overlap we get on a hash of the
 *	low nibble of the protocol value is RARP/SNAP/X.25.
 *
 *      NOTE:  That is no longer true with the addition of VLAN tags.  Not
 *             sure which should go first, but I bet it won't make much
 *             difference if we are running VLANs.  The good news is that
 *             this protocol won't be in the list unless compiled in, so
 *             the average user (w/out VLANs) will not be adversely affected.
 *             --BLG
 *
 *		0800	IP
 *		8100    802.1Q VLAN
 *		0001	802.3
 *		0002	AX.25
 *		0004	802.2
 *		8035	RARP
 *		0005	SNAP
 *		0805	X.25
 *		0806	ARP
 *		8137	IPX
 *		0009	Localtalk
 *		86DD	IPv6
 */
#define PTYPE_HASH_SIZE	(16)
#define PTYPE_HASH_MASK	(PTYPE_HASH_SIZE - 1)

#endif	/* _LINUX_NETDEVICE_H */