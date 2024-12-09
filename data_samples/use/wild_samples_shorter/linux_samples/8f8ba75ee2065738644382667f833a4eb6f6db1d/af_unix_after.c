	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm, false);
	if (err < 0)
		return err;

	err = -EOPNOTSUPP;
	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm, false);
	if (err < 0)
		return err;

	err = -EOPNOTSUPP;