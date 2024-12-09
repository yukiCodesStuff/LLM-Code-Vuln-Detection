	while (clipcount--) {
		if (copy_in_user(&kclips->c, &uclips->c, sizeof(uclips->c)))
			return -EFAULT;
		if (put_user(clipcount ? kclips + 1 : NULL, &kclips->next))
			return -EFAULT;
		uclips++;
		kclips++;
	}
	return 0;
}

static int put_v4l2_window32(struct v4l2_window __user *kp,
			     struct v4l2_window32 __user *up)
{
	struct v4l2_clip __user *kclips;
	struct v4l2_clip32 __user *uclips;
	compat_caddr_t p;
	u32 clipcount;

	if (copy_in_user(&up->w, &kp->w, sizeof(kp->w)) ||
	    assign_in_user(&up->field, &kp->field) ||
	    assign_in_user(&up->chromakey, &kp->chromakey) ||
	    assign_in_user(&up->global_alpha, &kp->global_alpha) ||
	    get_user(clipcount, &kp->clipcount) ||
	    put_user(clipcount, &up->clipcount))
		return -EFAULT;
	if (!clipcount)
		return 0;

	if (get_user(kclips, &kp->clips))
		return -EFAULT;
	if (get_user(p, &up->clips))
		return -EFAULT;
	uclips = compat_ptr(p);
	while (clipcount--) {
		if (copy_in_user(&uclips->c, &kclips->c, sizeof(uclips->c)))
			return -EFAULT;
		uclips++;
		kclips++;
	}
	return 0;
}
{
	struct v4l2_clip __user *kclips;
	struct v4l2_clip32 __user *uclips;
	compat_caddr_t p;
	u32 clipcount;

	if (copy_in_user(&up->w, &kp->w, sizeof(kp->w)) ||
	    assign_in_user(&up->field, &kp->field) ||
	    assign_in_user(&up->chromakey, &kp->chromakey) ||
	    assign_in_user(&up->global_alpha, &kp->global_alpha) ||
	    get_user(clipcount, &kp->clipcount) ||
	    put_user(clipcount, &up->clipcount))
		return -EFAULT;
	if (!clipcount)
		return 0;

	if (get_user(kclips, &kp->clips))
		return -EFAULT;
	if (get_user(p, &up->clips))
		return -EFAULT;
	uclips = compat_ptr(p);
	while (clipcount--) {
		if (copy_in_user(&uclips->c, &kclips->c, sizeof(uclips->c)))
			return -EFAULT;
		uclips++;
		kclips++;
	}