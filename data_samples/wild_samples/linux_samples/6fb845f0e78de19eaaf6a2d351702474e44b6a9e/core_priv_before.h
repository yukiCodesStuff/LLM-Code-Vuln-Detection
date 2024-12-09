{
	return 0;
}
#endif

struct ib_device *ib_device_get_by_index(u32 ifindex);
void ib_device_put(struct ib_device *device);
/* RDMA device netlink */
void nldev_init(void);
void nldev_exit(void);

static inline struct ib_qp *_ib_create_qp(struct ib_device *dev,
					  struct ib_pd *pd,
					  struct ib_qp_init_attr *attr,
					  struct ib_udata *udata,
					  struct ib_uobject *uobj)
{
	struct ib_qp *qp;

	if (!dev->ops.create_qp)
		return ERR_PTR(-EOPNOTSUPP);

	qp = dev->ops.create_qp(pd, attr, udata);
	if (IS_ERR(qp))
		return qp;

	qp->device = dev;
	qp->pd = pd;
	qp->uobject = uobj;
	/*
	 * We don't track XRC QPs for now, because they don't have PD
	 * and more importantly they are created internaly by driver,
	 * see mlx5 create_dev_resources() as an example.
	 */
	if (attr->qp_type < IB_QPT_XRC_INI) {
		qp->res.type = RDMA_RESTRACK_QP;
		if (uobj)
			rdma_restrack_uadd(&qp->res);
		else
			rdma_restrack_kadd(&qp->res);
	} else
		qp->res.valid = false;

	return qp;
}

struct rdma_dev_addr;
int rdma_resolve_ip_route(struct sockaddr *src_addr,
			  const struct sockaddr *dst_addr,
			  struct rdma_dev_addr *addr);

int rdma_addr_find_l2_eth_by_grh(const union ib_gid *sgid,
				 const union ib_gid *dgid,
				 u8 *dmac, const struct ib_gid_attr *sgid_attr,
				 int *hoplimit);
void rdma_copy_src_l2_addr(struct rdma_dev_addr *dev_addr,
			   const struct net_device *dev);

struct sa_path_rec;
int roce_resolve_route_from_path(struct sa_path_rec *rec,
				 const struct ib_gid_attr *attr);

struct net_device *rdma_read_gid_attr_ndev_rcu(const struct ib_gid_attr *attr);
#endif /* _CORE_PRIV_H */