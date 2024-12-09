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
static int encrypt_authorizer(struct ceph_x_authorizer *au,
			      u64 *server_challenge)
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

static void ceph_x_authorizer_cleanup(struct ceph_x_authorizer *au)
{
	ceph_crypto_key_destroy(&au->session_key);
	if (au->buf) {
		ceph_buffer_put(au->buf);
		au->buf = NULL;
	}
}

static int ceph_x_build_authorizer(struct ceph_auth_client *ac,
				   struct ceph_x_ticket_handler *th,
				   struct ceph_x_authorizer *au)
{
	int maxlen;
	struct ceph_x_authorize_a *msg_a;
	struct ceph_x_authorize_b *msg_b;
	int ret;
	int ticket_blob_len =
		(th->ticket_blob ? th->ticket_blob->vec.iov_len : 0);

	dout("build_authorizer for %s %p\n",
	     ceph_entity_type_name(th->service), au);

	ceph_crypto_key_destroy(&au->session_key);
	ret = ceph_crypto_key_clone(&au->session_key, &th->session_key);
	if (ret)
		goto out_au;

	maxlen = sizeof(*msg_a) + ticket_blob_len +
		ceph_x_encrypt_buflen(sizeof(*msg_b));
	dout("  need len %d\n", maxlen);
	if (au->buf && au->buf->alloc_len < maxlen) {
		ceph_buffer_put(au->buf);
		au->buf = NULL;
	}
	if (!au->buf) {
		au->buf = ceph_buffer_new(maxlen, GFP_NOFS);
		if (!au->buf) {
			ret = -ENOMEM;
			goto out_au;
		}
	}
	au->service = th->service;
	au->secret_id = th->secret_id;

	msg_a = au->buf->vec.iov_base;
	msg_a->struct_v = 1;
	msg_a->global_id = cpu_to_le64(ac->global_id);
	msg_a->service_id = cpu_to_le32(th->service);
	msg_a->ticket_blob.struct_v = 1;
	msg_a->ticket_blob.secret_id = cpu_to_le64(th->secret_id);
	msg_a->ticket_blob.blob_len = cpu_to_le32(ticket_blob_len);
	if (ticket_blob_len) {
		memcpy(msg_a->ticket_blob.blob, th->ticket_blob->vec.iov_base,
		       th->ticket_blob->vec.iov_len);
	}
	dout(" th %p secret_id %lld %lld\n", th, th->secret_id,
	     le64_to_cpu(msg_a->ticket_blob.secret_id));

	get_random_bytes(&au->nonce, sizeof(au->nonce));
	ret = encrypt_authorizer(au, NULL);
	if (ret) {
		pr_err("failed to encrypt authorizer: %d", ret);
		goto out_au;
	}

	dout(" built authorizer nonce %llx len %d\n", au->nonce,
	     (int)au->buf->vec.iov_len);
	return 0;

out_au:
	ceph_x_authorizer_cleanup(au);
	return ret;
}

static int ceph_x_encode_ticket(struct ceph_x_ticket_handler *th,
				void **p, void *end)
{
	ceph_decode_need(p, end, 1 + sizeof(u64), bad);
	ceph_encode_8(p, 1);
	ceph_encode_64(p, th->secret_id);
	if (th->ticket_blob) {
		const char *buf = th->ticket_blob->vec.iov_base;
		u32 len = th->ticket_blob->vec.iov_len;

		ceph_encode_32_safe(p, end, len, bad);
		ceph_encode_copy_safe(p, end, buf, len, bad);
	} else {
		ceph_encode_32_safe(p, end, 0, bad);
	}

	return 0;
bad:
	return -ERANGE;
}

static bool need_key(struct ceph_x_ticket_handler *th)
{
	if (!th->have_key)
		return true;

	return ktime_get_real_seconds() >= th->renew_after;
}

