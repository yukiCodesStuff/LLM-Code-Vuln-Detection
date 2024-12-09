{
	unsigned long flags;
	int dev = MINOR(file_inode(file)->i_rdev);
	int avail;
	struct sync_port *port;
	unsigned char *start;
	unsigned char *end;

	if (dev < 0 || dev >= NBR_PORTS || !ports[dev].enabled) {
		DEBUG(pr_info("Invalid minor %d\n", dev));
		return -ENODEV;
	}
	port = &ports[dev];

	if (!port->started)
		sync_serial_start_port(port);

	/* Calculate number of available bytes */
	/* Save pointers to avoid that they are modified by interrupt */
	spin_lock_irqsave(&port->lock, flags);
	start = port->readp;
	end = port->writep;
	spin_unlock_irqrestore(&port->lock, flags);

	while ((start == end) && !port->in_buffer_len) {
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		wait_event_interruptible(port->in_wait_q,
					 !(start == end && !port->full));

		if (signal_pending(current))
			return -EINTR;

		spin_lock_irqsave(&port->lock, flags);
		start = port->readp;
		end = port->writep;
		spin_unlock_irqrestore(&port->lock, flags);
	}

	DEBUGREAD(pr_info("R%d c %d ri %u wi %u /%u\n",
			  dev, count,
			  start - port->flip, end - port->flip,
			  port->in_buffer_size));

	/* Lazy read, never return wrapped data. */
	if (end > start)
		avail = end - start;
	else
		avail = port->flip + port->in_buffer_size - start;

	count = count > avail ? avail : count;
	if (copy_to_user(buf, start, count))
		return -EFAULT;

	/* If timestamp requested, find timestamp of first returned byte
	 * and copy it.
	 * N.B: Applications that request timstamps MUST read data in
	 * chunks that are multiples of IN_DESCR_SIZE.
	 * Otherwise the timestamps will not be aligned to the data read.
	 */
	if (ts != NULL) {
		int idx = port->read_ts_idx;
		memcpy(ts, &port->timestamp[idx], sizeof(struct timespec));
		port->read_ts_idx += count / IN_DESCR_SIZE;
		if (port->read_ts_idx >= NBR_IN_DESCR)
			port->read_ts_idx = 0;
	}

	spin_lock_irqsave(&port->lock, flags);
	port->readp += count;
	/* Check for wrap */
	if (port->readp >= port->flip + port->in_buffer_size)
		port->readp = port->flip;
	port->in_buffer_len -= count;
	port->full = 0;
	spin_unlock_irqrestore(&port->lock, flags);

	DEBUGREAD(pr_info("r %d\n", count));

	return count;
}