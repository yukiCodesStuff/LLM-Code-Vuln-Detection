struct ipoib_cm_tx {
	struct ib_cm_id	    *id;
	struct ib_qp	    *qp;
	struct list_head     list;
	struct net_device   *dev;
	struct ipoib_neigh  *neigh;
	struct ipoib_path   *path;
	struct ipoib_tx_buf *tx_ring;
	unsigned int	     tx_head;
	unsigned int	     tx_tail;
	unsigned long	     flags;
	u32		     mtu;
	unsigned int         max_send_sge;
};

struct ipoib_cm_rx_buf {
	struct sk_buff *skb;
	u64 mapping[IPOIB_CM_RX_SG];
};

struct ipoib_cm_dev_priv {
	struct ib_srq	       *srq;
	struct ipoib_cm_rx_buf *srq_ring;
	struct ib_cm_id	       *id;
	struct list_head	passive_ids;   /* state: LIVE */
	struct list_head	rx_error_list; /* state: ERROR */
	struct list_head	rx_flush_list; /* state: FLUSH, drain not started */
	struct list_head	rx_drain_list; /* state: FLUSH, drain started */
	struct list_head	rx_reap_list;  /* state: FLUSH, drain done */
	struct work_struct      start_task;
	struct work_struct      reap_task;
	struct work_struct      skb_task;
	struct work_struct      rx_reap_task;
	struct delayed_work     stale_task;
	struct sk_buff_head     skb_queue;
	struct list_head	start_list;
	struct list_head	reap_list;
	struct ib_wc		ibwc[IPOIB_NUM_WC];
	struct ib_sge		rx_sge[IPOIB_CM_RX_SG];
	struct ib_recv_wr       rx_wr;
	int			nonsrq_conn_qp;
	int			max_cm_mtu;
	int			num_frags;
};

struct ipoib_ethtool_st {
	u16     coalesce_usecs;
	u16     max_coalesced_frames;
};

struct ipoib_neigh_table;

struct ipoib_neigh_hash {
	struct ipoib_neigh_table       *ntbl;
	struct ipoib_neigh __rcu      **buckets;
	struct rcu_head			rcu;
	u32				mask;
	u32				size;
};

struct ipoib_neigh_table {
	struct ipoib_neigh_hash __rcu  *htbl;
	atomic_t			entries;
	struct completion		flushed;
	struct completion		deleted;
};

struct ipoib_qp_state_validate {
	struct work_struct work;
	struct ipoib_dev_priv   *priv;
};

/*
 * Device private locking: network stack tx_lock protects members used
 * in TX fast path, lock protects everything else.  lock nests inside
 * of tx_lock (ie tx_lock must be acquired first if needed).
 */
struct ipoib_dev_priv {
	spinlock_t lock;

	struct net_device *dev;
	void (*next_priv_destructor)(struct net_device *dev);

	struct napi_struct send_napi;
	struct napi_struct recv_napi;

	unsigned long flags;

	/*
	 * This protects access to the child_intfs list.
	 * To READ from child_intfs the RTNL or vlan_rwsem read side must be
	 * held.  To WRITE RTNL and the vlan_rwsem write side must be held (in
	 * that order) This lock exists because we have a few contexts where
	 * we need the child_intfs, but do not want to grab the RTNL.
	 */
	struct rw_semaphore vlan_rwsem;
	struct mutex mcast_mutex;

	struct rb_root  path_tree;
	struct list_head path_list;

	struct ipoib_neigh_table ntbl;

	struct ipoib_mcast *broadcast;
	struct list_head multicast_list;
	struct rb_root multicast_tree;

	struct workqueue_struct *wq;
	struct delayed_work mcast_task;
	struct work_struct carrier_on_task;
	struct work_struct flush_light;
	struct work_struct flush_normal;
	struct work_struct flush_heavy;
	struct work_struct restart_task;
	struct delayed_work ah_reap_task;
	struct delayed_work neigh_reap_task;
	struct ib_device *ca;
	u8		  port;
	u16		  pkey;
	u16		  pkey_index;
	struct ib_pd	 *pd;
	struct ib_cq	 *recv_cq;
	struct ib_cq	 *send_cq;
	struct ib_qp	 *qp;
	u32		  qkey;

	union ib_gid local_gid;
	u32	     local_lid;

	unsigned int admin_mtu;
	unsigned int mcast_mtu;
	unsigned int max_ib_mtu;

	struct ipoib_rx_buf *rx_ring;