static bool have_key(struct ceph_x_ticket_handler *th)
{
	if (th->have_key) {
		if (ktime_get_real_seconds() >= th->expires)
			th->have_key = false;
	}

	return th->have_key;
}

static void ceph_x_validate_tickets(struct ceph_auth_client *ac, int *pneed)
{
	int want = ac->want_keys;
	struct ceph_x_info *xi = ac->private;
	int service;

	*pneed = ac->want_keys & ~(xi->have_keys);

	for (service = 1; service <= want; service <<= 1) {
		struct ceph_x_ticket_handler *th;

		if (!(ac->want_keys & service))
			continue;

		if (*pneed & service)
			continue;

		th = get_ticket_handler(ac, service);
		if (IS_ERR(th)) {
			*pneed |= service;
			continue;
		}

		if (need_key(th))
			*pneed |= service;
		if (!have_key(th))
			xi->have_keys &= ~service;
	}
}

static int ceph_x_build_request(struct ceph_auth_client *ac,
				void *buf, void *end)
{
	struct ceph_x_info *xi = ac->private;
	int need;
	struct ceph_x_request_header *head = buf;
	int ret;
	struct ceph_x_ticket_handler *th =
		get_ticket_handler(ac, CEPH_ENTITY_TYPE_AUTH);

	if (IS_ERR(th))
		return PTR_ERR(th);

	ceph_x_validate_tickets(ac, &need);

	dout("build_request want %x have %x need %x\n",
	     ac->want_keys, xi->have_keys, need);

	if (need & CEPH_ENTITY_TYPE_AUTH) {
		struct ceph_x_authenticate *auth = (void *)(head + 1);
		void *p = auth + 1;
		void *enc_buf = xi->auth_authorizer.enc_buf;
		struct ceph_x_challenge_blob *blob = enc_buf +
							ceph_x_encrypt_offset();
		u64 *u;

		if (p > end)
			return -ERANGE;

		dout(" get_auth_session_key\n");
		head->op = cpu_to_le16(CEPHX_GET_AUTH_SESSION_KEY);

		/* encrypt and hash */
		get_random_bytes(&auth->client_challenge, sizeof(u64));
		blob->client_challenge = auth->client_challenge;
		blob->server_challenge = cpu_to_le64(xi->server_challenge);
		ret = ceph_x_encrypt(&xi->secret, enc_buf, CEPHX_AU_ENC_BUF_LEN,
				     sizeof(*blob));
		if (ret < 0)
			return ret;

		auth->struct_v = 1;
		auth->key = 0;
		for (u = (u64 *)enc_buf; u + 1 <= (u64 *)(enc_buf + ret); u++)
			auth->key ^= *(__le64 *)u;
		dout(" server_challenge %llx client_challenge %llx key %llx\n",
		     xi->server_challenge, le64_to_cpu(auth->client_challenge),
		     le64_to_cpu(auth->key));

		/* now encode the old ticket if exists */
		ret = ceph_x_encode_ticket(th, &p, end);
		if (ret < 0)
			return ret;

		return p - buf;
	}

	if (need) {
		void *p = head + 1;
		struct ceph_x_service_ticket_request *req;

		if (p > end)
			return -ERANGE;
		head->op = cpu_to_le16(CEPHX_GET_PRINCIPAL_SESSION_KEY);

		ret = ceph_x_build_authorizer(ac, th, &xi->auth_authorizer);
		if (ret)
			return ret;
		ceph_encode_copy(&p, xi->auth_authorizer.buf->vec.iov_base,
				 xi->auth_authorizer.buf->vec.iov_len);

		req = p;
		req->keys = cpu_to_le32(need);
		p += sizeof(*req);
		return p - buf;
	}

	return 0;
}

