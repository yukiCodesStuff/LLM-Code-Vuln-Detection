{
	struct printer_dev		*dev = fd->private_data;
	unsigned long			flags;
	size_t				size;
	size_t				bytes_copied;
	struct usb_request		*req;
	/* This is a pointer to the current USB rx request. */
	struct usb_request		*current_rx_req;
	/* This is the number of bytes in the current rx buffer. */
	size_t				current_rx_bytes;
	/* This is a pointer to the current rx buffer. */
	u8				*current_rx_buf;

	if (len == 0)
		return -EINVAL;

	DBG(dev, "printer_read trying to read %d bytes\n", (int)len);

	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return -ENODEV;
	}
		if (fd->f_flags & (O_NONBLOCK|O_NDELAY)) {
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}

		/* Sleep until data is available */
		wait_event_interruptible(dev->rx_wait,
				(likely(!list_empty(&dev->rx_buffers))));
		spin_lock_irqsave(&dev->lock, flags);
	}

	/* We have data to return then copy it to the caller's buffer.*/
	while ((current_rx_bytes || likely(!list_empty(&dev->rx_buffers)))
			&& len) {
		if (current_rx_bytes == 0) {
			req = container_of(dev->rx_buffers.next,
					struct usb_request, list);
			list_del_init(&req->list);

			if (req->actual && req->buf) {
				current_rx_req = req;
				current_rx_bytes = req->actual;
				current_rx_buf = req->buf;
			} else {
				list_add(&req->list, &dev->rx_reqs);
				continue;
			}
		}

		/* Don't leave irqs off while doing memory copies */
		spin_unlock_irqrestore(&dev->lock, flags);

		if (len > current_rx_bytes)
			size = current_rx_bytes;
		else
			size = len;

		size -= copy_to_user(buf, current_rx_buf, size);
		bytes_copied += size;
		len -= size;
		buf += size;

		spin_lock_irqsave(&dev->lock, flags);

		/* We've disconnected or reset so return. */
		if (dev->reset_printer) {
			list_add(&current_rx_req->list, &dev->rx_reqs);
			spin_unlock_irqrestore(&dev->lock, flags);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}

		/* If we not returning all the data left in this RX request
		 * buffer then adjust the amount of data left in the buffer.
		 * Othewise if we are done with this RX request buffer then
		 * requeue it to get any incoming data from the USB host.
		 */
		if (size < current_rx_bytes) {
			current_rx_bytes -= size;
			current_rx_buf += size;
		} else {
			list_add(&current_rx_req->list, &dev->rx_reqs);
			current_rx_bytes = 0;
			current_rx_buf = NULL;
			current_rx_req = NULL;
		}
	}

	dev->current_rx_req = current_rx_req;
	dev->current_rx_bytes = current_rx_bytes;
	dev->current_rx_buf = current_rx_buf;

	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	DBG(dev, "printer_read returned %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}
		if (dev->reset_printer) {
			list_add(&current_rx_req->list, &dev->rx_reqs);
			spin_unlock_irqrestore(&dev->lock, flags);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}

		/* If we not returning all the data left in this RX request
		 * buffer then adjust the amount of data left in the buffer.
		 * Othewise if we are done with this RX request buffer then
		 * requeue it to get any incoming data from the USB host.
		 */
		if (size < current_rx_bytes) {
			current_rx_bytes -= size;
			current_rx_buf += size;
		} else {
			list_add(&current_rx_req->list, &dev->rx_reqs);
			current_rx_bytes = 0;
			current_rx_buf = NULL;
			current_rx_req = NULL;
		}
		} else {
			list_add(&current_rx_req->list, &dev->rx_reqs);
			current_rx_bytes = 0;
			current_rx_buf = NULL;
			current_rx_req = NULL;
		}
	}

	dev->current_rx_req = current_rx_req;
	dev->current_rx_bytes = current_rx_bytes;
	dev->current_rx_buf = current_rx_buf;

	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	DBG(dev, "printer_read returned %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

static ssize_t
printer_write(struct file *fd, const char __user *buf, size_t len, loff_t *ptr)
{
	struct printer_dev	*dev = fd->private_data;
	unsigned long		flags;
	size_t			size;	/* Amount of data in a TX request. */
	size_t			bytes_copied = 0;
	struct usb_request	*req;
	int			value;

	DBG(dev, "printer_write trying to send %d bytes\n", (int)len);

	if (len == 0)
		return -EINVAL;

	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return -ENODEV;
	}
{
	struct printer_dev	*dev = fd->private_data;
	unsigned long		flags;
	size_t			size;	/* Amount of data in a TX request. */
	size_t			bytes_copied = 0;
	struct usb_request	*req;
	int			value;

	DBG(dev, "printer_write trying to send %d bytes\n", (int)len);

	if (len == 0)
		return -EINVAL;

	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return -ENODEV;
	}
		if (fd->f_flags & (O_NONBLOCK|O_NDELAY)) {
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}

		/* Sleep until a write buffer is available */
		wait_event_interruptible(dev->tx_wait,
				(likely(!list_empty(&dev->tx_reqs))));
		spin_lock_irqsave(&dev->lock, flags);
	}

	while (likely(!list_empty(&dev->tx_reqs)) && len) {

		if (len > USB_BUFSIZE)
			size = USB_BUFSIZE;
		else
			size = len;

		req = container_of(dev->tx_reqs.next, struct usb_request,
				list);
		list_del_init(&req->list);

		req->complete = tx_complete;
		req->length = size;

		/* Check if we need to send a zero length packet. */
		if (len > size)
			/* They will be more TX requests so no yet. */
			req->zero = 0;
		else
			/* If the data amount is not a multiple of the
			 * maxpacket size then send a zero length packet.
			 */
			req->zero = ((len % dev->in_ep->maxpacket) == 0);

		/* Don't leave irqs off while doing memory copies */
		spin_unlock_irqrestore(&dev->lock, flags);

		if (copy_from_user(req->buf, buf, size)) {
			list_add(&req->list, &dev->tx_reqs);
			mutex_unlock(&dev->lock_printer_io);
			return bytes_copied;
		}
		if (dev->reset_printer) {
			list_add(&req->list, &dev->tx_reqs);
			spin_unlock_irqrestore(&dev->lock, flags);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}

		list_add(&req->list, &dev->tx_reqs_active);

		/* here, we unlock, and only unlock, to avoid deadlock. */
		spin_unlock(&dev->lock);
		value = usb_ep_queue(dev->in_ep, req, GFP_ATOMIC);
		spin_lock(&dev->lock);
		if (value) {
			list_move(&req->list, &dev->tx_reqs);
			spin_unlock_irqrestore(&dev->lock, flags);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}
	}

	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	DBG(dev, "printer_write sent %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

static int
printer_fsync(struct file *fd, loff_t start, loff_t end, int datasync)
{
	struct printer_dev	*dev = fd->private_data;
	struct inode *inode = file_inode(fd);
	unsigned long		flags;
	int			tx_list_empty;

	inode_lock(inode);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		inode_unlock(inode);
		return -ENODEV;
	}

	tx_list_empty = (likely(list_empty(&dev->tx_reqs)));
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!tx_list_empty) {
		/* Sleep until all data has been sent */
		wait_event_interruptible(dev->tx_flush_wait,
				(likely(list_empty(&dev->tx_reqs_active))));
	}
	inode_unlock(inode);

	return 0;
}

