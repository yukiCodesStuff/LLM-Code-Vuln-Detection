		return -EOPNOTSUPP;

	if ((ops->flags & GENL_ADMIN_PERM) &&
	    !capable(CAP_NET_ADMIN))
		return -EPERM;

	if ((nlh->nlmsg_flags & NLM_F_DUMP) == NLM_F_DUMP) {
		int rc;