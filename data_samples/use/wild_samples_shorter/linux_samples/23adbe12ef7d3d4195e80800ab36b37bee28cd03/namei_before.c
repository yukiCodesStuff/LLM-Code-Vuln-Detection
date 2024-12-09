
	if (S_ISDIR(inode->i_mode)) {
		/* DACs are overridable for directories */
		if (inode_capable(inode, CAP_DAC_OVERRIDE))
			return 0;
		if (!(mask & MAY_WRITE))
			if (inode_capable(inode, CAP_DAC_READ_SEARCH))
				return 0;
		return -EACCES;
	}
	/*
	 * at least one exec bit set.
	 */
	if (!(mask & MAY_EXEC) || (inode->i_mode & S_IXUGO))
		if (inode_capable(inode, CAP_DAC_OVERRIDE))
			return 0;

	/*
	 * Searching includes executable on directories, else just read.
	 */
	mask &= MAY_READ | MAY_WRITE | MAY_EXEC;
	if (mask == MAY_READ)
		if (inode_capable(inode, CAP_DAC_READ_SEARCH))
			return 0;

	return -EACCES;
}
		return 0;
	if (uid_eq(dir->i_uid, fsuid))
		return 0;
	return !inode_capable(inode, CAP_FOWNER);
}

/*
 *	Check whether we can remove a link victim from directory dir, check