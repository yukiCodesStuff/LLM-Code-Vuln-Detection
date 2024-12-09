	if (error)
		goto out;

	nd_jump_link(&path);
	return NULL;
out:
	return ERR_PTR(error);
}
