	if (IS_ERR(sock_mnt)) {
		err = PTR_ERR(sock_mnt);
		goto out_mount;
	}

	/* The real protocol initialization is performed in later initcalls.
	 */

#ifdef CONFIG_NETFILTER
	err = netfilter_init();
	if (err)
		goto out;
#endif

	ptp_classifier_init();

out:
	return err;

out_mount:
	unregister_filesystem(&sock_fs_type);
out_fs:
	goto out;
}

core_initcall(sock_init);	/* early initcall */

static int __init jit_init(void)
{
#ifdef CONFIG_BPF_JIT_ALWAYS_ON
	bpf_jit_enable = 1;
#endif
	return 0;
}