static int ceph_x_handle_reply(struct ceph_auth_client *ac, int result,
			       void *buf, void *end)
{
	struct ceph_x_info *xi = ac->private;
	struct ceph_x_reply_header *head = buf;
	struct ceph_x_ticket_handler *th;
	int len = end - buf;
	int op;
	int ret;

	if (result)
		return result;  /* XXX hmm? */

	if (xi->starting) {
		/* it's a hello */
		struct ceph_x_server_challenge *sc = buf;

		if (len != sizeof(*sc))
			return -EINVAL;
		xi->server_challenge = le64_to_cpu(sc->server_challenge);
		dout("handle_reply got server challenge %llx\n",
		     xi->server_challenge);
		xi->starting = false;
		xi->have_keys &= ~CEPH_ENTITY_TYPE_AUTH;
		return -EAGAIN;
	}

	op = le16_to_cpu(head->op);
	result = le32_to_cpu(head->result);
	dout("handle_reply op %d result %d\n", op, result);
	switch (op) {
	case CEPHX_GET_AUTH_SESSION_KEY:
		/* verify auth key */
		ret = ceph_x_proc_ticket_reply(ac, &xi->secret,
					       buf + sizeof(*head), end);
		break;

	case CEPHX_GET_PRINCIPAL_SESSION_KEY:
		th = get_ticket_handler(ac, CEPH_ENTITY_TYPE_AUTH);
		if (IS_ERR(th))
			return PTR_ERR(th);
		ret = ceph_x_proc_ticket_reply(ac, &th->session_key,
					       buf + sizeof(*head), end);
		break;

	default:
		return -EINVAL;
	}
	if (ret)
		return ret;
	if (ac->want_keys == xi->have_keys)
		return 0;
	return -EAGAIN;
}

static void ceph_x_destroy_authorizer(struct ceph_authorizer *a)
{
	struct ceph_x_authorizer *au = (void *)a;

	ceph_x_authorizer_cleanup(au);
	kfree(au);
}

static int ceph_x_create_authorizer(
	struct ceph_auth_client *ac, int peer_type,
	struct ceph_auth_handshake *auth)
{
	struct ceph_x_authorizer *au;
	struct ceph_x_ticket_handler *th;
	int ret;

	th = get_ticket_handler(ac, peer_type);
	if (IS_ERR(th))
		return PTR_ERR(th);

	au = kzalloc(sizeof(*au), GFP_NOFS);
	if (!au)
		return -ENOMEM;

	au->base.destroy = ceph_x_destroy_authorizer;

	ret = ceph_x_build_authorizer(ac, th, au);
	if (ret) {
		kfree(au);
		return ret;
	}

	auth->authorizer = (struct ceph_authorizer *) au;
	auth->authorizer_buf = au->buf->vec.iov_base;
	auth->authorizer_buf_len = au->buf->vec.iov_len;
	auth->authorizer_reply_buf = au->enc_buf;
	auth->authorizer_reply_buf_len = CEPHX_AU_ENC_BUF_LEN;
	auth->sign_message = ac->ops->sign_message;
	auth->check_message_signature = ac->ops->check_message_signature;

	return 0;
}

static int ceph_x_update_authorizer(
	struct ceph_auth_client *ac, int peer_type,
	struct ceph_auth_handshake *auth)
{
	struct ceph_x_authorizer *au;
	struct ceph_x_ticket_handler *th;

	th = get_ticket_handler(ac, peer_type);
	if (IS_ERR(th))
		return PTR_ERR(th);