	struct ipoib_tx_buf *tx_ring;
	unsigned int	     tx_head;
	unsigned int	     tx_tail;
	struct ib_sge	     tx_sge[MAX_SKB_FRAGS + 1];
	struct ib_ud_wr      tx_wr;
	struct ib_wc	     send_wc[MAX_SEND_CQE];

	struct ib_recv_wr    rx_wr;
	struct ib_sge	     rx_sge[IPOIB_UD_RX_SG];

	struct ib_wc ibwc[IPOIB_NUM_WC];

	struct list_head dead_ahs;

	struct ib_event_handler event_handler;

	struct net_device *parent;
	struct list_head child_intfs;
	struct list_head list;
	int    child_type;

#ifdef CONFIG_INFINIBAND_IPOIB_CM
	struct ipoib_cm_dev_priv cm;
#endif

#ifdef CONFIG_INFINIBAND_IPOIB_DEBUG
	struct list_head fs_list;
	struct dentry *mcg_dentry;
	struct dentry *path_dentry;
#endif
	u64	hca_caps;
	struct ipoib_ethtool_st ethtool;
	unsigned int max_send_sge;
	bool sm_fullmember_sendonly_support;
	const struct net_device_ops	*rn_ops;
};

struct ipoib_ah {
	struct net_device *dev;
	struct ib_ah	  *ah;
	struct list_head   list;
	struct kref	   ref;
	unsigned int	   last_send;
	int  		   valid;
};

struct ipoib_path {
	struct net_device    *dev;
	struct sa_path_rec pathrec;
	struct ipoib_ah      *ah;
	struct sk_buff_head   queue;

	struct list_head      neigh_list;

	int		      query_id;
	struct ib_sa_query   *query;
	struct completion     done;

	struct rb_node	      rb_node;
	struct list_head      list;
};

struct ipoib_neigh {
	struct ipoib_ah    *ah;
#ifdef CONFIG_INFINIBAND_IPOIB_CM
	struct ipoib_cm_tx *cm;
#endif
	u8     daddr[INFINIBAND_ALEN];
	struct sk_buff_head queue;

	struct net_device *dev;

	struct list_head    list;
	struct ipoib_neigh __rcu *hnext;
	struct rcu_head     rcu;
	atomic_t	    refcnt;
	unsigned long       alive;
};

#define IPOIB_UD_MTU(ib_mtu)		(ib_mtu - IPOIB_ENCAP_LEN)
#define IPOIB_UD_BUF_SIZE(ib_mtu)	(ib_mtu + IB_GRH_BYTES)

void ipoib_neigh_dtor(struct ipoib_neigh *neigh);
static inline void ipoib_neigh_put(struct ipoib_neigh *neigh)
{
	if (atomic_dec_and_test(&neigh->refcnt))
		ipoib_neigh_dtor(neigh);
}
struct ipoib_neigh *ipoib_neigh_get(struct net_device *dev, u8 *daddr);
struct ipoib_neigh *ipoib_neigh_alloc(u8 *daddr,
				      struct net_device *dev);
void ipoib_neigh_free(struct ipoib_neigh *neigh);
void ipoib_del_neighs_by_gid(struct net_device *dev, u8 *gid);

extern struct workqueue_struct *ipoib_workqueue;

/* functions */

int ipoib_rx_poll(struct napi_struct *napi, int budget);
int ipoib_tx_poll(struct napi_struct *napi, int budget);
void ipoib_ib_rx_completion(struct ib_cq *cq, void *ctx_ptr);
void ipoib_ib_tx_completion(struct ib_cq *cq, void *ctx_ptr);

struct ipoib_ah *ipoib_create_ah(struct net_device *dev,
				 struct ib_pd *pd, struct rdma_ah_attr *attr);
void ipoib_free_ah(struct kref *kref);
static inline void ipoib_put_ah(struct ipoib_ah *ah)
{
	kref_put(&ah->ref, ipoib_free_ah);
}
int ipoib_open(struct net_device *dev);
void ipoib_intf_free(struct net_device *dev);
int ipoib_add_pkey_attr(struct net_device *dev);
int ipoib_add_umcast_attr(struct net_device *dev);

int ipoib_send(struct net_device *dev, struct sk_buff *skb,
	       struct ib_ah *address, u32 dqpn);
void ipoib_reap_ah(struct work_struct *work);

