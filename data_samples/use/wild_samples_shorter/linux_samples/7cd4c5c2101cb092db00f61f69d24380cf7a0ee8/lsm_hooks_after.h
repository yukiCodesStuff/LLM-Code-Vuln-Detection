 *	security attributes, e.g. for /proc/pid inodes.
 *	@p contains the task_struct for the task.
 *	@inode contains the inode structure for the inode.
 * @userns_create:
 *	Check permission prior to creating a new user namespace.
 *	@cred points to prepared creds.
 *	Return 0 if successful, otherwise < 0 error code.
 *
 * Security hooks for Netlink messaging.
 *
 * @netlink_send: