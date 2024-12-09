	} else {
		/* Normal events:
		 * event cell is freed after processing the event
		 */

		result = snd_seq_deliver_event(client, &cell->event, atomic, hop);
		snd_seq_cell_free(cell);
	}

	snd_seq_client_unlock(client);
	return result;
}


/* Allocate a cell from client pool and enqueue it to queue:
 * if pool is empty and blocking is TRUE, sleep until a new cell is
 * available.
 */
static int snd_seq_client_enqueue_event(struct snd_seq_client *client,
					struct snd_seq_event *event,
					struct file *file, int blocking,
					int atomic, int hop)
{
	struct snd_seq_event_cell *cell;
	int err;

	/* special queue values - force direct passing */
	if (event->queue == SNDRV_SEQ_ADDRESS_SUBSCRIBERS) {
		event->dest.client = SNDRV_SEQ_ADDRESS_SUBSCRIBERS;
		event->queue = SNDRV_SEQ_QUEUE_DIRECT;
	} else
#ifdef SUPPORT_BROADCAST
		if (event->queue == SNDRV_SEQ_ADDRESS_BROADCAST) {
			event->dest.client = SNDRV_SEQ_ADDRESS_BROADCAST;
			event->queue = SNDRV_SEQ_QUEUE_DIRECT;
		}
#endif
	if (event->dest.client == SNDRV_SEQ_ADDRESS_SUBSCRIBERS) {
		/* check presence of source port */
		struct snd_seq_client_port *src_port = snd_seq_port_use_ptr(client, event->source.port);
		if (src_port == NULL)
			return -EINVAL;
		snd_seq_port_unlock(src_port);
	}

	/* direct event processing without enqueued */
	if (snd_seq_ev_is_direct(event)) {
		if (event->type == SNDRV_SEQ_EVENT_NOTE)
			return -EINVAL; /* this event must be enqueued! */
		return snd_seq_deliver_event(client, event, atomic, hop);
	}

	/* Not direct, normal queuing */
	if (snd_seq_queue_is_used(event->queue, client->number) <= 0)
		return -EINVAL;  /* invalid queue */
	if (! snd_seq_write_pool_allocated(client))
		return -ENXIO; /* queue is not allocated */

	/* allocate an event cell */
	err = snd_seq_event_dup(client->pool, event, &cell, !blocking || atomic, file);
	if (err < 0)
		return err;

	/* we got a cell. enqueue it. */
	if ((err = snd_seq_enqueue_event(cell, atomic, hop)) < 0) {
		snd_seq_cell_free(cell);
		return err;
	}

	return 0;
}


/*
 * check validity of event type and data length.
 * return non-zero if invalid.
 */
static int check_event_type_and_length(struct snd_seq_event *ev)
{
	switch (snd_seq_ev_length_type(ev)) {
	case SNDRV_SEQ_EVENT_LENGTH_FIXED:
		if (snd_seq_ev_is_variable_type(ev))
			return -EINVAL;
		break;
	case SNDRV_SEQ_EVENT_LENGTH_VARIABLE:
		if (! snd_seq_ev_is_variable_type(ev) ||
		    (ev->data.ext.len & ~SNDRV_SEQ_EXT_MASK) >= SNDRV_SEQ_MAX_EVENT_LEN)
			return -EINVAL;
		break;
	case SNDRV_SEQ_EVENT_LENGTH_VARUSR:
		if (! snd_seq_ev_is_direct(ev))
			return -EINVAL;
		break;
	}
	return 0;
}


/* handle write() */
/* possible error values:
 *	-ENXIO	invalid client or file open mode
 *	-ENOMEM	malloc failed
 *	-EFAULT	seg. fault during copy from user space
 *	-EINVAL	invalid event
 *	-EAGAIN	no space in output pool
 *	-EINTR	interrupts while sleep
 *	-EMLINK	too many hops
 *	others	depends on return value from driver callback
 */
static ssize_t snd_seq_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *offset)
{
	struct snd_seq_client *client = file->private_data;
	int written = 0, len;
	int err;
	struct snd_seq_event event;

	if (!(snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_OUTPUT))
		return -ENXIO;

	/* check client structures are in place */
	if (snd_BUG_ON(!client))
		return -ENXIO;
		
	if (!client->accept_output || client->pool == NULL)
		return -ENXIO;

	/* allocate the pool now if the pool is not allocated yet */ 
	if (client->pool->size > 0 && !snd_seq_write_pool_allocated(client)) {
		mutex_lock(&client->ioctl_mutex);
		err = snd_seq_pool_init(client->pool);
		mutex_unlock(&client->ioctl_mutex);
		if (err < 0)
			return -ENOMEM;
	}

	/* only process whole events */
	err = -EINVAL;
	while (count >= sizeof(struct snd_seq_event)) {
		/* Read in the event header from the user */
		len = sizeof(event);
		if (copy_from_user(&event, buf, len)) {
			err = -EFAULT;
			break;
		}
		event.source.client = client->number;	/* fill in client number */
		/* Check for extension data length */
		if (check_event_type_and_length(&event)) {
			err = -EINVAL;
			break;
		}

		/* check for special events */
		if (event.type == SNDRV_SEQ_EVENT_NONE)
			goto __skip_event;
		else if (snd_seq_ev_is_reserved(&event)) {
			err = -EINVAL;
			break;
		}

		if (snd_seq_ev_is_variable(&event)) {
			int extlen = event.data.ext.len & ~SNDRV_SEQ_EXT_MASK;
			if ((size_t)(extlen + len) > count) {
				/* back out, will get an error this time or next */
				err = -EINVAL;
				break;
			}
			/* set user space pointer */
			event.data.ext.len = extlen | SNDRV_SEQ_EXT_USRPTR;
			event.data.ext.ptr = (char __force *)buf
						+ sizeof(struct snd_seq_event);
			len += extlen; /* increment data length */
		} else {
#ifdef CONFIG_COMPAT
			if (client->convert32 && snd_seq_ev_is_varusr(&event)) {
				void *ptr = (void __force *)compat_ptr(event.data.raw32.d[1]);
				event.data.ext.ptr = ptr;
			}
#endif
		}

		/* ok, enqueue it */
		err = snd_seq_client_enqueue_event(client, &event, file,
						   !(file->f_flags & O_NONBLOCK),
						   0, 0);
		if (err < 0)
			break;

	__skip_event:
		/* Update pointers and counts */
		count -= len;
		buf += len;
		written += len;
	}

	return written ? written : err;
}


/*
 * handle polling
 */
static __poll_t snd_seq_poll(struct file *file, poll_table * wait)
{
	struct snd_seq_client *client = file->private_data;
	__poll_t mask = 0;

	/* check client structures are in place */
	if (snd_BUG_ON(!client))
		return -ENXIO;

	if ((snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_INPUT) &&
	    client->data.user.fifo) {

		/* check if data is available in the outqueue */
		if (snd_seq_fifo_poll_wait(client->data.user.fifo, file, wait))
			mask |= EPOLLIN | EPOLLRDNORM;
	}

	if (snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_OUTPUT) {

		/* check if data is available in the pool */
		if (!snd_seq_write_pool_allocated(client) ||
		    snd_seq_pool_poll_wait(client->pool, file, wait))
			mask |= EPOLLOUT | EPOLLWRNORM;
	}

	return mask;
}


/*-----------------------------------------------------*/

static int snd_seq_ioctl_pversion(struct snd_seq_client *client, void *arg)
{
	int *pversion = arg;

	*pversion = SNDRV_SEQ_VERSION;
	return 0;
}

static int snd_seq_ioctl_client_id(struct snd_seq_client *client, void *arg)
{
	int *client_id = arg;

	*client_id = client->number;
	return 0;
}

/* SYSTEM_INFO ioctl() */
static int snd_seq_ioctl_system_info(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_system_info *info = arg;

	memset(info, 0, sizeof(*info));
	/* fill the info fields */
	info->queues = SNDRV_SEQ_MAX_QUEUES;
	info->clients = SNDRV_SEQ_MAX_CLIENTS;
	info->ports = SNDRV_SEQ_MAX_PORTS;
	info->channels = 256;	/* fixed limit */
	info->cur_clients = client_usage.cur;
	info->cur_queues = snd_seq_queue_get_cur_queues();

	return 0;
}


/* RUNNING_MODE ioctl() */
static int snd_seq_ioctl_running_mode(struct snd_seq_client *client, void  *arg)
{
	struct snd_seq_running_info *info = arg;
	struct snd_seq_client *cptr;
	int err = 0;

	/* requested client number */
	cptr = snd_seq_client_use_ptr(info->client);
	if (cptr == NULL)
		return -ENOENT;		/* don't change !!! */

#ifdef SNDRV_BIG_ENDIAN
	if (!info->big_endian) {
		err = -EINVAL;
		goto __err;
	}
#else
	if (info->big_endian) {
		err = -EINVAL;
		goto __err;
	}

#endif
	if (info->cpu_mode > sizeof(long)) {
		err = -EINVAL;
		goto __err;
	}
	cptr->convert32 = (info->cpu_mode < sizeof(long));
 __err:
	snd_seq_client_unlock(cptr);
	return err;
}

