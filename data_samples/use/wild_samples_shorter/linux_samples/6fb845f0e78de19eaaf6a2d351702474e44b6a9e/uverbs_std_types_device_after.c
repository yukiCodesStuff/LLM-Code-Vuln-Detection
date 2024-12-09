static int UVERBS_HANDLER(UVERBS_METHOD_QUERY_PORT)(
	struct uverbs_attr_bundle *attrs)
{
	struct ib_device *ib_dev;
	struct ib_port_attr attr = {};
	struct ib_uverbs_query_port_resp_ex resp = {};
	struct ib_ucontext *ucontext;
	int ret;
	u8 port_num;

	ucontext = ib_uverbs_get_ucontext(attrs);
	if (IS_ERR(ucontext))
		return PTR_ERR(ucontext);
	ib_dev = ucontext->device;

	/* FIXME: Extend the UAPI_DEF_OBJ_NEEDS_FN stuff.. */
	if (!ib_dev->ops.query_port)
		return -EOPNOTSUPP;
