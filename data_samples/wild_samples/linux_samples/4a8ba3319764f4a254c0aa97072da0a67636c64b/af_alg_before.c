{
	struct af_alg_completion *completion = req->data;

	completion->err = err;
	complete(&completion->completion);
}
EXPORT_SYMBOL_GPL(af_alg_complete);

static int __init af_alg_init(void)
{
	int err = proto_register(&alg_proto, 0);

	if (err)
		goto out;

	err = sock_register(&alg_family);
	if (err != 0)
		goto out_unregister_proto;

out:
	return err;

out_unregister_proto:
	proto_unregister(&alg_proto);
	goto out;
}

static void __exit af_alg_exit(void)
{
	sock_unregister(PF_ALG);
	proto_unregister(&alg_proto);
}

module_init(af_alg_init);
module_exit(af_alg_exit);
MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(AF_ALG);