	au = (struct ceph_x_authorizer *)auth->authorizer;
	if (au->secret_id < th->secret_id) {
		dout("ceph_x_update_authorizer service %u secret %llu < %llu\n",
		     au->service, au->secret_id, th->secret_id);
		return ceph_x_build_authorizer(ac, th, au);
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

	ceph_x_authorizer_cleanup(&xi->auth_authorizer);

	kfree(ac->private);
	ac->private = NULL;
}

static void invalidate_ticket(struct ceph_auth_client *ac, int peer_type)
{
	struct ceph_x_ticket_handler *th;

	th = get_ticket_handler(ac, peer_type);
	if (!IS_ERR(th))
		th->have_key = false;
}

static void ceph_x_invalidate_authorizer(struct ceph_auth_client *ac,
					 int peer_type)
{
	/*
	 * We are to invalidate a service ticket in the hopes of
	 * getting a new, hopefully more valid, one.  But, we won't get
	 * it unless our AUTH ticket is good, so invalidate AUTH ticket
	 * as well, just in case.
	 */
	invalidate_ticket(ac, peer_type);
	invalidate_ticket(ac, CEPH_ENTITY_TYPE_AUTH);
}

static int calc_signature(struct ceph_x_authorizer *au, struct ceph_msg *msg,
			  __le64 *psig)
{
	void *enc_buf = au->enc_buf;
	struct {
		__le32 len;
		__le32 header_crc;
		__le32 front_crc;
		__le32 middle_crc;
		__le32 data_crc;
	} __packed *sigblock = enc_buf + ceph_x_encrypt_offset();
	int ret;

	sigblock->len = cpu_to_le32(4*sizeof(u32));
	sigblock->header_crc = msg->hdr.crc;
	sigblock->front_crc = msg->footer.front_crc;
	sigblock->middle_crc = msg->footer.middle_crc;
	sigblock->data_crc =  msg->footer.data_crc;
	ret = ceph_x_encrypt(&au->session_key, enc_buf, CEPHX_AU_ENC_BUF_LEN,
			     sizeof(*sigblock));
	if (ret < 0)
		return ret;

	*psig = *(__le64 *)(enc_buf + sizeof(u32));
	return 0;
}

static int ceph_x_sign_message(struct ceph_auth_handshake *auth,
			       struct ceph_msg *msg)
{
	__le64 sig;
	int ret;

	if (ceph_test_opt(from_msgr(msg->con->msgr), NOMSGSIGN))
		return 0;

	ret = calc_signature((struct ceph_x_authorizer *)auth->authorizer,
			     msg, &sig);
	if (ret)
		return ret;

	msg->footer.sig = sig;
	msg->footer.flags |= CEPH_MSG_FOOTER_SIGNED;
	return 0;
}

static int ceph_x_check_message_signature(struct ceph_auth_handshake *auth,
					  struct ceph_msg *msg)
{
	__le64 sig_check;
	int ret;

	if (ceph_test_opt(from_msgr(msg->con->msgr), NOMSGSIGN))
		return 0;

