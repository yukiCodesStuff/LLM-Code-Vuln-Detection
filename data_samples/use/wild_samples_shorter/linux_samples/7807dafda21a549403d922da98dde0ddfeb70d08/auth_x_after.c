		if (ret < 0)
			return ret;

		auth->struct_v = 3;  /* nautilus+ */
		auth->key = 0;
		for (u = (u64 *)enc_buf; u + 1 <= (u64 *)(enc_buf + ret); u++)
			auth->key ^= *(__le64 *)u;
		dout(" server_challenge %llx client_challenge %llx key %llx\n",