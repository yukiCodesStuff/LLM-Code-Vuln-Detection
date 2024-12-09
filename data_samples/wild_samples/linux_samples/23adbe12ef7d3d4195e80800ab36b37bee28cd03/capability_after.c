{
	return ns_capable(&init_user_ns, cap);
}
EXPORT_SYMBOL(capable);

/**
 * capable_wrt_inode_uidgid - Check nsown_capable and uid and gid mapped
 * @inode: The inode in question
 * @cap: The capability in question
 *
 * Return true if the current task has the given capability targeted at
 * its own user namespace and that the given inode's uid and gid are
 * mapped into the current user namespace.
 */
bool capable_wrt_inode_uidgid(const struct inode *inode, int cap)
{
	struct user_namespace *ns = current_user_ns();

	return ns_capable(ns, cap) && kuid_has_mapping(ns, inode->i_uid) &&
		kgid_has_mapping(ns, inode->i_gid);
}