struct ipoib_path *__path_find(struct net_device *dev, void *gid);
void ipoib_mark_paths_invalid(struct net_device *dev);
void ipoib_flush_paths(struct net_device *dev);
struct net_device *ipoib_intf_alloc(struct ib_device *hca, u8 port,
				    const char *format);
int ipoib_intf_init(struct ib_device *hca, u8 port, const char *format,
		    struct net_device *dev);
void ipoib_ib_tx_timer_func(struct timer_list *t);
void ipoib_ib_dev_flush_light(struct work_struct *work);
void ipoib_ib_dev_flush_normal(struct work_struct *work);
void ipoib_ib_dev_flush_heavy(struct work_struct *work);
void ipoib_pkey_event(struct work_struct *work);
void ipoib_ib_dev_cleanup(struct net_device *dev);

int ipoib_ib_dev_open_default(struct net_device *dev);
int ipoib_ib_dev_open(struct net_device *dev);
int ipoib_ib_dev_stop(struct net_device *dev);
void ipoib_ib_dev_up(struct net_device *dev);
void ipoib_ib_dev_down(struct net_device *dev);
int ipoib_ib_dev_stop_default(struct net_device *dev);
void ipoib_pkey_dev_check_presence(struct net_device *dev);

void ipoib_mcast_join_task(struct work_struct *work);
void ipoib_mcast_carrier_on_task(struct work_struct *work);
void ipoib_mcast_send(struct net_device *dev, u8 *daddr, struct sk_buff *skb);

void ipoib_mcast_restart_task(struct work_struct *work);
void ipoib_mcast_start_thread(struct net_device *dev);
int ipoib_mcast_stop_thread(struct net_device *dev);

void ipoib_mcast_dev_down(struct net_device *dev);
void ipoib_mcast_dev_flush(struct net_device *dev);

int ipoib_dma_map_tx(struct ib_device *ca, struct ipoib_tx_buf *tx_req);
void ipoib_dma_unmap_tx(struct ipoib_dev_priv *priv,
			struct ipoib_tx_buf *tx_req);

struct rtnl_link_ops *ipoib_get_link_ops(void);

static inline void ipoib_build_sge(struct ipoib_dev_priv *priv,
				   struct ipoib_tx_buf *tx_req)
{
	int i, off;
	struct sk_buff *skb = tx_req->skb;
	skb_frag_t *frags = skb_shinfo(skb)->frags;
	int nr_frags = skb_shinfo(skb)->nr_frags;
	u64 *mapping = tx_req->mapping;

	if (skb_headlen(skb)) {
		priv->tx_sge[0].addr         = mapping[0];
		priv->tx_sge[0].length       = skb_headlen(skb);
		off = 1;
	} else
		off = 0;

	for (i = 0; i < nr_frags; ++i) {
		priv->tx_sge[i + off].addr = mapping[i + off];
		priv->tx_sge[i + off].length = skb_frag_size(&frags[i]);
	}
	priv->tx_wr.wr.num_sge	     = nr_frags + off;
}

#ifdef CONFIG_INFINIBAND_IPOIB_DEBUG
struct ipoib_mcast_iter *ipoib_mcast_iter_init(struct net_device *dev);
int ipoib_mcast_iter_next(struct ipoib_mcast_iter *iter);
void ipoib_mcast_iter_read(struct ipoib_mcast_iter *iter,
				  union ib_gid *gid,
				  unsigned long *created,
				  unsigned int *queuelen,
				  unsigned int *complete,
				  unsigned int *send_only);

struct ipoib_path_iter *ipoib_path_iter_init(struct net_device *dev);
int ipoib_path_iter_next(struct ipoib_path_iter *iter);
void ipoib_path_iter_read(struct ipoib_path_iter *iter,
			  struct ipoib_path *path);
#endif

int ipoib_mcast_attach(struct net_device *dev, struct ib_device *hca,
		       union ib_gid *mgid, u16 mlid, int set_qkey, u32 qkey);
int ipoib_mcast_detach(struct net_device *dev, struct ib_device *hca,
		       union ib_gid *mgid, u16 mlid);
void ipoib_mcast_remove_list(struct list_head *remove_list);
void ipoib_check_and_add_mcast_sendonly(struct ipoib_dev_priv *priv, u8 *mgid,
				struct list_head *remove_list);

int ipoib_init_qp(struct net_device *dev);
int ipoib_transport_dev_init(struct net_device *dev, struct ib_device *ca);
void ipoib_transport_dev_cleanup(struct net_device *dev);

