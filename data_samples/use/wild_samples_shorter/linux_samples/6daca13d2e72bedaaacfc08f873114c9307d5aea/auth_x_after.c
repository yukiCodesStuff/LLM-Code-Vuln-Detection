 * authorizer.  The first part (ceph_x_authorize_a) should already be
 * encoded.
 */
static int encrypt_authorizer(struct ceph_x_authorizer *au,
			      u64 *server_challenge)
{
	struct ceph_x_authorize_a *msg_a;
	struct ceph_x_authorize_b *msg_b;
	void *p, *end;
	end = au->buf->vec.iov_base + au->buf->vec.iov_len;

	msg_b = p + ceph_x_encrypt_offset();
	msg_b->struct_v = 2;
	msg_b->nonce = cpu_to_le64(au->nonce);
	if (server_challenge) {
		msg_b->have_challenge = 1;
		msg_b->server_challenge_plus_one =
		    cpu_to_le64(*server_challenge + 1);
	} else {
		msg_b->have_challenge = 0;
		msg_b->server_challenge_plus_one = 0;
	}

	ret = ceph_x_encrypt(&au->session_key, p, end - p, sizeof(*msg_b));
	if (ret < 0)
		return ret;

	p += ret;
	if (server_challenge) {
		WARN_ON(p != end);
	} else {
		WARN_ON(p > end);
		au->buf->vec.iov_len = p - au->buf->vec.iov_base;
	}

	return 0;
}

	     le64_to_cpu(msg_a->ticket_blob.secret_id));

	get_random_bytes(&au->nonce, sizeof(au->nonce));
	ret = encrypt_authorizer(au, NULL);
	if (ret) {
		pr_err("failed to encrypt authorizer: %d", ret);
		goto out_au;
	}
	return 0;
}

static int decrypt_authorize_challenge(struct ceph_x_authorizer *au,
				       void *challenge_buf,
				       int challenge_buf_len,
				       u64 *server_challenge)
{
	struct ceph_x_authorize_challenge *ch =
	    challenge_buf + sizeof(struct ceph_x_encrypt_header);
	int ret;

	/* no leading len */
	ret = __ceph_x_decrypt(&au->session_key, challenge_buf,
			       challenge_buf_len);
	if (ret < 0)
		return ret;
	if (ret < sizeof(*ch)) {
		pr_err("bad size %d for ceph_x_authorize_challenge\n", ret);
		return -EINVAL;
	}

	*server_challenge = le64_to_cpu(ch->server_challenge);
	return 0;
}

static int ceph_x_add_authorizer_challenge(struct ceph_auth_client *ac,
					   struct ceph_authorizer *a,
					   void *challenge_buf,
					   int challenge_buf_len)
{
	struct ceph_x_authorizer *au = (void *)a;
	u64 server_challenge;
	int ret;

	ret = decrypt_authorize_challenge(au, challenge_buf, challenge_buf_len,
					  &server_challenge);
	if (ret) {
		pr_err("failed to decrypt authorize challenge: %d", ret);
		return ret;
	}

	ret = encrypt_authorizer(au, &server_challenge);
	if (ret) {
		pr_err("failed to encrypt authorizer w/ challenge: %d", ret);
		return ret;
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
	.add_authorizer_challenge = ceph_x_add_authorizer_challenge,
	.verify_authorizer_reply = ceph_x_verify_authorizer_reply,
	.invalidate_authorizer = ceph_x_invalidate_authorizer,
	.reset =  ceph_x_reset,
	.destroy = ceph_x_destroy,