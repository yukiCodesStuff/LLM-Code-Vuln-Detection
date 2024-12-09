	} else {
		int ret = ceph_auth_update_authorizer(ac, CEPH_ENTITY_TYPE_OSD,
						     auth);
		if (ret)
			return ERR_PTR(ret);
	}
	*proto = ac->protocol;

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
	struct ceph_osd_client *osdc = o->o_osdc;
	struct ceph_auth_client *ac = osdc->client->monc.auth;

	return ceph_auth_verify_authorizer_reply(ac, o->o_auth.authorizer);
}

static int invalidate_authorizer(struct ceph_connection *con)
{
	struct ceph_osd *o = con->private;
	struct ceph_osd_client *osdc = o->o_osdc;
	struct ceph_auth_client *ac = osdc->client->monc.auth;

	ceph_auth_invalidate_authorizer(ac, CEPH_ENTITY_TYPE_OSD);
	return ceph_monc_validate_auth(&osdc->client->monc);
}

static void osd_reencode_message(struct ceph_msg *msg)
{
	int type = le16_to_cpu(msg->hdr.type);

	if (type == CEPH_MSG_OSD_OP)
		encode_request_finish(msg);
}

static int osd_sign_message(struct ceph_msg *msg)
{
	struct ceph_osd *o = msg->con->private;
	struct ceph_auth_handshake *auth = &o->o_auth;

	return ceph_auth_sign_message(auth, msg);
}

static int osd_check_message_signature(struct ceph_msg *msg)
{
	struct ceph_osd *o = msg->con->private;
	struct ceph_auth_handshake *auth = &o->o_auth;

	return ceph_auth_check_message_signature(auth, msg);
}

static const struct ceph_connection_operations osd_con_ops = {
	.get = get_osd_con,
	.put = put_osd_con,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.add_authorizer_challenge = add_authorizer_challenge,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.alloc_msg = alloc_msg,
	.reencode_message = osd_reencode_message,
	.sign_message = osd_sign_message,
	.check_message_signature = osd_check_message_signature,
	.fault = osd_fault,
};
static const struct ceph_connection_operations osd_con_ops = {
	.get = get_osd_con,
	.put = put_osd_con,
	.dispatch = dispatch,
	.get_authorizer = get_authorizer,
	.add_authorizer_challenge = add_authorizer_challenge,
	.verify_authorizer_reply = verify_authorizer_reply,
	.invalidate_authorizer = invalidate_authorizer,
	.alloc_msg = alloc_msg,
	.reencode_message = osd_reencode_message,
	.sign_message = osd_sign_message,
	.check_message_signature = osd_check_message_signature,
	.fault = osd_fault,
};