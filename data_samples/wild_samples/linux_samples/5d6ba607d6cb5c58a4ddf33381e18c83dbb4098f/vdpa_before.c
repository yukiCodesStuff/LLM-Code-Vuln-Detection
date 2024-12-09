static const struct nla_policy vdpa_nl_policy[VDPA_ATTR_MAX + 1] = {
	[VDPA_ATTR_MGMTDEV_BUS_NAME] = { .type = NLA_NUL_STRING },
	[VDPA_ATTR_MGMTDEV_DEV_NAME] = { .type = NLA_STRING },
	[VDPA_ATTR_DEV_NAME] = { .type = NLA_STRING },
	[VDPA_ATTR_DEV_NET_CFG_MACADDR] = NLA_POLICY_ETH_ADDR,
	/* virtio spec 1.1 section 5.1.4.1 for valid MTU range */
	[VDPA_ATTR_DEV_NET_CFG_MTU] = NLA_POLICY_MIN(NLA_U16, 68),
	[VDPA_ATTR_DEV_QUEUE_INDEX] = { .type = NLA_U32 },
	[VDPA_ATTR_DEV_FEATURES] = { .type = NLA_U64 },
};

static const struct genl_ops vdpa_nl_ops[] = {
	{
		.cmd = VDPA_CMD_MGMTDEV_GET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = vdpa_nl_cmd_mgmtdev_get_doit,
		.dumpit = vdpa_nl_cmd_mgmtdev_get_dumpit,
	},
	{
		.cmd = VDPA_CMD_DEV_NEW,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = vdpa_nl_cmd_dev_add_set_doit,
		.flags = GENL_ADMIN_PERM,
	},
	{
		.cmd = VDPA_CMD_DEV_DEL,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = vdpa_nl_cmd_dev_del_set_doit,
		.flags = GENL_ADMIN_PERM,
	},
	{
		.cmd = VDPA_CMD_DEV_GET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = vdpa_nl_cmd_dev_get_doit,
		.dumpit = vdpa_nl_cmd_dev_get_dumpit,
	},
	{
		.cmd = VDPA_CMD_DEV_CONFIG_GET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = vdpa_nl_cmd_dev_config_get_doit,
		.dumpit = vdpa_nl_cmd_dev_config_get_dumpit,
	},
	{
		.cmd = VDPA_CMD_DEV_VSTATS_GET,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = vdpa_nl_cmd_dev_stats_get_doit,
		.flags = GENL_ADMIN_PERM,
	},
};

static struct genl_family vdpa_nl_family __ro_after_init = {
	.name = VDPA_GENL_NAME,
	.version = VDPA_GENL_VERSION,
	.maxattr = VDPA_ATTR_MAX,
	.policy = vdpa_nl_policy,
	.netnsok = false,
	.module = THIS_MODULE,
	.ops = vdpa_nl_ops,
	.n_ops = ARRAY_SIZE(vdpa_nl_ops),
	.resv_start_op = VDPA_CMD_DEV_VSTATS_GET + 1,
};

static int vdpa_init(void)
{
	int err;

	err = bus_register(&vdpa_bus);
	if (err)
		return err;
	err = genl_register_family(&vdpa_nl_family);
	if (err)
		goto err;
	return 0;

err:
	bus_unregister(&vdpa_bus);
	return err;
}

static void __exit vdpa_exit(void)
{
	genl_unregister_family(&vdpa_nl_family);
	bus_unregister(&vdpa_bus);
	ida_destroy(&vdpa_index_ida);
}
core_initcall(vdpa_init);
module_exit(vdpa_exit);

MODULE_AUTHOR("Jason Wang <jasowang@redhat.com>");
MODULE_LICENSE("GPL v2");