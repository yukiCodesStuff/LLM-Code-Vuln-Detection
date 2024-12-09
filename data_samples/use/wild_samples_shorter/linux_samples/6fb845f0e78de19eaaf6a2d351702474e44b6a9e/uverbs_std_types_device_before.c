static int UVERBS_HANDLER(UVERBS_METHOD_QUERY_PORT)(
	struct uverbs_attr_bundle *attrs)
{
	struct ib_device *ib_dev = attrs->ufile->device->ib_dev;
	struct ib_port_attr attr = {};
	struct ib_uverbs_query_port_resp_ex resp = {};
	int ret;
	u8 port_num;

	/* FIXME: Extend the UAPI_DEF_OBJ_NEEDS_FN stuff.. */
	if (!ib_dev->ops.query_port)
		return -EOPNOTSUPP;
