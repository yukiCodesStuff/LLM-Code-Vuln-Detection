	} else {
		int ret = ceph_auth_update_authorizer(ac, CEPH_ENTITY_TYPE_MDS,
						      auth);
		if (ret)
			return ERR_PTR(ret);
	}
	*proto = ac->protocol;

	return auth;
}


static int verify_authorizer_reply(struct ceph_connection *con)
{
	struct ceph_mds_session *s = con->private;
	struct ceph_mds_client *mdsc = s->s_mdsc;
	struct ceph_auth_client *ac = mdsc->fsc->client->monc.auth;

	return ceph_auth_verify_authorizer_reply(ac, s->s_auth.authorizer);
}

static int invalidate_authorizer(struct ceph_connection *con)
{
	struct ceph_mds_session *s = con->private;
	struct ceph_mds_client *mdsc = s->s_mdsc;
	struct ceph_auth_client *ac = mdsc->fsc->client->monc.auth;

	ceph_auth_invalidate_authorizer(ac, CEPH_ENTITY_TYPE_MDS);

	return ceph_monc_validate_auth(&mdsc->fsc->client->monc);
}

static struct ceph_msg *mds_alloc_msg(struct ceph_connection *con,
				struct ceph_msg_header *hdr, int *skip)
{
	struct ceph_msg *msg;
	int type = (int) le16_to_cpu(hdr->type);
	int front_len = (int) le32_to_cpu(hdr->front_len);

	if (con->in_msg)
		return con->in_msg;

	*skip = 0;
	msg = ceph_msg_new(type, front_len, GFP_NOFS, false);
	if (!msg) {
		pr_err("unable to allocate msg type %d len %d\n",
		       type, front_len);
		return NULL;
	}
static const struct ceph_connection_operations mds_con_ops = {
	.get = con_get,
	.put = con_put,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.peer_reset = peer_reset,
	.alloc_msg = mds_alloc_msg,
	.sign_message = mds_sign_message,
	.check_message_signature = mds_check_message_signature,
};

/* eof */