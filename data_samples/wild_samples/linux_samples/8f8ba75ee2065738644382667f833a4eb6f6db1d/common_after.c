	{
		struct sockaddr_atmpvc pvc;

		if (!vcc->dev || !test_bit(ATM_VF_ADDR, &vcc->flags))
			return -ENOTCONN;
		memset(&pvc, 0, sizeof(pvc));
		pvc.sap_family = AF_ATMPVC;
		pvc.sap_addr.itf = vcc->dev->number;
		pvc.sap_addr.vpi = vcc->vpi;
		pvc.sap_addr.vci = vcc->vci;
		return copy_to_user(optval, &pvc, sizeof(pvc)) ? -EFAULT : 0;
	}
	default:
		if (level == SOL_SOCKET)
			return -EINVAL;
		break;
	}
	if (!vcc->dev || !vcc->dev->ops->getsockopt)
		return -EINVAL;
	return vcc->dev->ops->getsockopt(vcc, level, optname, optval, len);
}

int register_atmdevice_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&atm_dev_notify_chain, nb);
}
EXPORT_SYMBOL_GPL(register_atmdevice_notifier);

void unregister_atmdevice_notifier(struct notifier_block *nb)
{
	atomic_notifier_chain_unregister(&atm_dev_notify_chain, nb);
}
EXPORT_SYMBOL_GPL(unregister_atmdevice_notifier);

static int __init atm_init(void)
{
	int error;

	error = proto_register(&vcc_proto, 0);
	if (error < 0)
		goto out;
	error = atmpvc_init();
	if (error < 0) {
		pr_err("atmpvc_init() failed with %d\n", error);
		goto out_unregister_vcc_proto;
	}
	error = atmsvc_init();
	if (error < 0) {
		pr_err("atmsvc_init() failed with %d\n", error);
		goto out_atmpvc_exit;
	}
	error = atm_proc_init();
	if (error < 0) {
		pr_err("atm_proc_init() failed with %d\n", error);
		goto out_atmsvc_exit;
	}
	error = atm_sysfs_init();
	if (error < 0) {
		pr_err("atm_sysfs_init() failed with %d\n", error);
		goto out_atmproc_exit;
	}
out:
	return error;
out_atmproc_exit:
	atm_proc_exit();
out_atmsvc_exit:
	atmsvc_exit();
out_atmpvc_exit:
	atmsvc_exit();
out_unregister_vcc_proto:
	proto_unregister(&vcc_proto);
	goto out;
}

static void __exit atm_exit(void)
{
	atm_proc_exit();
	atm_sysfs_exit();
	atmsvc_exit();
	atmpvc_exit();
	proto_unregister(&vcc_proto);
}

subsys_initcall(atm_init);

module_exit(atm_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_ATMPVC);
MODULE_ALIAS_NETPROTO(PF_ATMSVC);