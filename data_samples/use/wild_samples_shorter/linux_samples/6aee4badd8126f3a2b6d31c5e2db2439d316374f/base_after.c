	if (error)
		goto out;

	error = nd_jump_link(&path);
out:
	return ERR_PTR(error);
}
