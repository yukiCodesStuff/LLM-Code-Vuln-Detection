	return !mnt_already_visible(ns, mnt, new_mnt_flags);
}

bool mnt_may_suid(struct vfsmount *mnt)
{
	/*
	 * Foreign mounts (accessed via fchdir or through /proc
	 * symlinks) are always treated as if they are nosuid.  This
	 * prevents namespaces from trusting potentially unsafe
	 * suid/sgid bits, file caps, or security labels that originate
	 * in other namespaces.
	 */
	return !(mnt->mnt_flags & MNT_NOSUID) && check_mnt(real_mount(mnt)) &&
	       current_in_userns(mnt->mnt_sb->s_user_ns);
}

static struct ns_common *mntns_get(struct task_struct *task)
{
	struct ns_common *ns = NULL;
	struct nsproxy *nsproxy;