	spin_lock_irqsave(&timer->dev->lock, flags);
	if (timer->id >= 0)
		list_move_tail(&timer->list, &timer->dev->expired);
	wake_up_interruptible(&timer->dev->wait);
	spin_unlock_irqrestore(&timer->dev->lock, flags);
}

static int
misdn_add_timer(struct mISDNtimerdev *dev, int timeout)