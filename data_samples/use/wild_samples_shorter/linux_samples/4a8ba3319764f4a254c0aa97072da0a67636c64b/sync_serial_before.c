				  struct timespec *ts)
{
	unsigned long flags;
	int dev = MINOR(file->f_dentry->d_inode->i_rdev);
	int avail;
	struct sync_port *port;
	unsigned char *start;
	unsigned char *end;