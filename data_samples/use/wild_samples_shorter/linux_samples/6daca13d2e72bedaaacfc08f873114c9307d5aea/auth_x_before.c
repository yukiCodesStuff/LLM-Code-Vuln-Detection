 * authorizer.  The first part (ceph_x_authorize_a) should already be
 * encoded.
 */
static int encrypt_authorizer(struct ceph_x_authorizer *au)
{
	struct ceph_x_authorize_a *msg_a;
	struct ceph_x_authorize_b *msg_b;
	void *p, *end;
	end = au->buf->vec.iov_base + au->buf->vec.iov_len;

	msg_b = p + ceph_x_encrypt_offset();
	msg_b->struct_v = 1;
	msg_b->nonce = cpu_to_le64(au->nonce);

	ret = ceph_x_encrypt(&au->session_key, p, end - p, sizeof(*msg_b));
	if (ret < 0)
		return ret;

	p += ret;
	WARN_ON(p > end);
	au->buf->vec.iov_len = p - au->buf->vec.iov_base;

	return 0;
}

	     le64_to_cpu(msg_a->ticket_blob.secret_id));

	get_random_bytes(&au->nonce, sizeof(au->nonce));
	ret = encrypt_authorizer(au);
	if (ret) {
		pr_err("failed to encrypt authorizer: %d", ret);
		goto out_au;
	}
	return 0;
}

static int ceph_x_verify_authorizer_reply(struct ceph_auth_client *ac,
					  struct ceph_authorizer *a)
{
	struct ceph_x_authorizer *au = (void *)a;
	.handle_reply = ceph_x_handle_reply,
	.create_authorizer = ceph_x_create_authorizer,
	.update_authorizer = ceph_x_update_authorizer,
	.verify_authorizer_reply = ceph_x_verify_authorizer_reply,
	.invalidate_authorizer = ceph_x_invalidate_authorizer,
	.reset =  ceph_x_reset,
	.destroy = ceph_x_destroy,