#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/mount.h>
#include "ecryptfs_kernel.h"

struct ecryptfs_open_req {
	struct file **lower_file;
	flags |= IS_RDONLY(d_inode(lower_dentry)) ? O_RDONLY : O_RDWR;
	(*lower_file) = dentry_open(&req.path, flags, cred);
	if (!IS_ERR(*lower_file))
		goto out;
	if ((flags & O_ACCMODE) == O_RDONLY) {
		rc = PTR_ERR((*lower_file));
		goto out;
	}
	mutex_unlock(&ecryptfs_kthread_ctl.mux);
	wake_up(&ecryptfs_kthread_ctl.wait);
	wait_for_completion(&req.done);
	if (IS_ERR(*lower_file))
		rc = PTR_ERR(*lower_file);
out:
	return rc;
}