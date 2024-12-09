	if (con->auth) {
		/*
		 * Any connection that defines ->get_authorizer()
		 * should also define ->verify_authorizer_reply().
		 * See get_connect_authorizer().
		 */
		ret = con->ops->verify_authorizer_reply(con);
		if (ret < 0) {
			con->error_msg = "bad authorize reply";
			return ret;