/* CLIENT_INFO ioctl() */
static void get_client_info(struct snd_seq_client *cptr,
			    struct snd_seq_client_info *info)
{
	info->client = cptr->number;

	/* fill the info fields */
	info->type = cptr->type;
	strcpy(info->name, cptr->name);
	info->filter = cptr->filter;
	info->event_lost = cptr->event_lost;
	memcpy(info->event_filter, cptr->event_filter, 32);
	info->num_ports = cptr->num_ports;

	if (cptr->type == USER_CLIENT)
		info->pid = pid_vnr(cptr->data.user.owner);
	else
		info->pid = -1;

	if (cptr->type == KERNEL_CLIENT)
		info->card = cptr->data.kernel.card ? cptr->data.kernel.card->number : -1;
	else
		info->card = -1;

	memset(info->reserved, 0, sizeof(info->reserved));
}

static int snd_seq_ioctl_get_client_info(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_info *client_info = arg;
	struct snd_seq_client *cptr;

	/* requested client number */
	cptr = snd_seq_client_use_ptr(client_info->client);
	if (cptr == NULL)
		return -ENOENT;		/* don't change !!! */

	get_client_info(cptr, client_info);
	snd_seq_client_unlock(cptr);

	return 0;
}


/* CLIENT_INFO ioctl() */
static int snd_seq_ioctl_set_client_info(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_info *client_info = arg;

	/* it is not allowed to set the info fields for an another client */
	if (client->number != client_info->client)
		return -EPERM;
	/* also client type must be set now */
	if (client->type != client_info->type)
		return -EINVAL;

	/* fill the info fields */
	if (client_info->name[0])
		strlcpy(client->name, client_info->name, sizeof(client->name));

	client->filter = client_info->filter;
	client->event_lost = client_info->event_lost;
	memcpy(client->event_filter, client_info->event_filter, 32);

	return 0;
}


/* 
 * CREATE PORT ioctl() 
 */
static int snd_seq_ioctl_create_port(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_port_info *info = arg;
	struct snd_seq_client_port *port;
	struct snd_seq_port_callback *callback;
	int port_idx;

	/* it is not allowed to create the port for an another client */
	if (info->addr.client != client->number)
		return -EPERM;

	port = snd_seq_create_port(client, (info->flags & SNDRV_SEQ_PORT_FLG_GIVEN_PORT) ? info->addr.port : -1);
	if (port == NULL)
		return -ENOMEM;

	if (client->type == USER_CLIENT && info->kernel) {
		port_idx = port->addr.port;
		snd_seq_port_unlock(port);
		snd_seq_delete_port(client, port_idx);
		return -EINVAL;
	}
	if (client->type == KERNEL_CLIENT) {
		if ((callback = info->kernel) != NULL) {
			if (callback->owner)
				port->owner = callback->owner;
			port->private_data = callback->private_data;
			port->private_free = callback->private_free;
			port->event_input = callback->event_input;
			port->c_src.open = callback->subscribe;
			port->c_src.close = callback->unsubscribe;
			port->c_dest.open = callback->use;
			port->c_dest.close = callback->unuse;
		}
	}

	info->addr = port->addr;

	snd_seq_set_port_info(port, info);
	snd_seq_system_client_ev_port_start(port->addr.client, port->addr.port);
	snd_seq_port_unlock(port);

	return 0;
}

/* 
 * DELETE PORT ioctl() 
 */
static int snd_seq_ioctl_delete_port(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_port_info *info = arg;
	int err;

	/* it is not allowed to remove the port for an another client */
	if (info->addr.client != client->number)
		return -EPERM;

	err = snd_seq_delete_port(client, info->addr.port);
	if (err >= 0)
		snd_seq_system_client_ev_port_exit(client->number, info->addr.port);
	return err;
}


/* 
 * GET_PORT_INFO ioctl() (on any client) 
 */
static int snd_seq_ioctl_get_port_info(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_port_info *info = arg;
	struct snd_seq_client *cptr;
	struct snd_seq_client_port *port;

	cptr = snd_seq_client_use_ptr(info->addr.client);
	if (cptr == NULL)
		return -ENXIO;

	port = snd_seq_port_use_ptr(cptr, info->addr.port);
	if (port == NULL) {
		snd_seq_client_unlock(cptr);
		return -ENOENT;			/* don't change */
	}

	/* get port info */
	snd_seq_get_port_info(port, info);
	snd_seq_port_unlock(port);
	snd_seq_client_unlock(cptr);

	return 0;
}


/* 
 * SET_PORT_INFO ioctl() (only ports on this/own client) 
 */
static int snd_seq_ioctl_set_port_info(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_port_info *info = arg;
	struct snd_seq_client_port *port;

	if (info->addr.client != client->number) /* only set our own ports ! */
		return -EPERM;
	port = snd_seq_port_use_ptr(client, info->addr.port);
	if (port) {
		snd_seq_set_port_info(port, info);
		snd_seq_port_unlock(port);
	}
	return 0;
}


/*
 * port subscription (connection)
 */
#define PERM_RD		(SNDRV_SEQ_PORT_CAP_READ|SNDRV_SEQ_PORT_CAP_SUBS_READ)
#define PERM_WR		(SNDRV_SEQ_PORT_CAP_WRITE|SNDRV_SEQ_PORT_CAP_SUBS_WRITE)

static int check_subscription_permission(struct snd_seq_client *client,
					 struct snd_seq_client_port *sport,
					 struct snd_seq_client_port *dport,
					 struct snd_seq_port_subscribe *subs)
{
	if (client->number != subs->sender.client &&
	    client->number != subs->dest.client) {
		/* connection by third client - check export permission */
		if (check_port_perm(sport, SNDRV_SEQ_PORT_CAP_NO_EXPORT))
			return -EPERM;
		if (check_port_perm(dport, SNDRV_SEQ_PORT_CAP_NO_EXPORT))
			return -EPERM;
	}

	/* check read permission */
	/* if sender or receiver is the subscribing client itself,
	 * no permission check is necessary
	 */
	if (client->number != subs->sender.client) {
		if (! check_port_perm(sport, PERM_RD))
			return -EPERM;
	}
	/* check write permission */
	if (client->number != subs->dest.client) {
		if (! check_port_perm(dport, PERM_WR))
			return -EPERM;
	}
	return 0;
}

/*
 * send an subscription notify event to user client:
 * client must be user client.
 */
int snd_seq_client_notify_subscription(int client, int port,
				       struct snd_seq_port_subscribe *info,
				       int evtype)
{
	struct snd_seq_event event;

	memset(&event, 0, sizeof(event));
	event.type = evtype;
	event.data.connect.dest = info->dest;
	event.data.connect.sender = info->sender;

	return snd_seq_system_notify(client, port, &event);  /* non-atomic */
}


/* 
 * add to port's subscription list IOCTL interface 
 */
static int snd_seq_ioctl_subscribe_port(struct snd_seq_client *client,
					void *arg)
{
	struct snd_seq_port_subscribe *subs = arg;
	int result = -EINVAL;
	struct snd_seq_client *receiver = NULL, *sender = NULL;
	struct snd_seq_client_port *sport = NULL, *dport = NULL;

	if ((receiver = snd_seq_client_use_ptr(subs->dest.client)) == NULL)
		goto __end;
	if ((sender = snd_seq_client_use_ptr(subs->sender.client)) == NULL)
		goto __end;
	if ((sport = snd_seq_port_use_ptr(sender, subs->sender.port)) == NULL)
		goto __end;
	if ((dport = snd_seq_port_use_ptr(receiver, subs->dest.port)) == NULL)
		goto __end;

	result = check_subscription_permission(client, sport, dport, subs);
	if (result < 0)
		goto __end;

	/* connect them */
	result = snd_seq_port_connect(client, sender, sport, receiver, dport, subs);
	if (! result) /* broadcast announce */
		snd_seq_client_notify_subscription(SNDRV_SEQ_ADDRESS_SUBSCRIBERS, 0,
						   subs, SNDRV_SEQ_EVENT_PORT_SUBSCRIBED);
      __end:
      	if (sport)
		snd_seq_port_unlock(sport);
	if (dport)
		snd_seq_port_unlock(dport);
	if (sender)
		snd_seq_client_unlock(sender);
	if (receiver)
		snd_seq_client_unlock(receiver);
	return result;
}


/* 
 * remove from port's subscription list 
 */
