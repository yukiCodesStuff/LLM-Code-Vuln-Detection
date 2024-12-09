	return auth;
}


static int verify_authorizer_reply(struct ceph_connection *con)
{
	struct ceph_mds_session *s = con->private;
	.put = con_put,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.peer_reset = peer_reset,
	.alloc_msg = mds_alloc_msg,