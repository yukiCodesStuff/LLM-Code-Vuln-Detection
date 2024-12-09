{
	int ret = 0;

	mutex_lock(&ac->mutex);
	if (ac->ops && ac->ops->update_authorizer)
		ret = ac->ops->update_authorizer(ac, peer_type, a);
	mutex_unlock(&ac->mutex);
	return ret;
}
EXPORT_SYMBOL(ceph_auth_update_authorizer);

int ceph_auth_verify_authorizer_reply(struct ceph_auth_client *ac,
				      struct ceph_authorizer *a)
{
	int ret = 0;

	mutex_lock(&ac->mutex);
	if (ac->ops && ac->ops->verify_authorizer_reply)
		ret = ac->ops->verify_authorizer_reply(ac, a);
	mutex_unlock(&ac->mutex);
	return ret;
}