static int snd_seq_ioctl_unsubscribe_port(struct snd_seq_client *client,
					  void *arg)
{
	struct snd_seq_port_subscribe *subs = arg;
	int result = -ENXIO;
	struct snd_seq_client *receiver = NULL, *sender = NULL;
	struct snd_seq_client_port *sport = NULL, *dport = NULL;

	if ((receiver = snd_seq_client_use_ptr(subs->dest.client)) == NULL)
		goto __end;
	if ((sender = snd_seq_client_use_ptr(subs->sender.client)) == NULL)
		goto __end;
	if ((sport = snd_seq_port_use_ptr(sender, subs->sender.port)) == NULL)
		goto __end;
	if ((dport = snd_seq_port_use_ptr(receiver, subs->dest.port)) == NULL)
		goto __end;

	result = check_subscription_permission(client, sport, dport, subs);
	if (result < 0)
		goto __end;

	result = snd_seq_port_disconnect(client, sender, sport, receiver, dport, subs);
	if (! result) /* broadcast announce */
		snd_seq_client_notify_subscription(SNDRV_SEQ_ADDRESS_SUBSCRIBERS, 0,
						   subs, SNDRV_SEQ_EVENT_PORT_UNSUBSCRIBED);
      __end:
      	if (sport)
		snd_seq_port_unlock(sport);
	if (dport)
		snd_seq_port_unlock(dport);
	if (sender)
		snd_seq_client_unlock(sender);
	if (receiver)
		snd_seq_client_unlock(receiver);
	return result;
}


/* CREATE_QUEUE ioctl() */
static int snd_seq_ioctl_create_queue(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_queue_info *info = arg;
	struct snd_seq_queue *q;

	q = snd_seq_queue_alloc(client->number, info->locked, info->flags);
	if (IS_ERR(q))
		return PTR_ERR(q);

	info->queue = q->queue;
	info->locked = q->locked;
	info->owner = q->owner;

	/* set queue name */
	if (!info->name[0])
		snprintf(info->name, sizeof(info->name), "Queue-%d", q->queue);
	strlcpy(q->name, info->name, sizeof(q->name));
	snd_use_lock_free(&q->use_lock);

	return 0;
}

/* DELETE_QUEUE ioctl() */
static int snd_seq_ioctl_delete_queue(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_queue_info *info = arg;

	return snd_seq_queue_delete(client->number, info->queue);
}

/* GET_QUEUE_INFO ioctl() */
static int snd_seq_ioctl_get_queue_info(struct snd_seq_client *client,
					void *arg)
{
	struct snd_seq_queue_info *info = arg;
	struct snd_seq_queue *q;

	q = queueptr(info->queue);
	if (q == NULL)
		return -EINVAL;

	memset(info, 0, sizeof(*info));
	info->queue = q->queue;
	info->owner = q->owner;
	info->locked = q->locked;
	strlcpy(info->name, q->name, sizeof(info->name));
	queuefree(q);

	return 0;
}

/* SET_QUEUE_INFO ioctl() */
static int snd_seq_ioctl_set_queue_info(struct snd_seq_client *client,
					void *arg)
{
	struct snd_seq_queue_info *info = arg;
	struct snd_seq_queue *q;

	if (info->owner != client->number)
		return -EINVAL;

	/* change owner/locked permission */
	if (snd_seq_queue_check_access(info->queue, client->number)) {
		if (snd_seq_queue_set_owner(info->queue, client->number, info->locked) < 0)
			return -EPERM;
		if (info->locked)
			snd_seq_queue_use(info->queue, client->number, 1);
	} else {
		return -EPERM;
	}	

	q = queueptr(info->queue);
	if (! q)
		return -EINVAL;
	if (q->owner != client->number) {
		queuefree(q);
		return -EPERM;
	}
	strlcpy(q->name, info->name, sizeof(q->name));
	queuefree(q);

	return 0;
}

/* GET_NAMED_QUEUE ioctl() */
static int snd_seq_ioctl_get_named_queue(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_queue_info *info = arg;
	struct snd_seq_queue *q;

	q = snd_seq_queue_find_name(info->name);
	if (q == NULL)
		return -EINVAL;
	info->queue = q->queue;
	info->owner = q->owner;
	info->locked = q->locked;
	queuefree(q);

	return 0;
}

/* GET_QUEUE_STATUS ioctl() */
static int snd_seq_ioctl_get_queue_status(struct snd_seq_client *client,
					  void *arg)
{
	struct snd_seq_queue_status *status = arg;
	struct snd_seq_queue *queue;
	struct snd_seq_timer *tmr;

	queue = queueptr(status->queue);
	if (queue == NULL)
		return -EINVAL;
	memset(status, 0, sizeof(*status));
	status->queue = queue->queue;
	
	tmr = queue->timer;
	status->events = queue->tickq->cells + queue->timeq->cells;

	status->time = snd_seq_timer_get_cur_time(tmr);
	status->tick = snd_seq_timer_get_cur_tick(tmr);

	status->running = tmr->running;

	status->flags = queue->flags;
	queuefree(queue);

	return 0;
}


/* GET_QUEUE_TEMPO ioctl() */
static int snd_seq_ioctl_get_queue_tempo(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_queue_tempo *tempo = arg;
	struct snd_seq_queue *queue;
	struct snd_seq_timer *tmr;

	queue = queueptr(tempo->queue);
	if (queue == NULL)
		return -EINVAL;
	memset(tempo, 0, sizeof(*tempo));
	tempo->queue = queue->queue;
	
	tmr = queue->timer;

	tempo->tempo = tmr->tempo;
	tempo->ppq = tmr->ppq;
	tempo->skew_value = tmr->skew;
	tempo->skew_base = tmr->skew_base;
	queuefree(queue);

	return 0;
}


/* SET_QUEUE_TEMPO ioctl() */
int snd_seq_set_queue_tempo(int client, struct snd_seq_queue_tempo *tempo)
{
	if (!snd_seq_queue_check_access(tempo->queue, client))
		return -EPERM;
	return snd_seq_queue_timer_set_tempo(tempo->queue, client, tempo);
}
EXPORT_SYMBOL(snd_seq_set_queue_tempo);

static int snd_seq_ioctl_set_queue_tempo(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_queue_tempo *tempo = arg;
	int result;

	result = snd_seq_set_queue_tempo(client->number, tempo);
	return result < 0 ? result : 0;
}


/* GET_QUEUE_TIMER ioctl() */
static int snd_seq_ioctl_get_queue_timer(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_queue_timer *timer = arg;
	struct snd_seq_queue *queue;
	struct snd_seq_timer *tmr;

	queue = queueptr(timer->queue);
	if (queue == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&queue->timer_mutex)) {
		queuefree(queue);
		return -ERESTARTSYS;
	}
	tmr = queue->timer;
	memset(timer, 0, sizeof(*timer));
	timer->queue = queue->queue;

	timer->type = tmr->type;
	if (tmr->type == SNDRV_SEQ_TIMER_ALSA) {
		timer->u.alsa.id = tmr->alsa_id;
		timer->u.alsa.resolution = tmr->preferred_resolution;
	}
	mutex_unlock(&queue->timer_mutex);
	queuefree(queue);
	
	return 0;
}


/* SET_QUEUE_TIMER ioctl() */
static int snd_seq_ioctl_set_queue_timer(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_queue_timer *timer = arg;
	int result = 0;

	if (timer->type != SNDRV_SEQ_TIMER_ALSA)
		return -EINVAL;

	if (snd_seq_queue_check_access(timer->queue, client->number)) {
		struct snd_seq_queue *q;
		struct snd_seq_timer *tmr;

		q = queueptr(timer->queue);
		if (q == NULL)
			return -ENXIO;
		if (mutex_lock_interruptible(&q->timer_mutex)) {
			queuefree(q);
			return -ERESTARTSYS;
		}
		tmr = q->timer;
		snd_seq_queue_timer_close(timer->queue);
		tmr->type = timer->type;
		if (tmr->type == SNDRV_SEQ_TIMER_ALSA) {
			tmr->alsa_id = timer->u.alsa.id;
			tmr->preferred_resolution = timer->u.alsa.resolution;
		}
		result = snd_seq_queue_timer_open(timer->queue);
		mutex_unlock(&q->timer_mutex);
		queuefree(q);
	} else {
		return -EPERM;
	}	

	return result;
}


/* GET_QUEUE_CLIENT ioctl() */
static int snd_seq_ioctl_get_queue_client(struct snd_seq_client *client,
					  void *arg)
{
	struct snd_seq_queue_client *info = arg;
	int used;

	used = snd_seq_queue_is_used(info->queue, client->number);
	if (used < 0)
		return -EINVAL;
	info->used = used;
	info->client = client->number;

	return 0;
}


/* SET_QUEUE_CLIENT ioctl() */
static int snd_seq_ioctl_set_queue_client(struct snd_seq_client *client,
					  void *arg)
{
	struct snd_seq_queue_client *info = arg;
	int err;

	if (info->used >= 0) {
		err = snd_seq_queue_use(info->queue, client->number, info->used);
		if (err < 0)
			return err;
	}

	return snd_seq_ioctl_get_queue_client(client, arg);
}


