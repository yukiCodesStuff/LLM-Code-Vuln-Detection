{
	if (!proc_net_fops_create(net, "sockstat6", S_IRUGO,
			&sockstat6_seq_fops))
		return -ENOMEM;

	if (!proc_net_fops_create(net, "snmp6", S_IRUGO, &snmp6_seq_fops))
		goto proc_snmp6_fail;

	net->mib.proc_net_devsnmp6 = proc_mkdir("dev_snmp6", net->proc_net);
	if (!net->mib.proc_net_devsnmp6)
		goto proc_dev_snmp6_fail;
	return 0;

proc_dev_snmp6_fail:
	proc_net_remove(net, "snmp6");
proc_snmp6_fail:
	proc_net_remove(net, "sockstat6");
	return -ENOMEM;
}

static void __net_exit ipv6_proc_exit_net(struct net *net)
{
	proc_net_remove(net, "sockstat6");
	proc_net_remove(net, "dev_snmp6");
	proc_net_remove(net, "snmp6");
}

static struct pernet_operations ipv6_proc_ops = {
	.init = ipv6_proc_init_net,
	.exit = ipv6_proc_exit_net,
};

int __init ipv6_misc_proc_init(void)
{
	return register_pernet_subsys(&ipv6_proc_ops);
}