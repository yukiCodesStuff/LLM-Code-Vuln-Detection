	if (con->auth) {
		/*
		 * Any connection that defines ->get_authorizer()
		 * should also define ->add_authorizer_challenge() and
		 * ->verify_authorizer_reply().
		 *
		 * See get_connect_authorizer().
		 */
		if (con->in_reply.tag == CEPH_MSGR_TAG_CHALLENGE_AUTHORIZER) {
			ret = con->ops->add_authorizer_challenge(
				    con, con->auth->authorizer_reply_buf,
				    le32_to_cpu(con->in_reply.authorizer_len));
			if (ret < 0)
				return ret;

			con_out_kvec_reset(con);
			__prepare_write_connect(con);
			prepare_read_connect(con);
			return 0;
		}

		ret = con->ops->verify_authorizer_reply(con);
		if (ret < 0) {
			con->error_msg = "bad authorize reply";
			return ret;