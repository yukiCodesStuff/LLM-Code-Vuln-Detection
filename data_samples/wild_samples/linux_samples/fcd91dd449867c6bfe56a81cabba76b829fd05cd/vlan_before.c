	for (p = *head; p; p = p->next) {
		struct vlan_hdr *vhdr2;

		if (!NAPI_GRO_CB(p)->same_flow)
			continue;

		vhdr2 = (struct vlan_hdr *)(p->data + off_vlan);
		if (compare_vlan_header(vhdr, vhdr2))
			NAPI_GRO_CB(p)->same_flow = 0;
	}

	skb_gro_pull(skb, sizeof(*vhdr));
	skb_gro_postpull_rcsum(skb, vhdr, sizeof(*vhdr));
	pp = ptype->callbacks.gro_receive(head, skb);

out_unlock:
	rcu_read_unlock();
out:
	NAPI_GRO_CB(skb)->flush |= flush;

	return pp;
}

static int vlan_gro_complete(struct sk_buff *skb, int nhoff)
{
	struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data + nhoff);
	__be16 type = vhdr->h_vlan_encapsulated_proto;
	struct packet_offload *ptype;
	int err = -ENOENT;

	rcu_read_lock();
	ptype = gro_find_complete_by_type(type);
	if (ptype)
		err = ptype->callbacks.gro_complete(skb, nhoff + sizeof(*vhdr));

	rcu_read_unlock();
	return err;
}

static struct packet_offload vlan_packet_offloads[] __read_mostly = {
	{
		.type = cpu_to_be16(ETH_P_8021Q),
		.priority = 10,
		.callbacks = {
			.gro_receive = vlan_gro_receive,
			.gro_complete = vlan_gro_complete,
		},
	},
	{
		.type = cpu_to_be16(ETH_P_8021AD),
		.priority = 10,
		.callbacks = {
			.gro_receive = vlan_gro_receive,
			.gro_complete = vlan_gro_complete,
		},
	},
};

static int __net_init vlan_init_net(struct net *net)
{
	struct vlan_net *vn = net_generic(net, vlan_net_id);
	int err;

	vn->name_type = VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD;

	err = vlan_proc_init(net);

	return err;
}

static void __net_exit vlan_exit_net(struct net *net)
{
	vlan_proc_cleanup(net);
}

static struct pernet_operations vlan_net_ops = {
	.init = vlan_init_net,
	.exit = vlan_exit_net,
	.id   = &vlan_net_id,
	.size = sizeof(struct vlan_net),
};

static int __init vlan_proto_init(void)
{
	int err;
	unsigned int i;

	pr_info("%s v%s\n", vlan_fullname, vlan_version);

	err = register_pernet_subsys(&vlan_net_ops);
	if (err < 0)
		goto err0;

	err = register_netdevice_notifier(&vlan_notifier_block);
	if (err < 0)
		goto err2;

	err = vlan_gvrp_init();
	if (err < 0)
		goto err3;

	err = vlan_mvrp_init();
	if (err < 0)
		goto err4;

	err = vlan_netlink_init();
	if (err < 0)
		goto err5;

	for (i = 0; i < ARRAY_SIZE(vlan_packet_offloads); i++)
		dev_add_offload(&vlan_packet_offloads[i]);

	vlan_ioctl_set(vlan_ioctl_handler);
	return 0;

err5:
	vlan_mvrp_uninit();
err4:
	vlan_gvrp_uninit();
err3:
	unregister_netdevice_notifier(&vlan_notifier_block);
err2:
	unregister_pernet_subsys(&vlan_net_ops);
err0:
	return err;
}

static void __exit vlan_cleanup_module(void)
{
	unsigned int i;

	vlan_ioctl_set(NULL);

	for (i = 0; i < ARRAY_SIZE(vlan_packet_offloads); i++)
		dev_remove_offload(&vlan_packet_offloads[i]);

	vlan_netlink_fini();

	unregister_netdevice_notifier(&vlan_notifier_block);

	unregister_pernet_subsys(&vlan_net_ops);
	rcu_barrier(); /* Wait for completion of call_rcu()'s */

	vlan_mvrp_uninit();
	vlan_gvrp_uninit();
}

module_init(vlan_proto_init);
module_exit(vlan_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);