void ipoib_event(struct ib_event_handler *handler,
		 struct ib_event *record);

int ipoib_vlan_add(struct net_device *pdev, unsigned short pkey);
int ipoib_vlan_delete(struct net_device *pdev, unsigned short pkey);

int __ipoib_vlan_add(struct ipoib_dev_priv *ppriv, struct ipoib_dev_priv *priv,
		     u16 pkey, int child_type);

int  __init ipoib_netlink_init(void);
void __exit ipoib_netlink_fini(void);

void ipoib_set_umcast(struct net_device *ndev, int umcast_val);
int  ipoib_set_mode(struct net_device *dev, const char *buf);

void ipoib_setup_common(struct net_device *dev);

void ipoib_pkey_open(struct ipoib_dev_priv *priv);
void ipoib_drain_cq(struct net_device *dev);

void ipoib_set_ethtool_ops(struct net_device *dev);

#define IPOIB_FLAGS_RC		0x80
#define IPOIB_FLAGS_UC		0x40

/* We don't support UC connections at the moment */
#define IPOIB_CM_SUPPORTED(ha)   (ha[0] & (IPOIB_FLAGS_RC))

#ifdef CONFIG_INFINIBAND_IPOIB_CM

extern int ipoib_max_conn_qp;

static inline int ipoib_cm_admin_enabled(struct net_device *dev)
{
	struct ipoib_dev_priv *priv = ipoib_priv(dev);
	return IPOIB_CM_SUPPORTED(dev->dev_addr) &&
		test_bit(IPOIB_FLAG_ADMIN_CM, &priv->flags);
}

static inline int ipoib_cm_enabled(struct net_device *dev, u8 *hwaddr)
{
	struct ipoib_dev_priv *priv = ipoib_priv(dev);
	return IPOIB_CM_SUPPORTED(hwaddr) &&
		test_bit(IPOIB_FLAG_ADMIN_CM, &priv->flags);
}

static inline int ipoib_cm_up(struct ipoib_neigh *neigh)

{
	return test_bit(IPOIB_FLAG_OPER_UP, &neigh->cm->flags);
}

static inline struct ipoib_cm_tx *ipoib_cm_get(struct ipoib_neigh *neigh)
{
	return neigh->cm;
}

static inline void ipoib_cm_set(struct ipoib_neigh *neigh, struct ipoib_cm_tx *tx)
{
	neigh->cm = tx;
}

static inline int ipoib_cm_has_srq(struct net_device *dev)
{
	struct ipoib_dev_priv *priv = ipoib_priv(dev);
	return !!priv->cm.srq;
}

static inline unsigned int ipoib_cm_max_mtu(struct net_device *dev)
{
	struct ipoib_dev_priv *priv = ipoib_priv(dev);
	return priv->cm.max_cm_mtu;
}

void ipoib_cm_send(struct net_device *dev, struct sk_buff *skb, struct ipoib_cm_tx *tx);
int ipoib_cm_dev_open(struct net_device *dev);
void ipoib_cm_dev_stop(struct net_device *dev);
int ipoib_cm_dev_init(struct net_device *dev);
int ipoib_cm_add_mode_attr(struct net_device *dev);
void ipoib_cm_dev_cleanup(struct net_device *dev);
struct ipoib_cm_tx *ipoib_cm_create_tx(struct net_device *dev, struct ipoib_path *path,
				    struct ipoib_neigh *neigh);
void ipoib_cm_destroy_tx(struct ipoib_cm_tx *tx);
void ipoib_cm_skb_too_long(struct net_device *dev, struct sk_buff *skb,
			   unsigned int mtu);
void ipoib_cm_handle_rx_wc(struct net_device *dev, struct ib_wc *wc);
void ipoib_cm_handle_tx_wc(struct net_device *dev, struct ib_wc *wc);
#else

struct ipoib_cm_tx;

#define ipoib_max_conn_qp 0

static inline int ipoib_cm_admin_enabled(struct net_device *dev)
{
	return 0;
}
static inline int ipoib_cm_enabled(struct net_device *dev, u8 *hwaddr)

{
	return 0;
}

static inline int ipoib_cm_up(struct ipoib_neigh *neigh)

{
	return 0;
}

static inline struct ipoib_cm_tx *ipoib_cm_get(struct ipoib_neigh *neigh)
{
	return NULL;
}

static inline void ipoib_cm_set(struct ipoib_neigh *neigh, struct ipoib_cm_tx *tx)
{
}

