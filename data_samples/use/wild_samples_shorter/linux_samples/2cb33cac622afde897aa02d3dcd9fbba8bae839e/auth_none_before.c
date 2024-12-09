	return xi->starting;
}

/*
 * the generic auth code decode the global_id, and we carry no actual
 * authenticate state, so nothing happens here.
 */
	.destroy = destroy,
	.is_authenticated = is_authenticated,
	.should_authenticate = should_authenticate,
	.handle_reply = handle_reply,
	.create_authorizer = ceph_auth_none_create_authorizer,
	.destroy_authorizer = ceph_auth_none_destroy_authorizer,
};