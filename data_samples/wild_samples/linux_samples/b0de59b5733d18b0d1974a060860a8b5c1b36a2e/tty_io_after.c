{
	int i;
	struct inode *inode = file->f_path.dentry->d_inode;
	struct tty_struct *tty = file_tty(file);
	struct tty_ldisc *ld;

	if (tty_paranoia_check(tty, inode, "tty_read"))
		return -EIO;
	if (!tty || (test_bit(TTY_IO_ERROR, &tty->flags)))
		return -EIO;

	/* We want to wait for the line discipline to sort out in this
	   situation */
	ld = tty_ldisc_ref_wait(tty);
	if (ld->ops->read)
		i = (ld->ops->read)(tty, file, buf, count);
	else
		i = -EIO;
	tty_ldisc_deref(ld);

	return i;
}

void tty_write_unlock(struct tty_struct *tty)
	__releases(&tty->atomic_write_lock)
{
	mutex_unlock(&tty->atomic_write_lock);
	wake_up_interruptible_poll(&tty->write_wait, POLLOUT);
}

int tty_write_lock(struct tty_struct *tty, int ndelay)
	__acquires(&tty->atomic_write_lock)
{
	if (!mutex_trylock(&tty->atomic_write_lock)) {
		if (ndelay)
			return -EAGAIN;
		if (mutex_lock_interruptible(&tty->atomic_write_lock))
			return -ERESTARTSYS;
	}
	for (;;) {
		size_t size = count;
		if (size > chunk)
			size = chunk;
		ret = -EFAULT;
		if (copy_from_user(tty->write_buf, buf, size))
			break;
		ret = write(tty, file, tty->write_buf, size);
		if (ret <= 0)
			break;
		written += ret;
		buf += ret;
		count -= ret;
		if (!count)
			break;
		ret = -ERESTARTSYS;
		if (signal_pending(current))
			break;
		cond_resched();
	}
	if (written)
		ret = written;
out:
	tty_write_unlock(tty);
	return ret;
}

/**
 * tty_write_message - write a message to a certain tty, not just the console.
 * @tty: the destination tty_struct
 * @msg: the message to write
 *
 * This is used for messages that need to be redirected to a specific tty.
 * We don't put it into the syslog queue right now maybe in the future if
 * really needed.
 *
 * We must still hold the BTM and test the CLOSING flag for the moment.
 */

void tty_write_message(struct tty_struct *tty, char *msg)
{
	if (tty) {
		mutex_lock(&tty->atomic_write_lock);
		tty_lock(tty);
		if (tty->ops->write && !test_bit(TTY_CLOSING, &tty->flags)) {
			tty_unlock(tty);
			tty->ops->write(tty, msg, strlen(msg));
		} else
			tty_unlock(tty);
		tty_write_unlock(tty);
	}