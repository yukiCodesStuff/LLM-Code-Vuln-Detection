		return ret;

	if (!line->sigio) {
		chan_enable_winch(line->chan_out, tty);
		line->sigio = 1;
	}

	chan_window_size(line, &tty->winsize.ws_row,
	return 0;
}

static const struct tty_port_operations line_port_ops = {
	.activate = line_activate,
};

int line_open(struct tty_struct *tty, struct file *filp)
{
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
}

void line_close(struct tty_struct *tty, struct file * filp)
{
	struct line *line = tty->driver_data;

	int fd;
	int tty_fd;
	int pid;
	struct tty_struct *tty;
	unsigned long stack;
	struct work_struct work;
};

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

				   .fd  	= fd,
				   .tty_fd 	= tty_fd,
				   .pid  	= pid,
				   .tty 	= tty,
				   .stack	= stack });

	if (um_request_irq(WINCH_IRQ, fd, IRQ_READ, winch_interrupt,
			   IRQF_SHARED, "winch", winch) < 0) {
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
