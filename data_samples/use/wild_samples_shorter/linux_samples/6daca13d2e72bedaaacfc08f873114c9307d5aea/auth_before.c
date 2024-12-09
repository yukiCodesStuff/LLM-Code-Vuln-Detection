}
EXPORT_SYMBOL(ceph_auth_update_authorizer);

int ceph_auth_verify_authorizer_reply(struct ceph_auth_client *ac,
				      struct ceph_authorizer *a)
{
	int ret = 0;