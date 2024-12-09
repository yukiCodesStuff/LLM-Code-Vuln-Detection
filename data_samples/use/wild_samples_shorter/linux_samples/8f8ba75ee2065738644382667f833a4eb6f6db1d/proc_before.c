		goto proc_dev_snmp6_fail;
	return 0;

proc_snmp6_fail:
	proc_net_remove(net, "sockstat6");
proc_dev_snmp6_fail:
	proc_net_remove(net, "dev_snmp6");
	return -ENOMEM;
}

static void __net_exit ipv6_proc_exit_net(struct net *net)