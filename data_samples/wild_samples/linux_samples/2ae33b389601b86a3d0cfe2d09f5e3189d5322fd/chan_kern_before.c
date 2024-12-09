	list_for_each(ele, chans) {
		chan = list_entry(ele, struct chan, list);
		ret = open_one_chan(chan);
		if (chan->primary)
			err = ret;
	}
	return err;
}

void chan_enable_winch(struct chan *chan, struct tty_struct *tty)
{
	if (chan && chan->primary && chan->ops->winch)
		register_winch(chan->fd, tty);
}

static void line_timer_cb(struct work_struct *work)
{
	struct line *line = container_of(work, struct line, task.work);

	if (!line->throttled)
		chan_interrupt(line, line->driver->read_irq);
}

int enable_chan(struct line *line)
{
	struct list_head *ele;
	struct chan *chan;
	int err;

	INIT_DELAYED_WORK(&line->task, line_timer_cb);

	list_for_each(ele, &line->chan_list) {
		chan = list_entry(ele, struct chan, list);
		err = open_one_chan(chan);
		if (err) {
			if (chan->primary)
				goto out_close;

			continue;
		}

		if (chan->enabled)
			continue;
		err = line_setup_irq(chan->fd, chan->input, chan->output, line,
				     chan);
		if (err)
			goto out_close;

		chan->enabled = 1;
	}