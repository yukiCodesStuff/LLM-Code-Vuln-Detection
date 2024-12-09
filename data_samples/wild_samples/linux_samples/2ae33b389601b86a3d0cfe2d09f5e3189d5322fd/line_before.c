{
	int ret;
	struct line *line = tty->driver_data;

	ret = enable_chan(line);
	if (ret)
		return ret;

	if (!line->sigio) {
		chan_enable_winch(line->chan_out, tty);
		line->sigio = 1;
	}
	if (!line->sigio) {
		chan_enable_winch(line->chan_out, tty);
		line->sigio = 1;
	}

	chan_window_size(line, &tty->winsize.ws_row,
		&tty->winsize.ws_col);

	return 0;
}

static const struct tty_port_operations line_port_ops = {
	.activate = line_activate,
};

int line_open(struct tty_struct *tty, struct file *filp)
{
	struct line *line = tty->driver_data;

	return tty_port_open(&line->port, tty, filp);
}
{
	int ret;

	ret = tty_standard_install(driver, tty);
	if (ret)
		return ret;

	tty->driver_data = line;

	return 0;
}

static void unregister_winch(struct tty_struct *tty);

void line_cleanup(struct tty_struct *tty)
{
	struct line *line = tty->driver_data;

	if (line->sigio) {
		unregister_winch(tty);
		line->sigio = 0;
	}
struct winch {
	struct list_head list;
	int fd;
	int tty_fd;
	int pid;
	struct tty_struct *tty;
	unsigned long stack;
	struct work_struct work;
};

static void __free_winch(struct work_struct *work)
{
	struct winch *winch = container_of(work, struct winch, work);
	um_free_irq(WINCH_IRQ, winch);

	if (winch->pid != -1)
		os_kill_process(winch->pid, 1);
	if (winch->stack != 0)
		free_stack(winch->stack, 0);
	kfree(winch);
}
			if (err != -EAGAIN) {
				winch->fd = -1;
				list_del(&winch->list);
				os_close_file(fd);
				printk(KERN_ERR "winch_interrupt : "
				       "read failed, errno = %d\n", -err);
				printk(KERN_ERR "fd %d is losing SIGWINCH "
				       "support\n", winch->tty_fd);
				INIT_WORK(&winch->work, __free_winch);
				schedule_work(&winch->work);
				return IRQ_HANDLED;
			}
			goto out;
		}
	}
	tty = winch->tty;
	if (tty != NULL) {
		line = tty->driver_data;
		if (line != NULL) {
			chan_window_size(line, &tty->winsize.ws_row,
					 &tty->winsize.ws_col);
			kill_pgrp(tty->pgrp, SIGWINCH, 1);
		}
	}
 out:
	if (winch->fd != -1)
		reactivate_fd(winch->fd, WINCH_IRQ);
	return IRQ_HANDLED;
}

void register_winch_irq(int fd, int tty_fd, int pid, struct tty_struct *tty,
			unsigned long stack)
{
	struct winch *winch;

	winch = kmalloc(sizeof(*winch), GFP_KERNEL);
	if (winch == NULL) {
		printk(KERN_ERR "register_winch_irq - kmalloc failed\n");
		goto cleanup;
	}

	*winch = ((struct winch) { .list  	= LIST_HEAD_INIT(winch->list),
				   .fd  	= fd,
				   .tty_fd 	= tty_fd,
				   .pid  	= pid,
				   .tty 	= tty,
				   .stack	= stack });

	if (um_request_irq(WINCH_IRQ, fd, IRQ_READ, winch_interrupt,
			   IRQF_SHARED, "winch", winch) < 0) {
		printk(KERN_ERR "register_winch_irq - failed to register "
		       "IRQ\n");
		goto out_free;
	}

	spin_lock(&winch_handler_lock);
	list_add(&winch->list, &winch_handlers);
	spin_unlock(&winch_handler_lock);

	return;

 out_free:
	kfree(winch);
 cleanup:
	os_kill_process(pid, 1);
	os_close_file(fd);
	if (stack != 0)
		free_stack(stack, 0);
}

static void unregister_winch(struct tty_struct *tty)
{
	struct list_head *ele, *next;
	struct winch *winch;

	spin_lock(&winch_handler_lock);

	list_for_each_safe(ele, next, &winch_handlers) {
		winch = list_entry(ele, struct winch, list);
		if (winch->tty == tty) {
			free_winch(winch);
			break;
		}
	}
	spin_unlock(&winch_handler_lock);
}

static void winch_cleanup(void)
{
	struct list_head *ele, *next;
	struct winch *winch;

	spin_lock(&winch_handler_lock);

	list_for_each_safe(ele, next, &winch_handlers) {
		winch = list_entry(ele, struct winch, list);
		free_winch(winch);
	}

	spin_unlock(&winch_handler_lock);
}
__uml_exitcall(winch_cleanup);

char *add_xterm_umid(char *base)
{
	char *umid, *title;
	int len;

	umid = get_umid();
	if (*umid == '\0')
		return base;

	len = strlen(base) + strlen(" ()") + strlen(umid) + 1;
	title = kmalloc(len, GFP_KERNEL);
	if (title == NULL) {
		printk(KERN_ERR "Failed to allocate buffer for xterm title\n");
		return base;
	}

	snprintf(title, len, "%s (%s)", base, umid);
	return title;
}
	if (winch == NULL) {
		printk(KERN_ERR "register_winch_irq - kmalloc failed\n");
		goto cleanup;
	}

	*winch = ((struct winch) { .list  	= LIST_HEAD_INIT(winch->list),
				   .fd  	= fd,
				   .tty_fd 	= tty_fd,
				   .pid  	= pid,
				   .tty 	= tty,
				   .stack	= stack });

	if (um_request_irq(WINCH_IRQ, fd, IRQ_READ, winch_interrupt,
			   IRQF_SHARED, "winch", winch) < 0) {
		printk(KERN_ERR "register_winch_irq - failed to register "
		       "IRQ\n");
		goto out_free;
	}
{
	struct list_head *ele, *next;
	struct winch *winch;

	spin_lock(&winch_handler_lock);

	list_for_each_safe(ele, next, &winch_handlers) {
		winch = list_entry(ele, struct winch, list);
		if (winch->tty == tty) {
			free_winch(winch);
			break;
		}
	}