/* GET_CLIENT_POOL ioctl() */
static int snd_seq_ioctl_get_client_pool(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_pool *info = arg;
	struct snd_seq_client *cptr;

	cptr = snd_seq_client_use_ptr(info->client);
	if (cptr == NULL)
		return -ENOENT;
	memset(info, 0, sizeof(*info));
	info->client = cptr->number;
	info->output_pool = cptr->pool->size;
	info->output_room = cptr->pool->room;
	info->output_free = info->output_pool;
	info->output_free = snd_seq_unused_cells(cptr->pool);
	if (cptr->type == USER_CLIENT) {
		info->input_pool = cptr->data.user.fifo_pool_size;
		info->input_free = info->input_pool;
		if (cptr->data.user.fifo)
			info->input_free = snd_seq_unused_cells(cptr->data.user.fifo->pool);
	} else {
		info->input_pool = 0;
		info->input_free = 0;
	}
	snd_seq_client_unlock(cptr);
	
	return 0;
}

/* SET_CLIENT_POOL ioctl() */
static int snd_seq_ioctl_set_client_pool(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_pool *info = arg;
	int rc;

	if (client->number != info->client)
		return -EINVAL; /* can't change other clients */

	if (info->output_pool >= 1 && info->output_pool <= SNDRV_SEQ_MAX_EVENTS &&
	    (! snd_seq_write_pool_allocated(client) ||
	     info->output_pool != client->pool->size)) {
		if (snd_seq_write_pool_allocated(client)) {
			/* is the pool in use? */
			if (atomic_read(&client->pool->counter))
				return -EBUSY;
			/* remove all existing cells */
			snd_seq_pool_mark_closing(client->pool);
			snd_seq_queue_client_leave_cells(client->number);
			snd_seq_pool_done(client->pool);
		}
		client->pool->size = info->output_pool;
		rc = snd_seq_pool_init(client->pool);
		if (rc < 0)
			return rc;
	}
	if (client->type == USER_CLIENT && client->data.user.fifo != NULL &&
	    info->input_pool >= 1 &&
	    info->input_pool <= SNDRV_SEQ_MAX_CLIENT_EVENTS &&
	    info->input_pool != client->data.user.fifo_pool_size) {
		/* change pool size */
		rc = snd_seq_fifo_resize(client->data.user.fifo, info->input_pool);
		if (rc < 0)
			return rc;
		client->data.user.fifo_pool_size = info->input_pool;
	}
	if (info->output_room >= 1 &&
	    info->output_room <= client->pool->size) {
		client->pool->room  = info->output_room;
	}

	return snd_seq_ioctl_get_client_pool(client, arg);
}


/* REMOVE_EVENTS ioctl() */
static int snd_seq_ioctl_remove_events(struct snd_seq_client *client,
				       void *arg)
{
	struct snd_seq_remove_events *info = arg;

	/*
	 * Input mostly not implemented XXX.
	 */
	if (info->remove_mode & SNDRV_SEQ_REMOVE_INPUT) {
		/*
		 * No restrictions so for a user client we can clear
		 * the whole fifo
		 */
		if (client->type == USER_CLIENT && client->data.user.fifo)
			snd_seq_fifo_clear(client->data.user.fifo);
	}

	if (info->remove_mode & SNDRV_SEQ_REMOVE_OUTPUT)
		snd_seq_queue_remove_cells(client->number, info);

	return 0;
}


/*
 * get subscription info
 */
static int snd_seq_ioctl_get_subscription(struct snd_seq_client *client,
					  void *arg)
{
	struct snd_seq_port_subscribe *subs = arg;
	int result;
	struct snd_seq_client *sender = NULL;
	struct snd_seq_client_port *sport = NULL;
	struct snd_seq_subscribers *p;

	result = -EINVAL;
	if ((sender = snd_seq_client_use_ptr(subs->sender.client)) == NULL)
		goto __end;
	if ((sport = snd_seq_port_use_ptr(sender, subs->sender.port)) == NULL)
		goto __end;
	p = snd_seq_port_get_subscription(&sport->c_src, &subs->dest);
	if (p) {
		result = 0;
		*subs = p->info;
	} else
		result = -ENOENT;

      __end:
      	if (sport)
		snd_seq_port_unlock(sport);
	if (sender)
		snd_seq_client_unlock(sender);

	return result;
}


/*
 * get subscription info - check only its presence
 */
static int snd_seq_ioctl_query_subs(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_query_subs *subs = arg;
	int result = -ENXIO;
	struct snd_seq_client *cptr = NULL;
	struct snd_seq_client_port *port = NULL;
	struct snd_seq_port_subs_info *group;
	struct list_head *p;
	int i;

	if ((cptr = snd_seq_client_use_ptr(subs->root.client)) == NULL)
		goto __end;
	if ((port = snd_seq_port_use_ptr(cptr, subs->root.port)) == NULL)
		goto __end;

	switch (subs->type) {
	case SNDRV_SEQ_QUERY_SUBS_READ:
		group = &port->c_src;
		break;
	case SNDRV_SEQ_QUERY_SUBS_WRITE:
		group = &port->c_dest;
		break;
	default:
		goto __end;
	}

	down_read(&group->list_mutex);
	/* search for the subscriber */
	subs->num_subs = group->count;
	i = 0;
	result = -ENOENT;
	list_for_each(p, &group->list_head) {
		if (i++ == subs->index) {
			/* found! */
			struct snd_seq_subscribers *s;
			if (subs->type == SNDRV_SEQ_QUERY_SUBS_READ) {
				s = list_entry(p, struct snd_seq_subscribers, src_list);
				subs->addr = s->info.dest;
			} else {
				s = list_entry(p, struct snd_seq_subscribers, dest_list);
				subs->addr = s->info.sender;
			}
			subs->flags = s->info.flags;
			subs->queue = s->info.queue;
			result = 0;
			break;
		}
	}
	up_read(&group->list_mutex);

      __end:
   	if (port)
		snd_seq_port_unlock(port);
	if (cptr)
		snd_seq_client_unlock(cptr);

	return result;
}


/*
 * query next client
 */
static int snd_seq_ioctl_query_next_client(struct snd_seq_client *client,
					   void *arg)
{
	struct snd_seq_client_info *info = arg;
	struct snd_seq_client *cptr = NULL;

	/* search for next client */
	info->client++;
	if (info->client < 0)
		info->client = 0;
	for (; info->client < SNDRV_SEQ_MAX_CLIENTS; info->client++) {
		cptr = snd_seq_client_use_ptr(info->client);
		if (cptr)
			break; /* found */
	}
	if (cptr == NULL)
		return -ENOENT;

	get_client_info(cptr, info);
	snd_seq_client_unlock(cptr);

	return 0;
}

/* 
 * query next port
 */
static int snd_seq_ioctl_query_next_port(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_port_info *info = arg;
	struct snd_seq_client *cptr;
	struct snd_seq_client_port *port = NULL;

	cptr = snd_seq_client_use_ptr(info->addr.client);
	if (cptr == NULL)
		return -ENXIO;

	/* search for next port */
	info->addr.port++;
	port = snd_seq_port_query_nearest(cptr, info);
	if (port == NULL) {
		snd_seq_client_unlock(cptr);
		return -ENOENT;
	}

	/* get port info */
	info->addr = port->addr;
	snd_seq_get_port_info(port, info);
	snd_seq_port_unlock(port);
	snd_seq_client_unlock(cptr);

	return 0;
}

/* -------------------------------------------------------- */

