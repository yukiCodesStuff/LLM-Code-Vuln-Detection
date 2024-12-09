	if (ptype == NULL) {
		flush = 1;
		goto out_unlock;
	}

	skb_gro_pull(skb, sizeof(*eh));
	skb_gro_postpull_rcsum(skb, eh, sizeof(*eh));
	pp = ptype->callbacks.gro_receive(head, skb);

out_unlock:
	rcu_read_unlock();
out:
	NAPI_GRO_CB(skb)->flush |= flush;

	return pp;
}
EXPORT_SYMBOL(eth_gro_receive);

int eth_gro_complete(struct sk_buff *skb, int nhoff)
{
	struct ethhdr *eh = (struct ethhdr *)(skb->data + nhoff);
	__be16 type = eh->h_proto;
	struct packet_offload *ptype;
	int err = -ENOSYS;

	if (skb->encapsulation)
		skb_set_inner_mac_header(skb, nhoff);

	rcu_read_lock();
	ptype = gro_find_complete_by_type(type);
	if (ptype != NULL)
		err = ptype->callbacks.gro_complete(skb, nhoff +
						    sizeof(struct ethhdr));

	rcu_read_unlock();
	return err;
}
EXPORT_SYMBOL(eth_gro_complete);

static struct packet_offload eth_packet_offload __read_mostly = {
	.type = cpu_to_be16(ETH_P_TEB),
	.priority = 10,
	.callbacks = {
		.gro_receive = eth_gro_receive,
		.gro_complete = eth_gro_complete,
	},
};

static int __init eth_offload_init(void)
{
	dev_add_offload(&eth_packet_offload);

	return 0;
}

fs_initcall(eth_offload_init);

unsigned char * __weak arch_get_platform_mac_address(void)
{
	return NULL;
}

int eth_platform_get_mac_address(struct device *dev, u8 *mac_addr)
{
	const unsigned char *addr;
	struct device_node *dp;

	if (dev_is_pci(dev))
		dp = pci_device_to_OF_node(to_pci_dev(dev));
	else
		dp = dev->of_node;

	addr = NULL;
	if (dp)
		addr = of_get_mac_address(dp);
	if (!addr)
		addr = arch_get_platform_mac_address();

	if (!addr)
		return -ENODEV;

	ether_addr_copy(mac_addr, addr);
	return 0;
}
EXPORT_SYMBOL(eth_platform_get_mac_address);