	ret = calc_signature((struct ceph_x_authorizer *)auth->authorizer,
			     msg, &sig_check);
	if (ret)
		return ret;
	if (sig_check == msg->footer.sig)
		return 0;
	if (msg->footer.flags & CEPH_MSG_FOOTER_SIGNED)
		dout("ceph_x_check_message_signature %p has signature %llx "
		     "expect %llx\n", msg, msg->footer.sig, sig_check);
	else
		dout("ceph_x_check_message_signature %p sender did not set "
		     "CEPH_MSG_FOOTER_SIGNED\n", msg);
	return -EBADMSG;
}

static const struct ceph_auth_client_ops ceph_x_ops = {
	.name = "x",
	.is_authenticated = ceph_x_is_authenticated,
	.should_authenticate = ceph_x_should_authenticate,
	.build_request = ceph_x_build_request,
	.handle_reply = ceph_x_handle_reply,
	.create_authorizer = ceph_x_create_authorizer,
	.update_authorizer = ceph_x_update_authorizer,
	.add_authorizer_challenge = ceph_x_add_authorizer_challenge,
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

static void ceph_x_authorizer_cleanup(struct ceph_x_authorizer *au)
{
	ceph_crypto_key_destroy(&au->session_key);
	if (au->buf) {
		ceph_buffer_put(au->buf);
		au->buf = NULL;
	}
}

static int ceph_x_build_authorizer(struct ceph_auth_client *ac,
				   struct ceph_x_ticket_handler *th,
				   struct ceph_x_authorizer *au)
{
	int maxlen;
	struct ceph_x_authorize_a *msg_a;
	struct ceph_x_authorize_b *msg_b;
	int ret;
	int ticket_blob_len =
		(th->ticket_blob ? th->ticket_blob->vec.iov_len : 0);

	dout("build_authorizer for %s %p\n",
	     ceph_entity_type_name(th->service), au);

	ceph_crypto_key_destroy(&au->session_key);
	ret = ceph_crypto_key_clone(&au->session_key, &th->session_key);
	if (ret)
		goto out_au;

	maxlen = sizeof(*msg_a) + ticket_blob_len +
		ceph_x_encrypt_buflen(sizeof(*msg_b));
	dout("  need len %d\n", maxlen);
	if (au->buf && au->buf->alloc_len < maxlen) {
		ceph_buffer_put(au->buf);
		au->buf = NULL;
	}
	if (!au->buf) {
		au->buf = ceph_buffer_new(maxlen, GFP_NOFS);
		if (!au->buf) {
			ret = -ENOMEM;
			goto out_au;
		}
	}
	au->service = th->service;
	au->secret_id = th->secret_id;

	msg_a = au->buf->vec.iov_base;
	msg_a->struct_v = 1;
	msg_a->global_id = cpu_to_le64(ac->global_id);
	msg_a->service_id = cpu_to_le32(th->service);
	msg_a->ticket_blob.struct_v = 1;
	msg_a->ticket_blob.secret_id = cpu_to_le64(th->secret_id);
	msg_a->ticket_blob.blob_len = cpu_to_le32(ticket_blob_len);
	if (ticket_blob_len) {
		memcpy(msg_a->ticket_blob.blob, th->ticket_blob->vec.iov_base,
		       th->ticket_blob->vec.iov_len);
	}
	dout(" th %p secret_id %lld %lld\n", th, th->secret_id,
	     le64_to_cpu(msg_a->ticket_blob.secret_id));

	get_random_bytes(&au->nonce, sizeof(au->nonce));
	ret = encrypt_authorizer(au, NULL);
	if (ret) {
		pr_err("failed to encrypt authorizer: %d", ret);
		goto out_au;
	}

	dout(" built authorizer nonce %llx len %d\n", au->nonce,
	     (int)au->buf->vec.iov_len);
	return 0;

out_au:
	ceph_x_authorizer_cleanup(au);
	return ret;
}

static int ceph_x_encode_ticket(struct ceph_x_ticket_handler *th,
				void **p, void *end)
{
	ceph_decode_need(p, end, 1 + sizeof(u64), bad);
	ceph_encode_8(p, 1);
	ceph_encode_64(p, th->secret_id);
	if (th->ticket_blob) {
		const char *buf = th->ticket_blob->vec.iov_base;
		u32 len = th->ticket_blob->vec.iov_len;

		ceph_encode_32_safe(p, end, len, bad);
		ceph_encode_copy_safe(p, end, buf, len, bad);
	} else {
		ceph_encode_32_safe(p, end, 0, bad);
	}

	return 0;
bad:
	return -ERANGE;
}

static bool need_key(struct ceph_x_ticket_handler *th)
{
	if (!th->have_key)
		return true;

	return ktime_get_real_seconds() >= th->renew_after;
}

static bool have_key(struct ceph_x_ticket_handler *th)
{
	if (th->have_key) {
		if (ktime_get_real_seconds() >= th->expires)
			th->have_key = false;
	}

	return th->have_key;
}

static void ceph_x_validate_tickets(struct ceph_auth_client *ac, int *pneed)
{
	int want = ac->want_keys;
	struct ceph_x_info *xi = ac->private;
	int service;

	*pneed = ac->want_keys & ~(xi->have_keys);

	for (service = 1; service <= want; service <<= 1) {
		struct ceph_x_ticket_handler *th;

		if (!(ac->want_keys & service))
			continue;

		if (*pneed & service)
			continue;

		th = get_ticket_handler(ac, service);
		if (IS_ERR(th)) {
			*pneed |= service;
			continue;
		}

		if (need_key(th))
			*pneed |= service;
		if (!have_key(th))
			xi->have_keys &= ~service;
	}
}

static int ceph_x_build_request(struct ceph_auth_client *ac,
				void *buf, void *end)
{
	struct ceph_x_info *xi = ac->private;
	int need;
	struct ceph_x_request_header *head = buf;
	int ret;
	struct ceph_x_ticket_handler *th =
		get_ticket_handler(ac, CEPH_ENTITY_TYPE_AUTH);

	if (IS_ERR(th))
		return PTR_ERR(th);

	ceph_x_validate_tickets(ac, &need);

	dout("build_request want %x have %x need %x\n",
	     ac->want_keys, xi->have_keys, need);

	if (need & CEPH_ENTITY_TYPE_AUTH) {
		struct ceph_x_authenticate *auth = (void *)(head + 1);
		void *p = auth + 1;
		void *enc_buf = xi->auth_authorizer.enc_buf;
		struct ceph_x_challenge_blob *blob = enc_buf +
							ceph_x_encrypt_offset();
		u64 *u;

		if (p > end)
			return -ERANGE;

		dout(" get_auth_session_key\n");
		head->op = cpu_to_le16(CEPHX_GET_AUTH_SESSION_KEY);

		/* encrypt and hash */
		get_random_bytes(&auth->client_challenge, sizeof(u64));
		blob->client_challenge = auth->client_challenge;
		blob->server_challenge = cpu_to_le64(xi->server_challenge);
		ret = ceph_x_encrypt(&xi->secret, enc_buf, CEPHX_AU_ENC_BUF_LEN,
				     sizeof(*blob));
		if (ret < 0)
			return ret;

		auth->struct_v = 1;
		auth->key = 0;
		for (u = (u64 *)enc_buf; u + 1 <= (u64 *)(enc_buf + ret); u++)
			auth->key ^= *(__le64 *)u;
		dout(" server_challenge %llx client_challenge %llx key %llx\n",
		     xi->server_challenge, le64_to_cpu(auth->client_challenge),
		     le64_to_cpu(auth->key));

		/* now encode the old ticket if exists */
		ret = ceph_x_encode_ticket(th, &p, end);
		if (ret < 0)
			return ret;

		return p - buf;
	}

	if (need) {
		void *p = head + 1;
		struct ceph_x_service_ticket_request *req;

		if (p > end)
			return -ERANGE;
		head->op = cpu_to_le16(CEPHX_GET_PRINCIPAL_SESSION_KEY);

		ret = ceph_x_build_authorizer(ac, th, &xi->auth_authorizer);
		if (ret)
			return ret;
		ceph_encode_copy(&p, xi->auth_authorizer.buf->vec.iov_base,
				 xi->auth_authorizer.buf->vec.iov_len);

		req = p;
		req->keys = cpu_to_le32(need);
		p += sizeof(*req);
		return p - buf;
	}

	return 0;
}

static int ceph_x_handle_reply(struct ceph_auth_client *ac, int result,
			       void *buf, void *end)
{
	struct ceph_x_info *xi = ac->private;
	struct ceph_x_reply_header *head = buf;
	struct ceph_x_ticket_handler *th;
	int len = end - buf;
	int op;
	int ret;

	if (result)
		return result;  /* XXX hmm? */

	if (xi->starting) {
		/* it's a hello */
		struct ceph_x_server_challenge *sc = buf;

		if (len != sizeof(*sc))
			return -EINVAL;
		xi->server_challenge = le64_to_cpu(sc->server_challenge);
		dout("handle_reply got server challenge %llx\n",
		     xi->server_challenge);
		xi->starting = false;
		xi->have_keys &= ~CEPH_ENTITY_TYPE_AUTH;
		return -EAGAIN;
	}

	op = le16_to_cpu(head->op);
	result = le32_to_cpu(head->result);
	dout("handle_reply op %d result %d\n", op, result);
	switch (op) {
	case CEPHX_GET_AUTH_SESSION_KEY:
		/* verify auth key */
		ret = ceph_x_proc_ticket_reply(ac, &xi->secret,
					       buf + sizeof(*head), end);
		break;

	case CEPHX_GET_PRINCIPAL_SESSION_KEY:
		th = get_ticket_handler(ac, CEPH_ENTITY_TYPE_AUTH);
		if (IS_ERR(th))
			return PTR_ERR(th);
		ret = ceph_x_proc_ticket_reply(ac, &th->session_key,
					       buf + sizeof(*head), end);
		break;

	default:
		return -EINVAL;
	}
	if (ret)
		return ret;
	if (ac->want_keys == xi->have_keys)
		return 0;
	return -EAGAIN;
}

static void ceph_x_destroy_authorizer(struct ceph_authorizer *a)
{
	struct ceph_x_authorizer *au = (void *)a;

	ceph_x_authorizer_cleanup(au);
	kfree(au);
}

static int ceph_x_create_authorizer(
	struct ceph_auth_client *ac, int peer_type,
	struct ceph_auth_handshake *auth)
{
	struct ceph_x_authorizer *au;
	struct ceph_x_ticket_handler *th;
	int ret;

	th = get_ticket_handler(ac, peer_type);
	if (IS_ERR(th))
		return PTR_ERR(th);

	au = kzalloc(sizeof(*au), GFP_NOFS);
	if (!au)
		return -ENOMEM;

	au->base.destroy = ceph_x_destroy_authorizer;

	ret = ceph_x_build_authorizer(ac, th, au);
	if (ret) {
		kfree(au);
		return ret;
	}

	auth->authorizer = (struct ceph_authorizer *) au;
	auth->authorizer_buf = au->buf->vec.iov_base;
	auth->authorizer_buf_len = au->buf->vec.iov_len;
	auth->authorizer_reply_buf = au->enc_buf;
	auth->authorizer_reply_buf_len = CEPHX_AU_ENC_BUF_LEN;
	auth->sign_message = ac->ops->sign_message;
	auth->check_message_signature = ac->ops->check_message_signature;

	return 0;
}

static int ceph_x_update_authorizer(
	struct ceph_auth_client *ac, int peer_type,
	struct ceph_auth_handshake *auth)
{
	struct ceph_x_authorizer *au;
	struct ceph_x_ticket_handler *th;

	th = get_ticket_handler(ac, peer_type);
	if (IS_ERR(th))
		return PTR_ERR(th);

	au = (struct ceph_x_authorizer *)auth->authorizer;
	if (au->secret_id < th->secret_id) {
		dout("ceph_x_update_authorizer service %u secret %llu < %llu\n",
		     au->service, au->secret_id, th->secret_id);
		return ceph_x_build_authorizer(ac, th, au);
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

	ceph_x_authorizer_cleanup(&xi->auth_authorizer);

	kfree(ac->private);
	ac->private = NULL;
}

static void invalidate_ticket(struct ceph_auth_client *ac, int peer_type)
{
	struct ceph_x_ticket_handler *th;

	th = get_ticket_handler(ac, peer_type);
	if (!IS_ERR(th))
		th->have_key = false;
}

static void ceph_x_invalidate_authorizer(struct ceph_auth_client *ac,
					 int peer_type)
{
	/*
	 * We are to invalidate a service ticket in the hopes of
	 * getting a new, hopefully more valid, one.  But, we won't get
	 * it unless our AUTH ticket is good, so invalidate AUTH ticket
	 * as well, just in case.
	 */
	invalidate_ticket(ac, peer_type);
	invalidate_ticket(ac, CEPH_ENTITY_TYPE_AUTH);
}

static int calc_signature(struct ceph_x_authorizer *au, struct ceph_msg *msg,
			  __le64 *psig)
{
	void *enc_buf = au->enc_buf;
	struct {
		__le32 len;
		__le32 header_crc;
		__le32 front_crc;
		__le32 middle_crc;
		__le32 data_crc;
	} __packed *sigblock = enc_buf + ceph_x_encrypt_offset();
	int ret;

	sigblock->len = cpu_to_le32(4*sizeof(u32));
	sigblock->header_crc = msg->hdr.crc;
	sigblock->front_crc = msg->footer.front_crc;
	sigblock->middle_crc = msg->footer.middle_crc;
	sigblock->data_crc =  msg->footer.data_crc;
	ret = ceph_x_encrypt(&au->session_key, enc_buf, CEPHX_AU_ENC_BUF_LEN,
			     sizeof(*sigblock));
	if (ret < 0)
		return ret;

	*psig = *(__le64 *)(enc_buf + sizeof(u32));
	return 0;
}

static int ceph_x_sign_message(struct ceph_auth_handshake *auth,
			       struct ceph_msg *msg)
{
	__le64 sig;
	int ret;

	if (ceph_test_opt(from_msgr(msg->con->msgr), NOMSGSIGN))
		return 0;

	ret = calc_signature((struct ceph_x_authorizer *)auth->authorizer,
			     msg, &sig);
	if (ret)
		return ret;

	msg->footer.sig = sig;
	msg->footer.flags |= CEPH_MSG_FOOTER_SIGNED;
	return 0;
}

static int ceph_x_check_message_signature(struct ceph_auth_handshake *auth,
					  struct ceph_msg *msg)
{
	__le64 sig_check;
	int ret;

	if (ceph_test_opt(from_msgr(msg->con->msgr), NOMSGSIGN))
		return 0;

	ret = calc_signature((struct ceph_x_authorizer *)auth->authorizer,
			     msg, &sig_check);
	if (ret)
		return ret;
	if (sig_check == msg->footer.sig)
		return 0;
	if (msg->footer.flags & CEPH_MSG_FOOTER_SIGNED)
		dout("ceph_x_check_message_signature %p has signature %llx "
		     "expect %llx\n", msg, msg->footer.sig, sig_check);
	else
		dout("ceph_x_check_message_signature %p sender did not set "
		     "CEPH_MSG_FOOTER_SIGNED\n", msg);
	return -EBADMSG;
}

static const struct ceph_auth_client_ops ceph_x_ops = {
	.name = "x",
	.is_authenticated = ceph_x_is_authenticated,
	.should_authenticate = ceph_x_should_authenticate,
	.build_request = ceph_x_build_request,
	.handle_reply = ceph_x_handle_reply,
	.create_authorizer = ceph_x_create_authorizer,
	.update_authorizer = ceph_x_update_authorizer,
	.add_authorizer_challenge = ceph_x_add_authorizer_challenge,
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
	if (ticket_blob_len) {
		memcpy(msg_a->ticket_blob.blob, th->ticket_blob->vec.iov_base,
		       th->ticket_blob->vec.iov_len);
	}
	dout(" th %p secret_id %lld %lld\n", th, th->secret_id,
	     le64_to_cpu(msg_a->ticket_blob.secret_id));

	get_random_bytes(&au->nonce, sizeof(au->nonce));
	ret = encrypt_authorizer(au, NULL);
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
static const struct ceph_auth_client_ops ceph_x_ops = {
	.name = "x",
	.is_authenticated = ceph_x_is_authenticated,
	.should_authenticate = ceph_x_should_authenticate,
	.build_request = ceph_x_build_request,
	.handle_reply = ceph_x_handle_reply,
	.create_authorizer = ceph_x_create_authorizer,
	.update_authorizer = ceph_x_update_authorizer,
	.add_authorizer_challenge = ceph_x_add_authorizer_challenge,
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