static const struct ioctl_handler {
	unsigned int cmd;
	int (*func)(struct snd_seq_client *client, void *arg);
} ioctl_handlers[] = {
	{ SNDRV_SEQ_IOCTL_PVERSION, snd_seq_ioctl_pversion },
	{ SNDRV_SEQ_IOCTL_CLIENT_ID, snd_seq_ioctl_client_id },
	{ SNDRV_SEQ_IOCTL_SYSTEM_INFO, snd_seq_ioctl_system_info },
	{ SNDRV_SEQ_IOCTL_RUNNING_MODE, snd_seq_ioctl_running_mode },
	{ SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, snd_seq_ioctl_get_client_info },
	{ SNDRV_SEQ_IOCTL_SET_CLIENT_INFO, snd_seq_ioctl_set_client_info },
	{ SNDRV_SEQ_IOCTL_CREATE_PORT, snd_seq_ioctl_create_port },
	{ SNDRV_SEQ_IOCTL_DELETE_PORT, snd_seq_ioctl_delete_port },
	{ SNDRV_SEQ_IOCTL_GET_PORT_INFO, snd_seq_ioctl_get_port_info },
	{ SNDRV_SEQ_IOCTL_SET_PORT_INFO, snd_seq_ioctl_set_port_info },
	{ SNDRV_SEQ_IOCTL_SUBSCRIBE_PORT, snd_seq_ioctl_subscribe_port },
	{ SNDRV_SEQ_IOCTL_UNSUBSCRIBE_PORT, snd_seq_ioctl_unsubscribe_port },
	{ SNDRV_SEQ_IOCTL_CREATE_QUEUE, snd_seq_ioctl_create_queue },
	{ SNDRV_SEQ_IOCTL_DELETE_QUEUE, snd_seq_ioctl_delete_queue },
	{ SNDRV_SEQ_IOCTL_GET_QUEUE_INFO, snd_seq_ioctl_get_queue_info },
	{ SNDRV_SEQ_IOCTL_SET_QUEUE_INFO, snd_seq_ioctl_set_queue_info },
	{ SNDRV_SEQ_IOCTL_GET_NAMED_QUEUE, snd_seq_ioctl_get_named_queue },
	{ SNDRV_SEQ_IOCTL_GET_QUEUE_STATUS, snd_seq_ioctl_get_queue_status },
	{ SNDRV_SEQ_IOCTL_GET_QUEUE_TEMPO, snd_seq_ioctl_get_queue_tempo },
	{ SNDRV_SEQ_IOCTL_SET_QUEUE_TEMPO, snd_seq_ioctl_set_queue_tempo },
	{ SNDRV_SEQ_IOCTL_GET_QUEUE_TIMER, snd_seq_ioctl_get_queue_timer },
	{ SNDRV_SEQ_IOCTL_SET_QUEUE_TIMER, snd_seq_ioctl_set_queue_timer },
	{ SNDRV_SEQ_IOCTL_GET_QUEUE_CLIENT, snd_seq_ioctl_get_queue_client },
	{ SNDRV_SEQ_IOCTL_SET_QUEUE_CLIENT, snd_seq_ioctl_set_queue_client },
	{ SNDRV_SEQ_IOCTL_GET_CLIENT_POOL, snd_seq_ioctl_get_client_pool },
	{ SNDRV_SEQ_IOCTL_SET_CLIENT_POOL, snd_seq_ioctl_set_client_pool },
	{ SNDRV_SEQ_IOCTL_GET_SUBSCRIPTION, snd_seq_ioctl_get_subscription },
	{ SNDRV_SEQ_IOCTL_QUERY_NEXT_CLIENT, snd_seq_ioctl_query_next_client },
	{ SNDRV_SEQ_IOCTL_QUERY_NEXT_PORT, snd_seq_ioctl_query_next_port },
	{ SNDRV_SEQ_IOCTL_REMOVE_EVENTS, snd_seq_ioctl_remove_events },
	{ SNDRV_SEQ_IOCTL_QUERY_SUBS, snd_seq_ioctl_query_subs },
	{ 0, NULL },
};

static long snd_seq_ioctl(struct file *file, unsigned int cmd,
			  unsigned long arg)
{
	struct snd_seq_client *client = file->private_data;
	/* To use kernel stack for ioctl data. */
	union {
		int pversion;
		int client_id;
		struct snd_seq_system_info	system_info;
		struct snd_seq_running_info	running_info;
		struct snd_seq_client_info	client_info;
		struct snd_seq_port_info	port_info;
		struct snd_seq_port_subscribe	port_subscribe;
		struct snd_seq_queue_info	queue_info;
		struct snd_seq_queue_status	queue_status;
		struct snd_seq_queue_tempo	tempo;
		struct snd_seq_queue_timer	queue_timer;
		struct snd_seq_queue_client	queue_client;
		struct snd_seq_client_pool	client_pool;
		struct snd_seq_remove_events	remove_events;
		struct snd_seq_query_subs	query_subs;
	} buf;
	const struct ioctl_handler *handler;
	unsigned long size;
	int err;

	if (snd_BUG_ON(!client))
		return -ENXIO;

	for (handler = ioctl_handlers; handler->cmd > 0; ++handler) {
		if (handler->cmd == cmd)
			break;
	}
	if (handler->cmd == 0)
		return -ENOTTY;

	memset(&buf, 0, sizeof(buf));

	/*
	 * All of ioctl commands for ALSA sequencer get an argument of size
	 * within 13 bits. We can safely pick up the size from the command.
	 */
	size = _IOC_SIZE(handler->cmd);
	if (handler->cmd & IOC_IN) {
		if (copy_from_user(&buf, (const void __user *)arg, size))
			return -EFAULT;
	}

	mutex_lock(&client->ioctl_mutex);
	err = handler->func(client, &buf);
	mutex_unlock(&client->ioctl_mutex);
	if (err >= 0) {
		/* Some commands includes a bug in 'dir' field. */
		if (handler->cmd == SNDRV_SEQ_IOCTL_SET_QUEUE_CLIENT ||
		    handler->cmd == SNDRV_SEQ_IOCTL_SET_CLIENT_POOL ||
		    (handler->cmd & IOC_OUT))
			if (copy_to_user((void __user *)arg, &buf, size))
				return -EFAULT;
	}

	return err;
}

#ifdef CONFIG_COMPAT
#include "seq_compat.c"
#else
#define snd_seq_ioctl_compat	NULL
#endif

/* -------------------------------------------------------- */


/* exported to kernel modules */
int snd_seq_create_kernel_client(struct snd_card *card, int client_index,
				 const char *name_fmt, ...)
{
	struct snd_seq_client *client;
	va_list args;

	if (snd_BUG_ON(in_interrupt()))
		return -EBUSY;

	if (card && client_index >= SNDRV_SEQ_CLIENTS_PER_CARD)
		return -EINVAL;
	if (card == NULL && client_index >= SNDRV_SEQ_GLOBAL_CLIENTS)
		return -EINVAL;

	if (mutex_lock_interruptible(&register_mutex))
		return -ERESTARTSYS;

	if (card) {
		client_index += SNDRV_SEQ_GLOBAL_CLIENTS
			+ card->number * SNDRV_SEQ_CLIENTS_PER_CARD;
		if (client_index >= SNDRV_SEQ_DYNAMIC_CLIENTS_BEGIN)
			client_index = -1;
	}

	/* empty write queue as default */
	client = seq_create_client1(client_index, 0);
	if (client == NULL) {
		mutex_unlock(&register_mutex);
		return -EBUSY;	/* failure code */
	}
	usage_alloc(&client_usage, 1);

	client->accept_input = 1;
	client->accept_output = 1;
	client->data.kernel.card = card;
		
	va_start(args, name_fmt);
	vsnprintf(client->name, sizeof(client->name), name_fmt, args);
	va_end(args);

	client->type = KERNEL_CLIENT;
	mutex_unlock(&register_mutex);

	/* make others aware this new client */
	snd_seq_system_client_ev_client_start(client->number);
	
	/* return client number to caller */
	return client->number;
}
EXPORT_SYMBOL(snd_seq_create_kernel_client);

/* exported to kernel modules */
int snd_seq_delete_kernel_client(int client)
{
	struct snd_seq_client *ptr;

	if (snd_BUG_ON(in_interrupt()))
		return -EBUSY;

	ptr = clientptr(client);
	if (ptr == NULL)
		return -EINVAL;

	seq_free_client(ptr);
	kfree(ptr);
	return 0;
}
EXPORT_SYMBOL(snd_seq_delete_kernel_client);

/* skeleton to enqueue event, called from snd_seq_kernel_client_enqueue
 * and snd_seq_kernel_client_enqueue_blocking
 */
static int kernel_client_enqueue(int client, struct snd_seq_event *ev,
				 struct file *file, int blocking,
				 int atomic, int hop)
{
	struct snd_seq_client *cptr;
	int result;

	if (snd_BUG_ON(!ev))
		return -EINVAL;

	if (ev->type == SNDRV_SEQ_EVENT_NONE)
		return 0; /* ignore this */
	if (ev->type == SNDRV_SEQ_EVENT_KERNEL_ERROR)
		return -EINVAL; /* quoted events can't be enqueued */

	/* fill in client number */
	ev->source.client = client;

	if (check_event_type_and_length(ev))
		return -EINVAL;

	cptr = snd_seq_client_use_ptr(client);
	if (cptr == NULL)
		return -EINVAL;
	
	if (! cptr->accept_output)
		result = -EPERM;
	else /* send it */
		result = snd_seq_client_enqueue_event(cptr, ev, file, blocking, atomic, hop);

	snd_seq_client_unlock(cptr);
	return result;
}

/*
 * exported, called by kernel clients to enqueue events (w/o blocking)
 *
 * RETURN VALUE: zero if succeed, negative if error
 */
int snd_seq_kernel_client_enqueue(int client, struct snd_seq_event * ev,
				  int atomic, int hop)
{
	return kernel_client_enqueue(client, ev, NULL, 0, atomic, hop);
}
EXPORT_SYMBOL(snd_seq_kernel_client_enqueue);

/*
 * exported, called by kernel clients to enqueue events (with blocking)
 *
 * RETURN VALUE: zero if succeed, negative if error
 */
int snd_seq_kernel_client_enqueue_blocking(int client, struct snd_seq_event * ev,
					   struct file *file,
					   int atomic, int hop)
{
	return kernel_client_enqueue(client, ev, file, 1, atomic, hop);
}
EXPORT_SYMBOL(snd_seq_kernel_client_enqueue_blocking);

/* 
 * exported, called by kernel clients to dispatch events directly to other
 * clients, bypassing the queues.  Event time-stamp will be updated.
 *
 * RETURN VALUE: negative = delivery failed,
 *		 zero, or positive: the number of delivered events
 */
