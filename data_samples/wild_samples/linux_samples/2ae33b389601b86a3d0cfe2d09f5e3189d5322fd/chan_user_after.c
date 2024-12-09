	while(1) {
		/*
		 * This will be interrupted by SIGWINCH only, since
		 * other signals are blocked.
		 */
		sigsuspend(&sigs);

		count = write(pipe_fd, &c, sizeof(c));
		if (count != sizeof(c))
			printk(UM_KERN_ERR "winch_thread : write failed, "
			       "err = %d\n", errno);
	}
}

static int winch_tramp(int fd, struct tty_port *port, int *fd_out,
		       unsigned long *stack_out)
{
	struct winch_data data;
	int fds[2], n, err;
	char c;

	err = os_pipe(fds, 1, 1);
	if (err < 0) {
		printk(UM_KERN_ERR "winch_tramp : os_pipe failed, err = %d\n",
		       -err);
		goto out;
	}
	if (os_set_fd_block(*fd_out, 0)) {
		printk(UM_KERN_ERR "winch_tramp: failed to set thread_fd "
		       "non-blocking.\n");
		goto out_close;
	}

	return err;

 out_close:
	close(fds[1]);
	close(fds[0]);
 out:
	return err;
}

void register_winch(int fd, struct tty_port *port)
{
	unsigned long stack;
	int pid, thread, count, thread_fd = -1;
	char c = 1;

	if (!isatty(fd))
		return;

	pid = tcgetpgrp(fd);
	if (is_skas_winch(pid, fd, port)) {
		register_winch_irq(-1, fd, -1, port, 0);
		return;
	}
{
	unsigned long stack;
	int pid, thread, count, thread_fd = -1;
	char c = 1;

	if (!isatty(fd))
		return;

	pid = tcgetpgrp(fd);
	if (is_skas_winch(pid, fd, port)) {
		register_winch_irq(-1, fd, -1, port, 0);
		return;
	}