				  struct timespec *ts)
{
	unsigned long flags;
	int dev = MINOR(file_inode(file)->i_rdev);
	int avail;
	struct sync_port *port;
	unsigned char *start;
	unsigned char *end;