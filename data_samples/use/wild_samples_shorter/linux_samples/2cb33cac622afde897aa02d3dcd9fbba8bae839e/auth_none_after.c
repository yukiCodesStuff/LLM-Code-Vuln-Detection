	return xi->starting;
}

static int build_request(struct ceph_auth_client *ac, void *buf, void *end)
{
	return 0;
}

/*
 * the generic auth code decode the global_id, and we carry no actual
 * authenticate state, so nothing happens here.
 */
	.destroy = destroy,
	.is_authenticated = is_authenticated,
	.should_authenticate = should_authenticate,
	.build_request = build_request,
	.handle_reply = handle_reply,
	.create_authorizer = ceph_auth_none_create_authorizer,
	.destroy_authorizer = ceph_auth_none_destroy_authorizer,
};