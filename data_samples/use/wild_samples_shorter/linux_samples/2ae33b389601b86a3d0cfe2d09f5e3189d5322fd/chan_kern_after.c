	return err;
}

void chan_enable_winch(struct chan *chan, struct tty_port *port)
{
	if (chan && chan->primary && chan->ops->winch)
		register_winch(chan->fd, port);
}

static void line_timer_cb(struct work_struct *work)
{