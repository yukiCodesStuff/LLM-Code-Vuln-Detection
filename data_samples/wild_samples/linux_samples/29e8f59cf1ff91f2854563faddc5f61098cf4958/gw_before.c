{
	struct rtcanmsg *r;
	struct cgw_job *gwj;
	u8 limhops = 0;
	int err = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (nlmsg_len(nlh) < sizeof(*r))
		return -EINVAL;

	r = nlmsg_data(nlh);
	if (r->can_family != AF_CAN)
		return -EPFNOSUPPORT;

	/* so far we only support CAN -> CAN routings */
	if (r->gwtype != CGW_TYPE_CAN_CAN)
		return -EINVAL;

	gwj = kmem_cache_alloc(cgw_cache, GFP_KERNEL);
	if (!gwj)
		return -ENOMEM;

	gwj->handled_frames = 0;
	gwj->dropped_frames = 0;
	gwj->deleted_frames = 0;
	gwj->flags = r->flags;
	gwj->gwtype = r->gwtype;

	err = cgw_parse_attr(nlh, &gwj->mod, CGW_TYPE_CAN_CAN, &gwj->ccgw,
			     &limhops);
	if (err < 0)
		goto out;

	err = -ENODEV;

	/* ifindex == 0 is not allowed for job creation */
	if (!gwj->ccgw.src_idx || !gwj->ccgw.dst_idx)
		goto out;

	gwj->src.dev = __dev_get_by_index(&init_net, gwj->ccgw.src_idx);

	if (!gwj->src.dev)
		goto out;

	if (gwj->src.dev->type != ARPHRD_CAN)
		goto out;

	gwj->dst.dev = __dev_get_by_index(&init_net, gwj->ccgw.dst_idx);

	if (!gwj->dst.dev)
		goto out;

	if (gwj->dst.dev->type != ARPHRD_CAN)
		goto out;

	gwj->limit_hops = limhops;

	ASSERT_RTNL();

	err = cgw_register_filter(gwj);
	if (!err)
		hlist_add_head_rcu(&gwj->list, &cgw_list);
out:
	if (err)
		kmem_cache_free(cgw_cache, gwj);

	return err;
}

static void cgw_remove_all_jobs(void)
{
	struct cgw_job *gwj = NULL;
	struct hlist_node *nx;

	ASSERT_RTNL();

	hlist_for_each_entry_safe(gwj, nx, &cgw_list, list) {
		hlist_del(&gwj->list);
		cgw_unregister_filter(gwj);
		kmem_cache_free(cgw_cache, gwj);
	}
{
	struct cgw_job *gwj = NULL;
	struct hlist_node *nx;
	struct rtcanmsg *r;
	struct cf_mod mod;
	struct can_can_gw ccgw;
	u8 limhops = 0;
	int err = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (nlmsg_len(nlh) < sizeof(*r))
		return -EINVAL;

	r = nlmsg_data(nlh);
	if (r->can_family != AF_CAN)
		return -EPFNOSUPPORT;

	/* so far we only support CAN -> CAN routings */
	if (r->gwtype != CGW_TYPE_CAN_CAN)
		return -EINVAL;

	err = cgw_parse_attr(nlh, &mod, CGW_TYPE_CAN_CAN, &ccgw, &limhops);
	if (err < 0)
		return err;

	/* two interface indices both set to 0 => remove all entries */
	if (!ccgw.src_idx && !ccgw.dst_idx) {
		cgw_remove_all_jobs();
		return 0;
	}