int snd_seq_kernel_client_dispatch(int client, struct snd_seq_event * ev,
				   int atomic, int hop)
{
	struct snd_seq_client *cptr;
	int result;

	if (snd_BUG_ON(!ev))
		return -EINVAL;

	/* fill in client number */
	ev->queue = SNDRV_SEQ_QUEUE_DIRECT;
	ev->source.client = client;

	if (check_event_type_and_length(ev))
		return -EINVAL;

	cptr = snd_seq_client_use_ptr(client);
	if (cptr == NULL)
		return -EINVAL;

	if (!cptr->accept_output)
		result = -EPERM;
	else
		result = snd_seq_deliver_event(cptr, ev, atomic, hop);

	snd_seq_client_unlock(cptr);
	return result;
}
EXPORT_SYMBOL(snd_seq_kernel_client_dispatch);

/**
 * snd_seq_kernel_client_ctl - operate a command for a client with data in
 *			       kernel space.
 * @clientid:	A numerical ID for a client.
 * @cmd:	An ioctl(2) command for ALSA sequencer operation.
 * @arg:	A pointer to data in kernel space.
 *
 * Against its name, both kernel/application client can be handled by this
 * kernel API. A pointer of 'arg' argument should be in kernel space.
 *
 * Return: 0 at success. Negative error code at failure.
 */
int snd_seq_kernel_client_ctl(int clientid, unsigned int cmd, void *arg)
{
	const struct ioctl_handler *handler;
	struct snd_seq_client *client;

	client = clientptr(clientid);
	if (client == NULL)
		return -ENXIO;

	for (handler = ioctl_handlers; handler->cmd > 0; ++handler) {
		if (handler->cmd == cmd)
			return handler->func(client, arg);
	}

	pr_debug("ALSA: seq unknown ioctl() 0x%x (type='%c', number=0x%02x)\n",
		 cmd, _IOC_TYPE(cmd), _IOC_NR(cmd));
	return -ENOTTY;
}
EXPORT_SYMBOL(snd_seq_kernel_client_ctl);

/* exported (for OSS emulator) */
int snd_seq_kernel_client_write_poll(int clientid, struct file *file, poll_table *wait)
{
	struct snd_seq_client *client;

	client = clientptr(clientid);
	if (client == NULL)
		return -ENXIO;

	if (! snd_seq_write_pool_allocated(client))
		return 1;
	if (snd_seq_pool_poll_wait(client->pool, file, wait))
		return 1;
	return 0;
}
EXPORT_SYMBOL(snd_seq_kernel_client_write_poll);

/*---------------------------------------------------------------------------*/

#ifdef CONFIG_SND_PROC_FS
/*
 *  /proc interface
 */
static void snd_seq_info_dump_subscribers(struct snd_info_buffer *buffer,
					  struct snd_seq_port_subs_info *group,
					  int is_src, char *msg)
{
	struct list_head *p;
	struct snd_seq_subscribers *s;
	int count = 0;

	down_read(&group->list_mutex);
	if (list_empty(&group->list_head)) {
		up_read(&group->list_mutex);
		return;
	}
	snd_iprintf(buffer, msg);
	list_for_each(p, &group->list_head) {
		if (is_src)
			s = list_entry(p, struct snd_seq_subscribers, src_list);
		else
			s = list_entry(p, struct snd_seq_subscribers, dest_list);
		if (count++)
			snd_iprintf(buffer, ", ");
		snd_iprintf(buffer, "%d:%d",
			    is_src ? s->info.dest.client : s->info.sender.client,
			    is_src ? s->info.dest.port : s->info.sender.port);
		if (s->info.flags & SNDRV_SEQ_PORT_SUBS_TIMESTAMP)
			snd_iprintf(buffer, "[%c:%d]", ((s->info.flags & SNDRV_SEQ_PORT_SUBS_TIME_REAL) ? 'r' : 't'), s->info.queue);
		if (group->exclusive)
			snd_iprintf(buffer, "[ex]");
	}
	up_read(&group->list_mutex);
	snd_iprintf(buffer, "\n");
}

#define FLAG_PERM_RD(perm) ((perm) & SNDRV_SEQ_PORT_CAP_READ ? ((perm) & SNDRV_SEQ_PORT_CAP_SUBS_READ ? 'R' : 'r') : '-')
#define FLAG_PERM_WR(perm) ((perm) & SNDRV_SEQ_PORT_CAP_WRITE ? ((perm) & SNDRV_SEQ_PORT_CAP_SUBS_WRITE ? 'W' : 'w') : '-')
#define FLAG_PERM_EX(perm) ((perm) & SNDRV_SEQ_PORT_CAP_NO_EXPORT ? '-' : 'e')

#define FLAG_PERM_DUPLEX(perm) ((perm) & SNDRV_SEQ_PORT_CAP_DUPLEX ? 'X' : '-')

static void snd_seq_info_dump_ports(struct snd_info_buffer *buffer,
				    struct snd_seq_client *client)
{
	struct snd_seq_client_port *p;

	mutex_lock(&client->ports_mutex);
	list_for_each_entry(p, &client->ports_list_head, list) {
		snd_iprintf(buffer, "  Port %3d : \"%s\" (%c%c%c%c)\n",
			    p->addr.port, p->name,
			    FLAG_PERM_RD(p->capability),
			    FLAG_PERM_WR(p->capability),
			    FLAG_PERM_EX(p->capability),
			    FLAG_PERM_DUPLEX(p->capability));
		snd_seq_info_dump_subscribers(buffer, &p->c_src, 1, "    Connecting To: ");
		snd_seq_info_dump_subscribers(buffer, &p->c_dest, 0, "    Connected From: ");
	}
	mutex_unlock(&client->ports_mutex);
}


/* exported to seq_info.c */
void snd_seq_info_clients_read(struct snd_info_entry *entry, 
			       struct snd_info_buffer *buffer)
{
	int c;
	struct snd_seq_client *client;

	snd_iprintf(buffer, "Client info\n");
	snd_iprintf(buffer, "  cur  clients : %d\n", client_usage.cur);
	snd_iprintf(buffer, "  peak clients : %d\n", client_usage.peak);
	snd_iprintf(buffer, "  max  clients : %d\n", SNDRV_SEQ_MAX_CLIENTS);
	snd_iprintf(buffer, "\n");

	/* list the client table */
	for (c = 0; c < SNDRV_SEQ_MAX_CLIENTS; c++) {
		client = snd_seq_client_use_ptr(c);
		if (client == NULL)
			continue;
		if (client->type == NO_CLIENT) {
			snd_seq_client_unlock(client);
			continue;
		}

		snd_iprintf(buffer, "Client %3d : \"%s\" [%s]\n",
			    c, client->name,
			    client->type == USER_CLIENT ? "User" : "Kernel");
		snd_seq_info_dump_ports(buffer, client);
		if (snd_seq_write_pool_allocated(client)) {
			snd_iprintf(buffer, "  Output pool :\n");
			snd_seq_info_pool(buffer, client->pool, "    ");
		}
		if (client->type == USER_CLIENT && client->data.user.fifo &&
		    client->data.user.fifo->pool) {
			snd_iprintf(buffer, "  Input pool :\n");
			snd_seq_info_pool(buffer, client->data.user.fifo->pool, "    ");
		}
		snd_seq_client_unlock(client);
	}
}
#endif /* CONFIG_SND_PROC_FS */

/*---------------------------------------------------------------------------*/


/*
 *  REGISTRATION PART
 */

static const struct file_operations snd_seq_f_ops =
{
	.owner =	THIS_MODULE,
	.read =		snd_seq_read,
	.write =	snd_seq_write,
	.open =		snd_seq_open,
	.release =	snd_seq_release,
	.llseek =	no_llseek,
	.poll =		snd_seq_poll,
	.unlocked_ioctl =	snd_seq_ioctl,
	.compat_ioctl =	snd_seq_ioctl_compat,
};

static struct device seq_dev;

/* 
 * register sequencer device 
 */
int __init snd_sequencer_device_init(void)
{
	int err;

	snd_device_initialize(&seq_dev, NULL);
	dev_set_name(&seq_dev, "seq");

	if (mutex_lock_interruptible(&register_mutex))
		return -ERESTARTSYS;

	err = snd_register_device(SNDRV_DEVICE_TYPE_SEQUENCER, NULL, 0,
				  &snd_seq_f_ops, NULL, &seq_dev);
	if (err < 0) {
		mutex_unlock(&register_mutex);
		put_device(&seq_dev);
		return err;
	}
	
	mutex_unlock(&register_mutex);

	return 0;
}



/* 
 * unregister sequencer device 
 */
