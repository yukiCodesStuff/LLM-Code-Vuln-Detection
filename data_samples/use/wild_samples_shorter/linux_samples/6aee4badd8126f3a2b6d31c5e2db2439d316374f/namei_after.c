	struct path	root;
	struct inode	*inode; /* path.dentry.d_inode */
	unsigned int	flags;
	unsigned	seq, m_seq, r_seq;
	int		last_type;
	unsigned	depth;
	int		total_link_count;
	struct saved {

static bool legitimize_root(struct nameidata *nd)
{
	/*
	 * For scoped-lookups (where nd->root has been zeroed), we need to
	 * restart the whole lookup from scratch -- because set_root() is wrong
	 * for these lookups (nd->dfd is the root, not the filesystem root).
	 */
	if (!nd->root.mnt && (nd->flags & LOOKUP_IS_SCOPED))
		return false;
	/* Nothing to do if nd->root is zero or is managed by the VFS user. */
	if (!nd->root.mnt || (nd->flags & LOOKUP_ROOT))
		return true;
	nd->flags |= LOOKUP_ROOT_GRABBED;
	return legitimize_path(nd, &nd->root, nd->root_seq);
	int status;

	if (nd->flags & LOOKUP_RCU) {
		/*
		 * We don't want to zero nd->root for scoped-lookups or
		 * externally-managed nd->root.
		 */
		if (!(nd->flags & (LOOKUP_ROOT | LOOKUP_IS_SCOPED)))
			nd->root.mnt = NULL;
		if (unlikely(unlazy_walk(nd)))
			return -ECHILD;
	}

	if (unlikely(nd->flags & LOOKUP_IS_SCOPED)) {
		/*
		 * While the guarantee of LOOKUP_IS_SCOPED is (roughly) "don't
		 * ever step outside the root during lookup" and should already
		 * be guaranteed by the rest of namei, we want to avoid a namei
		 * BUG resulting in userspace being given a path that was not
		 * scoped within the root at some point during the lookup.
		 *
		 * So, do a final sanity-check to make sure that in the
		 * worst-case scenario (a complete bypass of LOOKUP_IS_SCOPED)
		 * we won't silently return an fd completely outside of the
		 * requested root to userspace.
		 *
		 * Userspace could move the path outside the root after this
		 * check, but as discussed elsewhere this is not a concern (the
		 * resolved file was inside the root at some point).
		 */
		if (!path_is_under(&nd->path, &nd->root))
			return -EXDEV;
	}

	if (likely(!(nd->flags & LOOKUP_JUMPED)))
		return 0;

	if (likely(!(dentry->d_flags & DCACHE_OP_WEAK_REVALIDATE)))
	return status;
}

static int set_root(struct nameidata *nd)
{
	struct fs_struct *fs = current->fs;

	/*
	 * Jumping to the real root in a scoped-lookup is a BUG in namei, but we
	 * still have to ensure it doesn't happen because it will cause a breakout
	 * from the dirfd.
	 */
	if (WARN_ON(nd->flags & LOOKUP_IS_SCOPED))
		return -ENOTRECOVERABLE;

	if (nd->flags & LOOKUP_RCU) {
		unsigned seq;

		do {
		get_fs_root(fs, &nd->root);
		nd->flags |= LOOKUP_ROOT_GRABBED;
	}
	return 0;
}

static void path_put_conditional(struct path *path, struct nameidata *nd)
{

static int nd_jump_root(struct nameidata *nd)
{
	if (unlikely(nd->flags & LOOKUP_BENEATH))
		return -EXDEV;
	if (unlikely(nd->flags & LOOKUP_NO_XDEV)) {
		/* Absolute path arguments to path_init() are allowed. */
		if (nd->path.mnt != NULL && nd->path.mnt != nd->root.mnt)
			return -EXDEV;
	}
	if (!nd->root.mnt) {
		int error = set_root(nd);
		if (error)
			return error;
	}
	if (nd->flags & LOOKUP_RCU) {
		struct dentry *d;
		nd->path = nd->root;
		d = nd->path.dentry;
 * Helper to directly jump to a known parsed path from ->get_link,
 * caller must have taken a reference to path beforehand.
 */
int nd_jump_link(struct path *path)
{
	int error = -ELOOP;
	struct nameidata *nd = current->nameidata;

	if (unlikely(nd->flags & LOOKUP_NO_MAGICLINKS))
		goto err;

	error = -EXDEV;
	if (unlikely(nd->flags & LOOKUP_NO_XDEV)) {
		if (nd->path.mnt != path->mnt)
			goto err;
	}
	/* Not currently safe for scoped-lookups. */
	if (unlikely(nd->flags & LOOKUP_IS_SCOPED))
		goto err;

	path_put(&nd->path);
	nd->path = *path;
	nd->inode = nd->path.dentry->d_inode;
	nd->flags |= LOOKUP_JUMPED;
	return 0;

err:
	path_put(path);
	return error;
}

static inline void put_link(struct nameidata *nd)
{
	int error;
	const char *res;

	if (unlikely(nd->flags & LOOKUP_NO_SYMLINKS))
		return ERR_PTR(-ELOOP);

	if (!(nd->flags & LOOKUP_RCU)) {
		touch_atime(&last->link);
		cond_resched();
	} else if (atime_needs_update(&last->link, inode)) {
			return res;
	}
	if (*res == '/') {
		error = nd_jump_root(nd);
		if (unlikely(error))
			return ERR_PTR(error);
		while (unlikely(*++res == '/'))
			;
	}
	if (!*res)
		break;
	}

	if (need_mntput) {
		if (path->mnt == mnt)
			mntput(path->mnt);
		if (unlikely(nd->flags & LOOKUP_NO_XDEV))
			ret = -EXDEV;
		else
			nd->flags |= LOOKUP_JUMPED;
	}
	if (ret == -EISDIR || !ret)
		ret = 1;
	if (ret > 0 && unlikely(d_flags_negative(flags)))
		ret = -ENOENT;
		mounted = __lookup_mnt(path->mnt, path->dentry);
		if (!mounted)
			break;
		if (unlikely(nd->flags & LOOKUP_NO_XDEV))
			return false;
		path->mnt = &mounted->mnt;
		path->dentry = mounted->mnt.mnt_root;
		nd->flags |= LOOKUP_JUMPED;
		*seqp = read_seqcount_begin(&path->dentry->d_seq);
	struct inode *inode = nd->inode;

	while (1) {
		if (path_equal(&nd->path, &nd->root)) {
			if (unlikely(nd->flags & LOOKUP_BENEATH))
				return -ECHILD;
			break;
		}
		if (nd->path.dentry != nd->path.mnt->mnt_root) {
			struct dentry *old = nd->path.dentry;
			struct dentry *parent = old->d_parent;
			unsigned seq;
			nd->path.dentry = parent;
			nd->seq = seq;
			if (unlikely(!path_connected(&nd->path)))
				return -ECHILD;
			break;
		} else {
			struct mount *mnt = real_mount(nd->path.mnt);
			struct mount *mparent = mnt->mnt_parent;
				return -ECHILD;
			if (&mparent->mnt == nd->path.mnt)
				break;
			if (unlikely(nd->flags & LOOKUP_NO_XDEV))
				return -ECHILD;
			/* we know that mountpoint was pinned */
			nd->path.dentry = mountpoint;
			nd->path.mnt = &mparent->mnt;
			inode = inode2;
			return -ECHILD;
		if (!mounted)
			break;
		if (unlikely(nd->flags & LOOKUP_NO_XDEV))
			return -ECHILD;
		nd->path.mnt = &mounted->mnt;
		nd->path.dentry = mounted->mnt.mnt_root;
		inode = nd->path.dentry->d_inode;
		nd->seq = read_seqcount_begin(&nd->path.dentry->d_seq);

static int follow_dotdot(struct nameidata *nd)
{
	while (1) {
		if (path_equal(&nd->path, &nd->root)) {
			if (unlikely(nd->flags & LOOKUP_BENEATH))
				return -EXDEV;
			break;
		}
		if (nd->path.dentry != nd->path.mnt->mnt_root) {
			int ret = path_parent_directory(&nd->path);
			if (ret)
				return ret;
		}
		if (!follow_up(&nd->path))
			break;
		if (unlikely(nd->flags & LOOKUP_NO_XDEV))
			return -EXDEV;
	}
	follow_mount(&nd->path);
	nd->inode = nd->path.dentry->d_inode;
	return 0;
static inline int handle_dots(struct nameidata *nd, int type)
{
	if (type == LAST_DOTDOT) {
		int error = 0;

		if (!nd->root.mnt) {
			error = set_root(nd);
			if (error)
				return error;
		}
		if (nd->flags & LOOKUP_RCU)
			error = follow_dotdot_rcu(nd);
		else
			error = follow_dotdot(nd);
		if (error)
			return error;

		if (unlikely(nd->flags & LOOKUP_IS_SCOPED)) {
			/*
			 * If there was a racing rename or mount along our
			 * path, then we can't be sure that ".." hasn't jumped
			 * above nd->root (and so userspace should retry or use
			 * some fallback).
			 */
			smp_rmb();
			if (unlikely(__read_seqcount_retry(&mount_lock.seqcount, nd->m_seq)))
				return -EAGAIN;
			if (unlikely(__read_seqcount_retry(&rename_lock.seqcount, nd->r_seq)))
				return -EAGAIN;
		}
	}
	return 0;
}

/* must be paired with terminate_walk() */
static const char *path_init(struct nameidata *nd, unsigned flags)
{
	int error;
	const char *s = nd->name->name;

	if (!*s)
		flags &= ~LOOKUP_RCU;
	nd->last_type = LAST_ROOT; /* if there are only slashes... */
	nd->flags = flags | LOOKUP_JUMPED | LOOKUP_PARENT;
	nd->depth = 0;

	nd->m_seq = __read_seqcount_begin(&mount_lock.seqcount);
	nd->r_seq = __read_seqcount_begin(&rename_lock.seqcount);
	smp_rmb();

	if (flags & LOOKUP_ROOT) {
		struct dentry *root = nd->root.dentry;
		struct inode *inode = root->d_inode;
		if (*s && unlikely(!d_can_lookup(root)))
		nd->path = nd->root;
		nd->inode = inode;
		if (flags & LOOKUP_RCU) {
			nd->seq = read_seqcount_begin(&nd->path.dentry->d_seq);
			nd->root_seq = nd->seq;
		} else {
			path_get(&nd->path);
		}
		return s;
	nd->path.mnt = NULL;
	nd->path.dentry = NULL;

	/* Absolute pathname -- fetch the root (LOOKUP_IN_ROOT uses nd->dfd). */
	if (*s == '/' && !(flags & LOOKUP_IN_ROOT)) {
		error = nd_jump_root(nd);
		if (unlikely(error))
			return ERR_PTR(error);
		return s;
	}

	/* Relative pathname -- get the starting-point it is relative to. */
	if (nd->dfd == AT_FDCWD) {
		if (flags & LOOKUP_RCU) {
			struct fs_struct *fs = current->fs;
			unsigned seq;

			get_fs_pwd(current->fs, &nd->path);
			nd->inode = nd->path.dentry->d_inode;
		}
	} else {
		/* Caller must check execute permissions on the starting path component */
		struct fd f = fdget_raw(nd->dfd);
		struct dentry *dentry;
			nd->inode = nd->path.dentry->d_inode;
		}
		fdput(f);
	}

	/* For scoped-lookups we need to set the root to the dirfd as well. */
	if (flags & LOOKUP_IS_SCOPED) {
		nd->root = nd->path;
		if (flags & LOOKUP_RCU) {
			nd->root_seq = nd->seq;
		} else {
			path_get(&nd->root);
			nd->flags |= LOOKUP_ROOT_GRABBED;
		}
	}
	return s;
}

static const char *trailing_symlink(struct nameidata *nd)
{