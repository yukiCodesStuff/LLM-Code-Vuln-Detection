static int put_v4l2_window32(struct v4l2_window __user *kp,
			     struct v4l2_window32 __user *up)
{
	struct v4l2_clip __user *kclips = kp->clips;
	struct v4l2_clip32 __user *uclips;
	compat_caddr_t p;
	u32 clipcount;

	if (!clipcount)
		return 0;

	if (get_user(p, &up->clips))
		return -EFAULT;
	uclips = compat_ptr(p);
	while (clipcount--) {