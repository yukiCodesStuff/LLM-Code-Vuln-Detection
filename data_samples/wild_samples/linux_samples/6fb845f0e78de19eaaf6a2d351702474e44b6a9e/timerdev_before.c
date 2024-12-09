{
	struct mISDNtimer *timer = from_timer(timer, t, tl);
	u_long			flags;

	spin_lock_irqsave(&timer->dev->lock, flags);
	if (timer->id >= 0)
		list_move_tail(&timer->list, &timer->dev->expired);
	spin_unlock_irqrestore(&timer->dev->lock, flags);
	wake_up_interruptible(&timer->dev->wait);
}

static int
misdn_add_timer(struct mISDNtimerdev *dev, int timeout)
{
	int			id;
	struct mISDNtimer	*timer;

	if (!timeout) {
		dev->work = 1;
		wake_up_interruptible(&dev->wait);
		id = 0;
	} else {
		timer = kzalloc(sizeof(struct mISDNtimer), GFP_KERNEL);
		if (!timer)
			return -ENOMEM;
		timer->dev = dev;
		timer_setup(&timer->tl, dev_expire_timer, 0);
		spin_lock_irq(&dev->lock);
		id = timer->id = dev->next_id++;
		if (dev->next_id < 0)
			dev->next_id = 1;
		list_add_tail(&timer->list, &dev->pending);
		timer->tl.expires = jiffies + ((HZ * (u_long)timeout) / 1000);
		add_timer(&timer->tl);
		spin_unlock_irq(&dev->lock);
	}
	return id;
}