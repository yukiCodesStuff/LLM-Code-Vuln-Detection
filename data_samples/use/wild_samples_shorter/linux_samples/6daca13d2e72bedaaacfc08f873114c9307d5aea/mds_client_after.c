	return auth;
}

static int add_authorizer_challenge(struct ceph_connection *con,
				    void *challenge_buf, int challenge_buf_len)
{
	struct ceph_mds_session *s = con->private;
	struct ceph_mds_client *mdsc = s->s_mdsc;
	struct ceph_auth_client *ac = mdsc->fsc->client->monc.auth;

	return ceph_auth_add_authorizer_challenge(ac, s->s_auth.authorizer,
					    challenge_buf, challenge_buf_len);
}

static int verify_authorizer_reply(struct ceph_connection *con)
{
	struct ceph_mds_session *s = con->private;
	.put = con_put,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.add_authorizer_challenge = add_authorizer_challenge,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.peer_reset = peer_reset,
	.alloc_msg = mds_alloc_msg,