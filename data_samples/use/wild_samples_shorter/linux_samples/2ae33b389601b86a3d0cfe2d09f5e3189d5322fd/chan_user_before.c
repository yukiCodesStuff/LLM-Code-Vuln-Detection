	}
}

static int winch_tramp(int fd, struct tty_struct *tty, int *fd_out,
		       unsigned long *stack_out)
{
	struct winch_data data;
	int fds[2], n, err;
	return err;
}

void register_winch(int fd, struct tty_struct *tty)
{
	unsigned long stack;
	int pid, thread, count, thread_fd = -1;
	char c = 1;
		return;

	pid = tcgetpgrp(fd);
	if (is_skas_winch(pid, fd, tty)) {
		register_winch_irq(-1, fd, -1, tty, 0);
		return;
	}

	if (pid == -1) {
		thread = winch_tramp(fd, tty, &thread_fd, &stack);
		if (thread < 0)
			return;

		register_winch_irq(thread_fd, fd, thread, tty, stack);

		count = write(thread_fd, &c, sizeof(c));
		if (count != sizeof(c))
			printk(UM_KERN_ERR "register_winch : failed to write "