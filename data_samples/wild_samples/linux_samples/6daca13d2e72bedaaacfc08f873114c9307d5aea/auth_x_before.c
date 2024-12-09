	while (num--) {
		ret = process_one_ticket(ac, secret, &p, end);
		if (ret)
			return ret;
	}

	return 0;

bad:
	return -EINVAL;
}

/*
 * Encode and encrypt the second part (ceph_x_authorize_b) of the
 * authorizer.  The first part (ceph_x_authorize_a) should already be
 * encoded.
 */
static int encrypt_authorizer(struct ceph_x_authorizer *au)
{
	struct ceph_x_authorize_a *msg_a;
	struct ceph_x_authorize_b *msg_b;
	void *p, *end;
	int ret;

	msg_a = au->buf->vec.iov_base;
	WARN_ON(msg_a->ticket_blob.secret_id != cpu_to_le64(au->secret_id));
	p = (void *)(msg_a + 1) + le32_to_cpu(msg_a->ticket_blob.blob_len);
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
{
	struct ceph_x_authorize_a *msg_a;
	struct ceph_x_authorize_b *msg_b;
	void *p, *end;
	int ret;

	msg_a = au->buf->vec.iov_base;
	WARN_ON(msg_a->ticket_blob.secret_id != cpu_to_le64(au->secret_id));
	p = (void *)(msg_a + 1) + le32_to_cpu(msg_a->ticket_blob.blob_len);
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

static void ceph_x_authorizer_cleanup(struct ceph_x_authorizer *au)
{
	ceph_crypto_key_destroy(&au->session_key);
	if (au->buf) {
		ceph_buffer_put(au->buf);
		au->buf = NULL;
	}
	if (ticket_blob_len) {
		memcpy(msg_a->ticket_blob.blob, th->ticket_blob->vec.iov_base,
		       th->ticket_blob->vec.iov_len);
	}
	dout(" th %p secret_id %lld %lld\n", th, th->secret_id,
	     le64_to_cpu(msg_a->ticket_blob.secret_id));

	get_random_bytes(&au->nonce, sizeof(au->nonce));
	ret = encrypt_authorizer(au);
	if (ret) {
		pr_err("failed to encrypt authorizer: %d", ret);
		goto out_au;
	}
	if (au->secret_id < th->secret_id) {
		dout("ceph_x_update_authorizer service %u secret %llu < %llu\n",
		     au->service, au->secret_id, th->secret_id);
		return ceph_x_build_authorizer(ac, th, au);
	}
	return 0;
}

static int ceph_x_verify_authorizer_reply(struct ceph_auth_client *ac,
					  struct ceph_authorizer *a)
{
	struct ceph_x_authorizer *au = (void *)a;
	void *p = au->enc_buf;
	struct ceph_x_authorize_reply *reply = p + ceph_x_encrypt_offset();
	int ret;

	ret = ceph_x_decrypt(&au->session_key, &p, p + CEPHX_AU_ENC_BUF_LEN);
	if (ret < 0)
		return ret;
	if (ret != sizeof(*reply))
		return -EPERM;

	if (au->nonce + 1 != le64_to_cpu(reply->nonce_plus_one))
		ret = -EPERM;
	else
		ret = 0;
	dout("verify_authorizer_reply nonce %llx got %llx ret %d\n",
	     au->nonce, le64_to_cpu(reply->nonce_plus_one), ret);
	return ret;
}

static void ceph_x_reset(struct ceph_auth_client *ac)
{
	struct ceph_x_info *xi = ac->private;

	dout("reset\n");
	xi->starting = true;
	xi->server_challenge = 0;
}

static void ceph_x_destroy(struct ceph_auth_client *ac)
{
	struct ceph_x_info *xi = ac->private;
	struct rb_node *p;

	dout("ceph_x_destroy %p\n", ac);
	ceph_crypto_key_destroy(&xi->secret);

	while ((p = rb_first(&xi->ticket_handlers)) != NULL) {
		struct ceph_x_ticket_handler *th =
			rb_entry(p, struct ceph_x_ticket_handler, node);
		remove_ticket_handler(ac, th);
	}
static const struct ceph_auth_client_ops ceph_x_ops = {
	.name = "x",
	.is_authenticated = ceph_x_is_authenticated,
	.should_authenticate = ceph_x_should_authenticate,
	.build_request = ceph_x_build_request,
	.handle_reply = ceph_x_handle_reply,
	.create_authorizer = ceph_x_create_authorizer,
	.update_authorizer = ceph_x_update_authorizer,
	.verify_authorizer_reply = ceph_x_verify_authorizer_reply,
	.invalidate_authorizer = ceph_x_invalidate_authorizer,
	.reset =  ceph_x_reset,
	.destroy = ceph_x_destroy,
	.sign_message = ceph_x_sign_message,
	.check_message_signature = ceph_x_check_message_signature,
};


int ceph_x_init(struct ceph_auth_client *ac)
{
	struct ceph_x_info *xi;
	int ret;

	dout("ceph_x_init %p\n", ac);
	ret = -ENOMEM;
	xi = kzalloc(sizeof(*xi), GFP_NOFS);
	if (!xi)
		goto out;

	ret = -EINVAL;
	if (!ac->key) {
		pr_err("no secret set (for auth_x protocol)\n");
		goto out_nomem;
	}

	ret = ceph_crypto_key_clone(&xi->secret, ac->key);
	if (ret < 0) {
		pr_err("cannot clone key: %d\n", ret);
		goto out_nomem;
	}

	xi->starting = true;
	xi->ticket_handlers = RB_ROOT;

	ac->protocol = CEPH_AUTH_CEPHX;
	ac->private = xi;
	ac->ops = &ceph_x_ops;
	return 0;

out_nomem:
	kfree(xi);
out:
	return ret;
}