void __exit snd_sequencer_device_done(void)
{
	snd_unregister_device(&seq_dev);
	put_device(&seq_dev);
}
	if (snd_seq_ev_is_direct(event)) {
		if (event->type == SNDRV_SEQ_EVENT_NOTE)
			return -EINVAL; /* this event must be enqueued! */
		return snd_seq_deliver_event(client, event, atomic, hop);
	}

	/* Not direct, normal queuing */
	if (snd_seq_queue_is_used(event->queue, client->number) <= 0)
		return -EINVAL;  /* invalid queue */
	if (! snd_seq_write_pool_allocated(client))
		return -ENXIO; /* queue is not allocated */

	/* allocate an event cell */
	err = snd_seq_event_dup(client->pool, event, &cell, !blocking || atomic, file);
	if (err < 0)
		return err;

	/* we got a cell. enqueue it. */
	if ((err = snd_seq_enqueue_event(cell, atomic, hop)) < 0) {
		snd_seq_cell_free(cell);
		return err;
	}
{
	struct snd_seq_client *client = file->private_data;
	int written = 0, len;
	int err;
	struct snd_seq_event event;

	if (!(snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_OUTPUT))
		return -ENXIO;

	/* check client structures are in place */
	if (snd_BUG_ON(!client))
		return -ENXIO;
		
	if (!client->accept_output || client->pool == NULL)
		return -ENXIO;

	/* allocate the pool now if the pool is not allocated yet */ 
	if (client->pool->size > 0 && !snd_seq_write_pool_allocated(client)) {
		mutex_lock(&client->ioctl_mutex);
		err = snd_seq_pool_init(client->pool);
		mutex_unlock(&client->ioctl_mutex);
		if (err < 0)
			return -ENOMEM;
	}
			if (client->convert32 && snd_seq_ev_is_varusr(&event)) {
				void *ptr = (void __force *)compat_ptr(event.data.raw32.d[1]);
				event.data.ext.ptr = ptr;
			}
#endif
		}

		/* ok, enqueue it */
		err = snd_seq_client_enqueue_event(client, &event, file,
						   !(file->f_flags & O_NONBLOCK),
						   0, 0);
		if (err < 0)
			break;

	__skip_event:
		/* Update pointers and counts */
		count -= len;
		buf += len;
		written += len;
	}

	return written ? written : err;
}


/*
 * handle polling
 */
static __poll_t snd_seq_poll(struct file *file, poll_table * wait)
{
	struct snd_seq_client *client = file->private_data;
	__poll_t mask = 0;

	/* check client structures are in place */
	if (snd_BUG_ON(!client))
		return -ENXIO;

	if ((snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_INPUT) &&
	    client->data.user.fifo) {

		/* check if data is available in the outqueue */
		if (snd_seq_fifo_poll_wait(client->data.user.fifo, file, wait))
			mask |= EPOLLIN | EPOLLRDNORM;
	}

	if (snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_OUTPUT) {

		/* check if data is available in the pool */
		if (!snd_seq_write_pool_allocated(client) ||
		    snd_seq_pool_poll_wait(client->pool, file, wait))
			mask |= EPOLLOUT | EPOLLWRNORM;
	}

	return mask;
}


/*-----------------------------------------------------*/

static int snd_seq_ioctl_pversion(struct snd_seq_client *client, void *arg)
{
	int *pversion = arg;

	*pversion = SNDRV_SEQ_VERSION;
	return 0;
}

static int snd_seq_ioctl_client_id(struct snd_seq_client *client, void *arg)
{
	int *client_id = arg;

	*client_id = client->number;
	return 0;
}

/* SYSTEM_INFO ioctl() */
static int snd_seq_ioctl_system_info(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_system_info *info = arg;

	memset(info, 0, sizeof(*info));
	/* fill the info fields */
	info->queues = SNDRV_SEQ_MAX_QUEUES;
	info->clients = SNDRV_SEQ_MAX_CLIENTS;
	info->ports = SNDRV_SEQ_MAX_PORTS;
	info->channels = 256;	/* fixed limit */
	info->cur_clients = client_usage.cur;
	info->cur_queues = snd_seq_queue_get_cur_queues();

	return 0;
}


/* RUNNING_MODE ioctl() */
static int snd_seq_ioctl_running_mode(struct snd_seq_client *client, void  *arg)
{
	struct snd_seq_running_info *info = arg;
	struct snd_seq_client *cptr;
	int err = 0;

	/* requested client number */
	cptr = snd_seq_client_use_ptr(info->client);
	if (cptr == NULL)
		return -ENOENT;		/* don't change !!! */

#ifdef SNDRV_BIG_ENDIAN
	if (!info->big_endian) {
		err = -EINVAL;
		goto __err;
	}
#else
	if (info->big_endian) {
		err = -EINVAL;
		goto __err;
	}

#endif
	if (info->cpu_mode > sizeof(long)) {
		err = -EINVAL;
		goto __err;
	}
	cptr->convert32 = (info->cpu_mode < sizeof(long));
 __err:
	snd_seq_client_unlock(cptr);
	return err;
}

/* CLIENT_INFO ioctl() */
static void get_client_info(struct snd_seq_client *cptr,
			    struct snd_seq_client_info *info)
{
	info->client = cptr->number;

	/* fill the info fields */
	info->type = cptr->type;
	strcpy(info->name, cptr->name);
	info->filter = cptr->filter;
	info->event_lost = cptr->event_lost;
	memcpy(info->event_filter, cptr->event_filter, 32);
	info->num_ports = cptr->num_ports;

	if (cptr->type == USER_CLIENT)
		info->pid = pid_vnr(cptr->data.user.owner);
	else
		info->pid = -1;

	if (cptr->type == KERNEL_CLIENT)
		info->card = cptr->data.kernel.card ? cptr->data.kernel.card->number : -1;
	else
		info->card = -1;

	memset(info->reserved, 0, sizeof(info->reserved));
}

static int snd_seq_ioctl_get_client_info(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_info *client_info = arg;
	struct snd_seq_client *cptr;

	/* requested client number */
	cptr = snd_seq_client_use_ptr(client_info->client);
	if (cptr == NULL)
		return -ENOENT;		/* don't change !!! */

	get_client_info(cptr, client_info);
	snd_seq_client_unlock(cptr);

	return 0;
}


/* CLIENT_INFO ioctl() */
static int snd_seq_ioctl_set_client_info(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_info *client_info = arg;

	/* it is not allowed to set the info fields for an another client */
	if (client->number != client_info->client)
		return -EPERM;
	/* also client type must be set now */
	if (client->type != client_info->type)
		return -EINVAL;

	/* fill the info fields */
	if (client_info->name[0])
		strlcpy(client->name, client_info->name, sizeof(client->name));

	client->filter = client_info->filter;
	client->event_lost = client_info->event_lost;
	memcpy(client->event_filter, client_info->event_filter, 32);

	return 0;
}


/* 
 * CREATE PORT ioctl() 
 */
static int snd_seq_ioctl_create_port(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_port_info *info = arg;
	struct snd_seq_client_port *port;
	struct snd_seq_port_callback *callback;
	int port_idx;

	/* it is not allowed to create the port for an another client */
	if (info->addr.client != client->number)
		return -EPERM;

	port = snd_seq_create_port(client, (info->flags & SNDRV_SEQ_PORT_FLG_GIVEN_PORT) ? info->addr.port : -1);
	if (port == NULL)
		return -ENOMEM;

	if (client->type == USER_CLIENT && info->kernel) {
		port_idx = port->addr.port;
		snd_seq_port_unlock(port);
		snd_seq_delete_port(client, port_idx);
		return -EINVAL;
	}
	if (client->type == KERNEL_CLIENT) {
		if ((callback = info->kernel) != NULL) {
			if (callback->owner)
				port->owner = callback->owner;
			port->private_data = callback->private_data;
			port->private_free = callback->private_free;
			port->event_input = callback->event_input;
			port->c_src.open = callback->subscribe;
			port->c_src.close = callback->unsubscribe;
			port->c_dest.open = callback->use;
			port->c_dest.close = callback->unuse;
		}
			if (client->convert32 && snd_seq_ev_is_varusr(&event)) {
				void *ptr = (void __force *)compat_ptr(event.data.raw32.d[1]);
				event.data.ext.ptr = ptr;
			}
#endif
		}

		/* ok, enqueue it */
		err = snd_seq_client_enqueue_event(client, &event, file,
						   !(file->f_flags & O_NONBLOCK),
						   0, 0);
		if (err < 0)
			break;

	__skip_event:
		/* Update pointers and counts */
		count -= len;
		buf += len;
		written += len;
	}

	return written ? written : err;
}


/*
 * handle polling
 */
static __poll_t snd_seq_poll(struct file *file, poll_table * wait)
{
	struct snd_seq_client *client = file->private_data;
	__poll_t mask = 0;

	/* check client structures are in place */
	if (snd_BUG_ON(!client))
		return -ENXIO;

	if ((snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_INPUT) &&
	    client->data.user.fifo) {

		/* check if data is available in the outqueue */
		if (snd_seq_fifo_poll_wait(client->data.user.fifo, file, wait))
			mask |= EPOLLIN | EPOLLRDNORM;
	}

	if (snd_seq_file_flags(file) & SNDRV_SEQ_LFLG_OUTPUT) {

		/* check if data is available in the pool */
		if (!snd_seq_write_pool_allocated(client) ||
		    snd_seq_pool_poll_wait(client->pool, file, wait))
			mask |= EPOLLOUT | EPOLLWRNORM;
	}

	return mask;
}


