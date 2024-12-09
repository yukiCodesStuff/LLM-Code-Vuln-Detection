struct mlx5e_encap_entry {
	/* neigh hash entry list of encaps sharing the same neigh */
	struct list_head encap_list;
	struct mlx5e_neigh m_neigh;
	/* a node of the eswitch encap hash table which keeping all the encap
	 * entries
	 */
	struct hlist_node encap_hlist;
	struct list_head flows;
	u32 encap_id;
	struct ip_tunnel_info tun_info;
	unsigned char h_dest[ETH_ALEN];	/* destination eth addr	*/

	struct net_device *out_dev;
	struct net_device *route_dev;
	int tunnel_type;
	int tunnel_hlen;
	int reformat_type;
	u8 flags;
	char *encap_header;
	int encap_size;
};

struct mlx5e_rep_sq {
	struct mlx5_flow_handle	*send_to_vport_rule;
	struct list_head	 list;
};

void *mlx5e_alloc_nic_rep_priv(struct mlx5_core_dev *mdev);
void mlx5e_rep_register_vport_reps(struct mlx5_core_dev *mdev);
void mlx5e_rep_unregister_vport_reps(struct mlx5_core_dev *mdev);
bool mlx5e_is_uplink_rep(struct mlx5e_priv *priv);
int mlx5e_add_sqs_fwd_rules(struct mlx5e_priv *priv);
void mlx5e_remove_sqs_fwd_rules(struct mlx5e_priv *priv);

void mlx5e_handle_rx_cqe_rep(struct mlx5e_rq *rq, struct mlx5_cqe64 *cqe);

int mlx5e_rep_encap_entry_attach(struct mlx5e_priv *priv,
				 struct mlx5e_encap_entry *e);
void mlx5e_rep_encap_entry_detach(struct mlx5e_priv *priv,
				  struct mlx5e_encap_entry *e);

void mlx5e_rep_queue_neigh_stats_work(struct mlx5e_priv *priv);

bool mlx5e_eswitch_rep(struct net_device *netdev);

#else /* CONFIG_MLX5_ESWITCH */
static inline bool mlx5e_is_uplink_rep(struct mlx5e_priv *priv) { return false; }