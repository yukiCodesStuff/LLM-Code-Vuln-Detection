	return auth;
}

static int add_authorizer_challenge(struct ceph_connection *con,
				    void *challenge_buf, int challenge_buf_len)
{
	struct ceph_osd *o = con->private;
	struct ceph_osd_client *osdc = o->o_osdc;
	struct ceph_auth_client *ac = osdc->client->monc.auth;

	return ceph_auth_add_authorizer_challenge(ac, o->o_auth.authorizer,
					    challenge_buf, challenge_buf_len);
}

static int verify_authorizer_reply(struct ceph_connection *con)
{
	struct ceph_osd *o = con->private;
	.put = put_osd_con,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.add_authorizer_challenge = add_authorizer_challenge,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.alloc_msg = alloc_msg,
	.reencode_message = osd_reencode_message,