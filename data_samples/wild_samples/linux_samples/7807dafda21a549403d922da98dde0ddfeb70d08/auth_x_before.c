	if (need & CEPH_ENTITY_TYPE_AUTH) {
		struct ceph_x_authenticate *auth = (void *)(head + 1);
		void *enc_buf = xi->auth_authorizer.enc_buf;
		struct ceph_x_challenge_blob *blob = enc_buf +
							ceph_x_encrypt_offset();
		u64 *u;

		p = auth + 1;
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

		auth->struct_v = 2;  /* nautilus+ */
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

		/* nautilus+: request service tickets at the same time */
		need = ac->want_keys & ~CEPH_ENTITY_TYPE_AUTH;
		WARN_ON(!need);
		ceph_encode_32_safe(&p, end, need, e_range);
		return p - buf;
	}

	if (need) {
		dout(" get_principal_session_key\n");
		ret = ceph_x_build_authorizer(ac, th, &xi->auth_authorizer);
		if (ret)
			return ret;

		p = buf;
		ceph_encode_16_safe(&p, end, CEPHX_GET_PRINCIPAL_SESSION_KEY,
				    e_range);
		ceph_encode_copy_safe(&p, end,
			xi->auth_authorizer.buf->vec.iov_base,
			xi->auth_authorizer.buf->vec.iov_len, e_range);
		ceph_encode_8_safe(&p, end, 1, e_range);
		ceph_encode_32_safe(&p, end, need, e_range);
		return p - buf;
	}

	return 0;

e_range:
	return -ERANGE;
}

static int decode_con_secret(void **p, void *end, u8 *con_secret,
			     int *con_secret_len)
{
	int len;

	ceph_decode_32_safe(p, end, len, bad);
	ceph_decode_need(p, end, len, bad);

	dout("%s len %d\n", __func__, len);
	if (con_secret) {
		if (len > CEPH_MAX_CON_SECRET_LEN) {
			pr_err("connection secret too big %d\n", len);
			goto bad_memzero;
		}