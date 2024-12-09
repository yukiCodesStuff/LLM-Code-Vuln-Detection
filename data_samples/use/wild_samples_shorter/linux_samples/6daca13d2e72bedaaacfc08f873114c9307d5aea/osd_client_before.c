	return auth;
}


static int verify_authorizer_reply(struct ceph_connection *con)
{
	struct ceph_osd *o = con->private;
	.put = put_osd_con,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.alloc_msg = alloc_msg,
	.reencode_message = osd_reencode_message,