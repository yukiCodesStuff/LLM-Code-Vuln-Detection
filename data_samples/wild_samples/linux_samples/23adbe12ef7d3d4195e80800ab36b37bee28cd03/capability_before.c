{
	return ns_capable(&init_user_ns, cap);
}
EXPORT_SYMBOL(capable);

/**
 * inode_capable - Check superior capability over inode
 * @inode: The inode in question
 * @cap: The capability in question
 *
 * Return true if the current task has the given superior capability
 * targeted at it's own user namespace and that the given inode is owned
 * by the current user namespace or a child namespace.
 *
 * Currently we check to see if an inode is owned by the current
 * user namespace by seeing if the inode's owner maps into the
 * current user namespace.
 *
 */
bool inode_capable(const struct inode *inode, int cap)
{
	struct user_namespace *ns = current_user_ns();

	return ns_capable(ns, cap) && kuid_has_mapping(ns, inode->i_uid);
}