/*-----------------------------------------------------*/

static int snd_seq_ioctl_pversion(struct snd_seq_client *client, void *arg)
{
	int *pversion = arg;

	*pversion = SNDRV_SEQ_VERSION;
	return 0;
}

static int snd_seq_ioctl_client_id(struct snd_seq_client *client, void *arg)
{
	int *client_id = arg;

	*client_id = client->number;
	return 0;
}

/* SYSTEM_INFO ioctl() */
static int snd_seq_ioctl_system_info(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_system_info *info = arg;

	memset(info, 0, sizeof(*info));
	/* fill the info fields */
	info->queues = SNDRV_SEQ_MAX_QUEUES;
	info->clients = SNDRV_SEQ_MAX_CLIENTS;
	info->ports = SNDRV_SEQ_MAX_PORTS;
	info->channels = 256;	/* fixed limit */
	info->cur_clients = client_usage.cur;
	info->cur_queues = snd_seq_queue_get_cur_queues();

	return 0;
}


/* RUNNING_MODE ioctl() */
static int snd_seq_ioctl_running_mode(struct snd_seq_client *client, void  *arg)
{
	struct snd_seq_running_info *info = arg;
	struct snd_seq_client *cptr;
	int err = 0;

	/* requested client number */
	cptr = snd_seq_client_use_ptr(info->client);
	if (cptr == NULL)
		return -ENOENT;		/* don't change !!! */

#ifdef SNDRV_BIG_ENDIAN
	if (!info->big_endian) {
		err = -EINVAL;
		goto __err;
	}
#else
	if (info->big_endian) {
		err = -EINVAL;
		goto __err;
	}

#endif
	if (info->cpu_mode > sizeof(long)) {
		err = -EINVAL;
		goto __err;
	}
	cptr->convert32 = (info->cpu_mode < sizeof(long));
 __err:
	snd_seq_client_unlock(cptr);
	return err;
}

/* CLIENT_INFO ioctl() */
static void get_client_info(struct snd_seq_client *cptr,
			    struct snd_seq_client_info *info)
{
	info->client = cptr->number;

	/* fill the info fields */
	info->type = cptr->type;
	strcpy(info->name, cptr->name);
	info->filter = cptr->filter;
	info->event_lost = cptr->event_lost;
	memcpy(info->event_filter, cptr->event_filter, 32);
	info->num_ports = cptr->num_ports;

	if (cptr->type == USER_CLIENT)
		info->pid = pid_vnr(cptr->data.user.owner);
	else
		info->pid = -1;

	if (cptr->type == KERNEL_CLIENT)
		info->card = cptr->data.kernel.card ? cptr->data.kernel.card->number : -1;
	else
		info->card = -1;

	memset(info->reserved, 0, sizeof(info->reserved));
}

static int snd_seq_ioctl_get_client_info(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_info *client_info = arg;
	struct snd_seq_client *cptr;

	/* requested client number */
	cptr = snd_seq_client_use_ptr(client_info->client);
	if (cptr == NULL)
		return -ENOENT;		/* don't change !!! */

	get_client_info(cptr, client_info);
	snd_seq_client_unlock(cptr);

	return 0;
}


/* CLIENT_INFO ioctl() */
static int snd_seq_ioctl_set_client_info(struct snd_seq_client *client,
					 void *arg)
{
	struct snd_seq_client_info *client_info = arg;

	/* it is not allowed to set the info fields for an another client */
	if (client->number != client_info->client)
		return -EPERM;
	/* also client type must be set now */
	if (client->type != client_info->type)
		return -EINVAL;

	/* fill the info fields */
	if (client_info->name[0])
		strlcpy(client->name, client_info->name, sizeof(client->name));

	client->filter = client_info->filter;
	client->event_lost = client_info->event_lost;
	memcpy(client->event_filter, client_info->event_filter, 32);

	return 0;
}


/* 
 * CREATE PORT ioctl() 
 */
static int snd_seq_ioctl_create_port(struct snd_seq_client *client, void *arg)
{
	struct snd_seq_port_info *info = arg;
	struct snd_seq_client_port *port;
	struct snd_seq_port_callback *callback;
	int port_idx;

	/* it is not allowed to create the port for an another client */
	if (info->addr.client != client->number)
		return -EPERM;

	port = snd_seq_create_port(client, (info->flags & SNDRV_SEQ_PORT_FLG_GIVEN_PORT) ? info->addr.port : -1);
	if (port == NULL)
		return -ENOMEM;

	if (client->type == USER_CLIENT && info->kernel) {
		port_idx = port->addr.port;
		snd_seq_port_unlock(port);
		snd_seq_delete_port(client, port_idx);
		return -EINVAL;
	}
	if (client->type == KERNEL_CLIENT) {
		if ((callback = info->kernel) != NULL) {
			if (callback->owner)
				port->owner = callback->owner;
			port->private_data = callback->private_data;
			port->private_free = callback->private_free;
			port->event_input = callback->event_input;
			port->c_src.open = callback->subscribe;
			port->c_src.close = callback->unsubscribe;
			port->c_dest.open = callback->use;
			port->c_dest.close = callback->unuse;
		}
{
	struct snd_seq_client *cptr;
	int result;

	if (snd_BUG_ON(!ev))
		return -EINVAL;

	if (ev->type == SNDRV_SEQ_EVENT_NONE)
		return 0; /* ignore this */
	if (ev->type == SNDRV_SEQ_EVENT_KERNEL_ERROR)
		return -EINVAL; /* quoted events can't be enqueued */

	/* fill in client number */
	ev->source.client = client;

	if (check_event_type_and_length(ev))
		return -EINVAL;

	cptr = snd_seq_client_use_ptr(client);
	if (cptr == NULL)
		return -EINVAL;
	
	if (! cptr->accept_output)
		result = -EPERM;
	else /* send it */
		result = snd_seq_client_enqueue_event(cptr, ev, file, blocking, atomic, hop);

	snd_seq_client_unlock(cptr);
	return result;
}

/*
 * exported, called by kernel clients to enqueue events (w/o blocking)
 *
 * RETURN VALUE: zero if succeed, negative if error
 */
int snd_seq_kernel_client_enqueue(int client, struct snd_seq_event * ev,
				  int atomic, int hop)
{
	return kernel_client_enqueue(client, ev, NULL, 0, atomic, hop);
}
EXPORT_SYMBOL(snd_seq_kernel_client_enqueue);

/*
 * exported, called by kernel clients to enqueue events (with blocking)
 *
 * RETURN VALUE: zero if succeed, negative if error
 */
int snd_seq_kernel_client_enqueue_blocking(int client, struct snd_seq_event * ev,
					   struct file *file,
					   int atomic, int hop)
{
	return kernel_client_enqueue(client, ev, file, 1, atomic, hop);
}
EXPORT_SYMBOL(snd_seq_kernel_client_enqueue_blocking);

/* 
 * exported, called by kernel clients to dispatch events directly to other
 * clients, bypassing the queues.  Event time-stamp will be updated.
 *
 * RETURN VALUE: negative = delivery failed,
 *		 zero, or positive: the number of delivered events
 */
int snd_seq_kernel_client_dispatch(int client, struct snd_seq_event * ev,
				   int atomic, int hop)
{
	struct snd_seq_client *cptr;
	int result;

	if (snd_BUG_ON(!ev))
		return -EINVAL;

	/* fill in client number */
	ev->queue = SNDRV_SEQ_QUEUE_DIRECT;
	ev->source.client = client;

	if (check_event_type_and_length(ev))
		return -EINVAL;

	cptr = snd_seq_client_use_ptr(client);
	if (cptr == NULL)
		return -EINVAL;

	if (!cptr->accept_output)
		result = -EPERM;
	else
		result = snd_seq_deliver_event(cptr, ev, atomic, hop);

	snd_seq_client_unlock(cptr);
	return result;
}
EXPORT_SYMBOL(snd_seq_kernel_client_dispatch);

/**
 * snd_seq_kernel_client_ctl - operate a command for a client with data in
 *			       kernel space.
 * @clientid:	A numerical ID for a client.
 * @cmd:	An ioctl(2) command for ALSA sequencer operation.
 * @arg:	A pointer to data in kernel space.
 *
 * Against its name, both kernel/application client can be handled by this
 * kernel API. A pointer of 'arg' argument should be in kernel space.
 *
 * Return: 0 at success. Negative error code at failure.
 */
int snd_seq_kernel_client_ctl(int clientid, unsigned int cmd, void *arg)
{
	const struct ioctl_handler *handler;
	struct snd_seq_client *client;

	client = clientptr(clientid);
	if (client == NULL)
		return -ENXIO;

	for (handler = ioctl_handlers; handler->cmd > 0; ++handler) {
		if (handler->cmd == cmd)
			return handler->func(client, arg);
	}