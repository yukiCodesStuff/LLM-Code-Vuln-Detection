 * inode_owner_or_capable - check current task permissions to inode
 * @inode: inode being checked
 *
 * Return true if current either has CAP_FOWNER to the inode, or
 * owns the file.
 */
bool inode_owner_or_capable(const struct inode *inode)
{
	if (uid_eq(current_fsuid(), inode->i_uid))
		return true;
	if (inode_capable(inode, CAP_FOWNER))
		return true;
	return false;
}
EXPORT_SYMBOL(inode_owner_or_capable);