static __poll_t
printer_poll(struct file *fd, poll_table *wait)
{
	struct printer_dev	*dev = fd->private_data;
	unsigned long		flags;
	__poll_t		status = 0;

	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return EPOLLERR | EPOLLHUP;
	}

	setup_rx_reqs(dev);
	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	poll_wait(fd, &dev->rx_wait, wait);
	poll_wait(fd, &dev->tx_wait, wait);

	spin_lock_irqsave(&dev->lock, flags);
	if (likely(!list_empty(&dev->tx_reqs)))
		status |= EPOLLOUT | EPOLLWRNORM;

	if (likely(dev->current_rx_bytes) ||
			likely(!list_empty(&dev->rx_buffers)))
		status |= EPOLLIN | EPOLLRDNORM;

	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

static long
printer_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{
	struct printer_dev	*dev = fd->private_data;
	unsigned long		flags;
	int			status = 0;

	DBG(dev, "printer_ioctl: cmd=0x%4.4x, arg=%lu\n", code, arg);

	/* handle ioctls */

	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return -ENODEV;
	}

	switch (code) {
	case GADGET_GET_PRINTER_STATUS:
		status = (int)dev->printer_status;
		break;
	case GADGET_SET_PRINTER_STATUS:
		dev->printer_status = (u8)arg;
		break;
	default:
		/* could not handle ioctl */
		DBG(dev, "printer_ioctl: ERROR cmd=0x%4.4xis not supported\n",
				code);
		status = -ENOTTY;
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

/* used after endpoint configuration */
static const struct file_operations printer_io_operations = {
	.owner =	THIS_MODULE,
	.open =		printer_open,
	.read =		printer_read,
	.write =	printer_write,
	.fsync =	printer_fsync,
	.poll =		printer_poll,
	.unlocked_ioctl = printer_ioctl,
	.release =	printer_close,
	.llseek =	noop_llseek,
};

/*-------------------------------------------------------------------------*/

static int
set_printer_interface(struct printer_dev *dev)
{
	int			result = 0;

	dev->in_ep->desc = ep_desc(dev->gadget, &fs_ep_in_desc, &hs_ep_in_desc,
				&ss_ep_in_desc);
	dev->in_ep->driver_data = dev;

	dev->out_ep->desc = ep_desc(dev->gadget, &fs_ep_out_desc,
				    &hs_ep_out_desc, &ss_ep_out_desc);
	dev->out_ep->driver_data = dev;

	result = usb_ep_enable(dev->in_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n", dev->in_ep->name, result);
		goto done;
	}

	result = usb_ep_enable(dev->out_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n", dev->out_ep->name, result);
		goto done;
	}

done:
	/* on error, disable any endpoints  */
	if (result != 0) {
		(void) usb_ep_disable(dev->in_ep);
		(void) usb_ep_disable(dev->out_ep);
		dev->in_ep->desc = NULL;
		dev->out_ep->desc = NULL;
	}

	/* caller is responsible for cleanup on error */
	return result;
}

static void printer_reset_interface(struct printer_dev *dev)
{
	unsigned long	flags;

	if (dev->interface < 0)
		return;

	if (dev->in_ep->desc)
		usb_ep_disable(dev->in_ep);

	if (dev->out_ep->desc)
		usb_ep_disable(dev->out_ep);

	spin_lock_irqsave(&dev->lock, flags);
	dev->in_ep->desc = NULL;
	dev->out_ep->desc = NULL;
	dev->interface = -1;
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* Change our operational Interface. */
static int set_interface(struct printer_dev *dev, unsigned number)
{
	int			result = 0;

	/* Free the current interface */
	printer_reset_interface(dev);

	result = set_printer_interface(dev);
	if (result)
		printer_reset_interface(dev);
	else
		dev->interface = number;

	if (!result)
		INFO(dev, "Using interface %x\n", number);

	return result;
}

static void printer_soft_reset(struct printer_dev *dev)
{
	struct usb_request	*req;

	if (usb_ep_disable(dev->in_ep))
		DBG(dev, "Failed to disable USB in_ep\n");
	if (usb_ep_disable(dev->out_ep))
		DBG(dev, "Failed to disable USB out_ep\n");

	if (dev->current_rx_req != NULL) {
		list_add(&dev->current_rx_req->list, &dev->rx_reqs);
		dev->current_rx_req = NULL;
	}
	dev->current_rx_bytes = 0;
	dev->current_rx_buf = NULL;
	dev->reset_printer = 1;

	while (likely(!(list_empty(&dev->rx_buffers)))) {
		req = container_of(dev->rx_buffers.next, struct usb_request,
				list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->rx_reqs);
	}

	while (likely(!(list_empty(&dev->rx_reqs_active)))) {
		req = container_of(dev->rx_buffers.next, struct usb_request,
				list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->rx_reqs);
	}

	while (likely(!(list_empty(&dev->tx_reqs_active)))) {
		req = container_of(dev->tx_reqs_active.next,
				struct usb_request, list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->tx_reqs);
	}

	if (usb_ep_enable(dev->in_ep))
		DBG(dev, "Failed to enable USB in_ep\n");
	if (usb_ep_enable(dev->out_ep))
		DBG(dev, "Failed to enable USB out_ep\n");

	wake_up_interruptible(&dev->rx_wait);
	wake_up_interruptible(&dev->tx_wait);
	wake_up_interruptible(&dev->tx_flush_wait);
}

/*-------------------------------------------------------------------------*/

static bool gprinter_req_match(struct usb_function *f,
			       const struct usb_ctrlrequest *ctrl,
			       bool config0)
{
	struct printer_dev	*dev = func_to_printer(f);
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	if (config0)
		return false;

	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE ||
	    (ctrl->bRequestType & USB_TYPE_MASK) != USB_TYPE_CLASS)
		return false;

	switch (ctrl->bRequest) {
	case GET_DEVICE_ID:
		w_index >>= 8;
		if (USB_DIR_IN & ctrl->bRequestType)
			break;
		return false;
	case GET_PORT_STATUS:
		if (!w_value && w_length == 1 &&
		    (USB_DIR_IN & ctrl->bRequestType))
			break;
		return false;
	case SOFT_RESET:
		if (!w_value && !w_length &&
		   !(USB_DIR_IN & ctrl->bRequestType))
			break;
		fallthrough;
	default:
		return false;
	}
	return w_index == dev->interface;
}

/*
 * The setup() callback implements all the ep0 functionality that's not
 * handled lower down.
 */
static int printer_func_setup(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct printer_dev *dev = func_to_printer(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	u8			*buf = req->buf;
	int			value = -EOPNOTSUPP;
	u16			wIndex = le16_to_cpu(ctrl->wIndex);
	u16			wValue = le16_to_cpu(ctrl->wValue);
	u16			wLength = le16_to_cpu(ctrl->wLength);

	DBG(dev, "ctrl req%02x.%02x v%04x i%04x l%d\n",
		ctrl->bRequestType, ctrl->bRequest, wValue, wIndex, wLength);

	switch (ctrl->bRequestType&USB_TYPE_MASK) {
	case USB_TYPE_CLASS:
		switch (ctrl->bRequest) {
		case GET_DEVICE_ID: /* Get the IEEE-1284 PNP String */
			/* Only one printer interface is supported. */
			if ((wIndex>>8) != dev->interface)
				break;

			if (!*dev->pnp_string) {
				value = 0;
				break;
			}
			value = strlen(*dev->pnp_string);
			buf[0] = (value >> 8) & 0xFF;
			buf[1] = value & 0xFF;
			memcpy(buf + 2, *dev->pnp_string, value);
			DBG(dev, "1284 PNP String: %x %s\n", value,
			    *dev->pnp_string);
			break;

		case GET_PORT_STATUS: /* Get Port Status */
			/* Only one printer interface is supported. */
			if (wIndex != dev->interface)
				break;

			buf[0] = dev->printer_status;
			value = min_t(u16, wLength, 1);
			break;

		case SOFT_RESET: /* Soft Reset */
			/* Only one printer interface is supported. */
			if (wIndex != dev->interface)
				break;

			printer_soft_reset(dev);

			value = 0;
			break;

		default:
			goto unknown;
		}
		if (value) {
			list_move(&req->list, &dev->tx_reqs);
			spin_unlock_irqrestore(&dev->lock, flags);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}
	}

	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	DBG(dev, "printer_write sent %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

static int
printer_fsync(struct file *fd, loff_t start, loff_t end, int datasync)
{
	struct printer_dev	*dev = fd->private_data;
	struct inode *inode = file_inode(fd);
	unsigned long		flags;
	int			tx_list_empty;

	inode_lock(inode);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		inode_unlock(inode);
		return -ENODEV;
	}

	tx_list_empty = (likely(list_empty(&dev->tx_reqs)));
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!tx_list_empty) {
		/* Sleep until all data has been sent */
		wait_event_interruptible(dev->tx_flush_wait,
				(likely(list_empty(&dev->tx_reqs_active))));
	}
	inode_unlock(inode);

	return 0;
}

static __poll_t
printer_poll(struct file *fd, poll_table *wait)
{
	struct printer_dev	*dev = fd->private_data;
	unsigned long		flags;
	__poll_t		status = 0;

	mutex_lock(&dev->lock_printer_io);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		mutex_unlock(&dev->lock_printer_io);
		return EPOLLERR | EPOLLHUP;
	}

	setup_rx_reqs(dev);
	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	poll_wait(fd, &dev->rx_wait, wait);
	poll_wait(fd, &dev->tx_wait, wait);

	spin_lock_irqsave(&dev->lock, flags);
	if (likely(!list_empty(&dev->tx_reqs)))
		status |= EPOLLOUT | EPOLLWRNORM;

	if (likely(dev->current_rx_bytes) ||
			likely(!list_empty(&dev->rx_buffers)))
		status |= EPOLLIN | EPOLLRDNORM;

	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

static long
printer_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{
	struct printer_dev	*dev = fd->private_data;
	unsigned long		flags;
	int			status = 0;

	DBG(dev, "printer_ioctl: cmd=0x%4.4x, arg=%lu\n", code, arg);

	/* handle ioctls */

	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return -ENODEV;
	}

	switch (code) {
	case GADGET_GET_PRINTER_STATUS:
		status = (int)dev->printer_status;
		break;
	case GADGET_SET_PRINTER_STATUS:
		dev->printer_status = (u8)arg;
		break;
	default:
		/* could not handle ioctl */
		DBG(dev, "printer_ioctl: ERROR cmd=0x%4.4xis not supported\n",
				code);
		status = -ENOTTY;
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

/* used after endpoint configuration */
static const struct file_operations printer_io_operations = {
	.owner =	THIS_MODULE,
	.open =		printer_open,
	.read =		printer_read,
	.write =	printer_write,
	.fsync =	printer_fsync,
	.poll =		printer_poll,
	.unlocked_ioctl = printer_ioctl,
	.release =	printer_close,
	.llseek =	noop_llseek,
};

/*-------------------------------------------------------------------------*/

static int
set_printer_interface(struct printer_dev *dev)
{
	int			result = 0;

	dev->in_ep->desc = ep_desc(dev->gadget, &fs_ep_in_desc, &hs_ep_in_desc,
				&ss_ep_in_desc);
	dev->in_ep->driver_data = dev;

	dev->out_ep->desc = ep_desc(dev->gadget, &fs_ep_out_desc,
				    &hs_ep_out_desc, &ss_ep_out_desc);
	dev->out_ep->driver_data = dev;

	result = usb_ep_enable(dev->in_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n", dev->in_ep->name, result);
		goto done;
	}

	result = usb_ep_enable(dev->out_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n", dev->out_ep->name, result);
		goto done;
	}

done:
	/* on error, disable any endpoints  */
	if (result != 0) {
		(void) usb_ep_disable(dev->in_ep);
		(void) usb_ep_disable(dev->out_ep);
		dev->in_ep->desc = NULL;
		dev->out_ep->desc = NULL;
	}

	/* caller is responsible for cleanup on error */
	return result;
}

static void printer_reset_interface(struct printer_dev *dev)
{
	unsigned long	flags;

	if (dev->interface < 0)
		return;

	if (dev->in_ep->desc)
		usb_ep_disable(dev->in_ep);

	if (dev->out_ep->desc)
		usb_ep_disable(dev->out_ep);

	spin_lock_irqsave(&dev->lock, flags);
	dev->in_ep->desc = NULL;
	dev->out_ep->desc = NULL;
	dev->interface = -1;
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* Change our operational Interface. */
static int set_interface(struct printer_dev *dev, unsigned number)
{
	int			result = 0;

	/* Free the current interface */
	printer_reset_interface(dev);

	result = set_printer_interface(dev);
	if (result)
		printer_reset_interface(dev);
	else
		dev->interface = number;

	if (!result)
		INFO(dev, "Using interface %x\n", number);

	return result;
}

static void printer_soft_reset(struct printer_dev *dev)
{
	struct usb_request	*req;

	if (usb_ep_disable(dev->in_ep))
		DBG(dev, "Failed to disable USB in_ep\n");
	if (usb_ep_disable(dev->out_ep))
		DBG(dev, "Failed to disable USB out_ep\n");

	if (dev->current_rx_req != NULL) {
		list_add(&dev->current_rx_req->list, &dev->rx_reqs);
		dev->current_rx_req = NULL;
	}
	dev->current_rx_bytes = 0;
	dev->current_rx_buf = NULL;
	dev->reset_printer = 1;

	while (likely(!(list_empty(&dev->rx_buffers)))) {
		req = container_of(dev->rx_buffers.next, struct usb_request,
				list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->rx_reqs);
	}

	while (likely(!(list_empty(&dev->rx_reqs_active)))) {
		req = container_of(dev->rx_buffers.next, struct usb_request,
				list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->rx_reqs);
	}

	while (likely(!(list_empty(&dev->tx_reqs_active)))) {
		req = container_of(dev->tx_reqs_active.next,
				struct usb_request, list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->tx_reqs);
	}

	if (usb_ep_enable(dev->in_ep))
		DBG(dev, "Failed to enable USB in_ep\n");
	if (usb_ep_enable(dev->out_ep))
		DBG(dev, "Failed to enable USB out_ep\n");

	wake_up_interruptible(&dev->rx_wait);
	wake_up_interruptible(&dev->tx_wait);
	wake_up_interruptible(&dev->tx_flush_wait);
}

/*-------------------------------------------------------------------------*/

static bool gprinter_req_match(struct usb_function *f,
			       const struct usb_ctrlrequest *ctrl,
			       bool config0)
{
	struct printer_dev	*dev = func_to_printer(f);
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	if (config0)
		return false;

	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE ||
	    (ctrl->bRequestType & USB_TYPE_MASK) != USB_TYPE_CLASS)
		return false;

	switch (ctrl->bRequest) {
	case GET_DEVICE_ID:
		w_index >>= 8;
		if (USB_DIR_IN & ctrl->bRequestType)
			break;
		return false;
	case GET_PORT_STATUS:
		if (!w_value && w_length == 1 &&
		    (USB_DIR_IN & ctrl->bRequestType))
			break;
		return false;
	case SOFT_RESET:
		if (!w_value && !w_length &&
		   !(USB_DIR_IN & ctrl->bRequestType))
			break;
		fallthrough;
	default:
		return false;
	}
	return w_index == dev->interface;
}

/*
 * The setup() callback implements all the ep0 functionality that's not
 * handled lower down.
 */
static int printer_func_setup(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct printer_dev *dev = func_to_printer(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	u8			*buf = req->buf;
	int			value = -EOPNOTSUPP;
	u16			wIndex = le16_to_cpu(ctrl->wIndex);
	u16			wValue = le16_to_cpu(ctrl->wValue);
	u16			wLength = le16_to_cpu(ctrl->wLength);

	DBG(dev, "ctrl req%02x.%02x v%04x i%04x l%d\n",
		ctrl->bRequestType, ctrl->bRequest, wValue, wIndex, wLength);

	switch (ctrl->bRequestType&USB_TYPE_MASK) {
	case USB_TYPE_CLASS:
		switch (ctrl->bRequest) {
		case GET_DEVICE_ID: /* Get the IEEE-1284 PNP String */
			/* Only one printer interface is supported. */
			if ((wIndex>>8) != dev->interface)
				break;

			if (!*dev->pnp_string) {
				value = 0;
				break;
			}
			value = strlen(*dev->pnp_string);
			buf[0] = (value >> 8) & 0xFF;
			buf[1] = value & 0xFF;
			memcpy(buf + 2, *dev->pnp_string, value);
			DBG(dev, "1284 PNP String: %x %s\n", value,
			    *dev->pnp_string);
			break;

		case GET_PORT_STATUS: /* Get Port Status */
			/* Only one printer interface is supported. */
			if (wIndex != dev->interface)
				break;

			buf[0] = dev->printer_status;
			value = min_t(u16, wLength, 1);
			break;

		case SOFT_RESET: /* Soft Reset */
			/* Only one printer interface is supported. */
			if (wIndex != dev->interface)
				break;

			printer_soft_reset(dev);

			value = 0;
			break;

		default:
			goto unknown;
		}
		if (value) {
			list_move(&req->list, &dev->tx_reqs);
			spin_unlock_irqrestore(&dev->lock, flags);
			mutex_unlock(&dev->lock_printer_io);
			return -EAGAIN;
		}
	}

	spin_unlock_irqrestore(&dev->lock, flags);
	mutex_unlock(&dev->lock_printer_io);

	DBG(dev, "printer_write sent %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

static int
printer_fsync(struct file *fd, loff_t start, loff_t end, int datasync)
{
	struct printer_dev	*dev = fd->private_data;
	struct inode *inode = file_inode(fd);
	unsigned long		flags;
	int			tx_list_empty;

	inode_lock(inode);
	spin_lock_irqsave(&dev->lock, flags);

	if (dev->interface < 0) {
		spin_unlock_irqrestore(&dev->lock, flags);
		inode_unlock(inode);
		return -ENODEV;
	}