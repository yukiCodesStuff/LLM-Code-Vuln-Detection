	if (err < 0) {
		unregister_pernet_device(&sit_net_ops);
		printk(KERN_INFO "sit init: Can't add protocol\n");
	}
	return err;
}

module_init(sit_init);
module_exit(sit_cleanup);
MODULE_LICENSE("GPL");
MODULE_ALIAS("sit0");