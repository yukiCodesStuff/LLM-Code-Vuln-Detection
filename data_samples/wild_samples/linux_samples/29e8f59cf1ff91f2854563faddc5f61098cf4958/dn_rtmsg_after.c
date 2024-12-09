{
	struct nlmsghdr *nlh = nlmsg_hdr(skb);

	if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
		return;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		RCV_SKB_FAIL(-EPERM);

	/* Eventually we might send routing messages too */

	RCV_SKB_FAIL(-EINVAL);
}

static struct nf_hook_ops dnrmg_ops __read_mostly = {
	.hook		= dnrmg_hook,
	.pf		= NFPROTO_DECNET,
	.hooknum	= NF_DN_ROUTE,
	.priority	= NF_DN_PRI_DNRTMSG,
};

static int __init dn_rtmsg_init(void)
{
	int rv = 0;
	struct netlink_kernel_cfg cfg = {
		.groups	= DNRNG_NLGRP_MAX,
		.input	= dnrmg_receive_user_skb,
	};

	dnrmg = netlink_kernel_create(&init_net, NETLINK_DNRTMSG, &cfg);
	if (dnrmg == NULL) {
		printk(KERN_ERR "dn_rtmsg: Cannot create netlink socket");
		return -ENOMEM;
	}

	rv = nf_register_hook(&dnrmg_ops);
	if (rv) {
		netlink_kernel_release(dnrmg);
	}

	return rv;
}

static void __exit dn_rtmsg_fini(void)
{
	nf_unregister_hook(&dnrmg_ops);
	netlink_kernel_release(dnrmg);
}


MODULE_DESCRIPTION("DECnet Routing Message Grabulator");
MODULE_AUTHOR("Steven Whitehouse <steve@chygwyn.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS_NET_PF_PROTO(PF_NETLINK, NETLINK_DNRTMSG);

module_init(dn_rtmsg_init);
module_exit(dn_rtmsg_fini);
