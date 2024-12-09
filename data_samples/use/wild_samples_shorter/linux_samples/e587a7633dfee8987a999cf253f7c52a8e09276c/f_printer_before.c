	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return -ENODEV;
	}

	/* We will use this flag later to check if a printer reset happened
	 * after we turn interrupts back on.
	 */
	dev->reset_printer = 0;

	setup_rx_reqs(dev);

	bytes_copied = 0;
	current_rx_req = dev->current_rx_req;
	current_rx_bytes = dev->current_rx_bytes;
		wait_event_interruptible(dev->rx_wait,
				(likely(!list_empty(&dev->rx_buffers))));
		spin_lock_irqsave(&dev->lock, flags);
	}

	/* We have data to return then copy it to the caller's buffer.*/
	while ((current_rx_bytes || likely(!list_empty(&dev->rx_buffers)))
			return -EAGAIN;
		}

		/* If we not returning all the data left in this RX request
		 * buffer then adjust the amount of data left in the buffer.
		 * Othewise if we are done with this RX request buffer then
		 * requeue it to get any incoming data from the USB host.
		return bytes_copied;
	else
		return -EAGAIN;
}

static ssize_t
printer_write(struct file *fd, const char __user *buf, size_t len, loff_t *ptr)
	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return -ENODEV;
	}

	/* Check if a printer reset happens while we have interrupts on */
	dev->reset_printer = 0;

		wait_event_interruptible(dev->tx_wait,
				(likely(!list_empty(&dev->tx_reqs))));
		spin_lock_irqsave(&dev->lock, flags);
	}

	while (likely(!list_empty(&dev->tx_reqs)) && len) {

			return -EAGAIN;
		}

		list_add(&req->list, &dev->tx_reqs_active);

		/* here, we unlock, and only unlock, to avoid deadlock. */
		spin_unlock(&dev->lock);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}
	}

	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);
		return bytes_copied;
	else
		return -EAGAIN;
}

static int
printer_fsync(struct file *fd, loff_t start, loff_t end, int datasync)