static inline int ipoib_cm_has_srq(struct net_device *dev)
{
	return 0;
}

static inline unsigned int ipoib_cm_max_mtu(struct net_device *dev)
{
	return 0;
}

static inline
void ipoib_cm_send(struct net_device *dev, struct sk_buff *skb, struct ipoib_cm_tx *tx)
{
	return;
}

static inline
int ipoib_cm_dev_open(struct net_device *dev)
{
	return 0;
}

static inline
void ipoib_cm_dev_stop(struct net_device *dev)
{
	return;
}

static inline
int ipoib_cm_dev_init(struct net_device *dev)
{
	return -EOPNOTSUPP;
}

static inline
void ipoib_cm_dev_cleanup(struct net_device *dev)
{
	return;
}

static inline
struct ipoib_cm_tx *ipoib_cm_create_tx(struct net_device *dev, struct ipoib_path *path,
				    struct ipoib_neigh *neigh)
{
	return NULL;
}

static inline
void ipoib_cm_destroy_tx(struct ipoib_cm_tx *tx)
{
	return;
}

static inline
int ipoib_cm_add_mode_attr(struct net_device *dev)
{
	return 0;
}

static inline void ipoib_cm_skb_too_long(struct net_device *dev, struct sk_buff *skb,
					 unsigned int mtu)
{
	dev_kfree_skb_any(skb);
}

static inline void ipoib_cm_handle_rx_wc(struct net_device *dev, struct ib_wc *wc)
{
}

static inline void ipoib_cm_handle_tx_wc(struct net_device *dev, struct ib_wc *wc)
{
}
#endif

#ifdef CONFIG_INFINIBAND_IPOIB_DEBUG
void ipoib_create_debug_files(struct net_device *dev);
void ipoib_delete_debug_files(struct net_device *dev);
int ipoib_register_debugfs(void);
void ipoib_unregister_debugfs(void);
#else
static inline void ipoib_create_debug_files(struct net_device *dev) { }
static inline void ipoib_delete_debug_files(struct net_device *dev) { }
static inline int ipoib_register_debugfs(void) { return 0; }
static inline void ipoib_unregister_debugfs(void) { }
#endif

#define ipoib_printk(level, priv, format, arg...)	\
	printk(level "%s: " format, ((struct ipoib_dev_priv *) priv)->dev->name , ## arg)
#define ipoib_warn(priv, format, arg...)		\
do {							\
	static DEFINE_RATELIMIT_STATE(_rs,		\
		10 * HZ /*10 seconds */,		\
		100);		\
	if (__ratelimit(&_rs))				\
		ipoib_printk(KERN_WARNING, priv, format , ## arg);\
} while (0)

extern int ipoib_sendq_size;
extern int ipoib_recvq_size;

extern struct ib_sa_client ipoib_sa_client;

#ifdef CONFIG_INFINIBAND_IPOIB_DEBUG
extern int ipoib_debug_level;

#define ipoib_dbg(priv, format, arg...)			\
	do {						\
		if (ipoib_debug_level > 0)			\
			ipoib_printk(KERN_DEBUG, priv, format , ## arg); \
	} while (0)
#define ipoib_dbg_mcast(priv, format, arg...)		\
	do {						\
		if (mcast_debug_level > 0)		\
			ipoib_printk(KERN_DEBUG, priv, format , ## arg); \
	} while (0)
#else /* CONFIG_INFINIBAND_IPOIB_DEBUG */
#define ipoib_dbg(priv, format, arg...)			\
	do { (void) (priv); } while (0)
#define ipoib_dbg_mcast(priv, format, arg...)		\
	do { (void) (priv); } while (0)
#endif /* CONFIG_INFINIBAND_IPOIB_DEBUG */

#ifdef CONFIG_INFINIBAND_IPOIB_DEBUG_DATA
#define ipoib_dbg_data(priv, format, arg...)		\
	do {						\
		if (data_debug_level > 0)		\
			ipoib_printk(KERN_DEBUG, priv, format , ## arg); \
	} while (0)
#else /* CONFIG_INFINIBAND_IPOIB_DEBUG_DATA */
#define ipoib_dbg_data(priv, format, arg...)		\
	do { (void) (priv); } while (0)
#endif /* CONFIG_INFINIBAND_IPOIB_DEBUG_DATA */

#define IPOIB_QPN(ha) (be32_to_cpup((__be32 *) ha) & 0xffffff)

extern const char ipoib_driver_version[];

#endif /* _IPOIB_H */