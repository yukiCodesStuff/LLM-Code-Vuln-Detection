{
	int ret = 0;

	mutex_lock(&ac->mutex);
	if (ac->ops && ac->ops->update_authorizer)
		ret = ac->ops->update_authorizer(ac, peer_type, a);
	mutex_unlock(&ac->mutex);
	return ret;
}
EXPORT_SYMBOL(ceph_auth_update_authorizer);

int ceph_auth_add_authorizer_challenge(struct ceph_auth_client *ac,
				       struct ceph_authorizer *a,
				       void *challenge_buf,
				       int challenge_buf_len)
{
	int ret = 0;

	mutex_lock(&ac->mutex);
	if (ac->ops && ac->ops->add_authorizer_challenge)
		ret = ac->ops->add_authorizer_challenge(ac, a, challenge_buf,
							challenge_buf_len);
	mutex_unlock(&ac->mutex);
	return ret;
}