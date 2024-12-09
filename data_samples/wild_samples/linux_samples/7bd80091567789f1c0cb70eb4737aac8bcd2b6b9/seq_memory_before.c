	if (waitqueue_active(&pool->output_sleep)) {
		/* has enough space now? */
		if (snd_seq_output_ok(pool))
			wake_up(&pool->output_sleep);
	}
	spin_unlock_irqrestore(&pool->lock, flags);
}


/*
 * allocate an event cell.
 */
static int snd_seq_cell_alloc(struct snd_seq_pool *pool,
			      struct snd_seq_event_cell **cellp,
			      int nonblock, struct file *file)
{
	struct snd_seq_event_cell *cell;
	unsigned long flags;
	int err = -EAGAIN;
	wait_queue_entry_t wait;

	if (pool == NULL)
		return -EINVAL;

	*cellp = NULL;

	init_waitqueue_entry(&wait, current);
	spin_lock_irqsave(&pool->lock, flags);
	if (pool->ptr == NULL) {	/* not initialized */
		pr_debug("ALSA: seq: pool is not initialized\n");
		err = -EINVAL;
		goto __error;
	}
	while (pool->free == NULL && ! nonblock && ! pool->closing) {

		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&pool->output_sleep, &wait);
		spin_unlock_irq(&pool->lock);
		schedule();
		spin_lock_irq(&pool->lock);
		remove_wait_queue(&pool->output_sleep, &wait);
		/* interrupted? */
		if (signal_pending(current)) {
			err = -ERESTARTSYS;
			goto __error;
		}
	if (cell) {
		int used;
		pool->free = cell->next;
		atomic_inc(&pool->counter);
		used = atomic_read(&pool->counter);
		if (pool->max_used < used)
			pool->max_used = used;
		pool->event_alloc_success++;
		/* clear cell pointers */
		cell->next = NULL;
		err = 0;
	} else
		pool->event_alloc_failures++;
	*cellp = cell;

__error:
	spin_unlock_irqrestore(&pool->lock, flags);
	return err;
}


/*
 * duplicate the event to a cell.
 * if the event has external data, the data is decomposed to additional
 * cells.
 */
int snd_seq_event_dup(struct snd_seq_pool *pool, struct snd_seq_event *event,
		      struct snd_seq_event_cell **cellp, int nonblock,
		      struct file *file)
{
	int ncells, err;
	unsigned int extlen;
	struct snd_seq_event_cell *cell;

	*cellp = NULL;

	ncells = 0;
	extlen = 0;
	if (snd_seq_ev_is_variable(event)) {
		extlen = event->data.ext.len & ~SNDRV_SEQ_EXT_MASK;
		ncells = (extlen + sizeof(struct snd_seq_event) - 1) / sizeof(struct snd_seq_event);
	}
	if (ncells >= pool->total_elements)
		return -ENOMEM;

	err = snd_seq_cell_alloc(pool, &cell, nonblock, file);
	if (err < 0)
		return err;

	/* copy the event */
	cell->event = *event;

	/* decompose */
	if (snd_seq_ev_is_variable(event)) {
		int len = extlen;
		int is_chained = event->data.ext.len & SNDRV_SEQ_EXT_CHAINED;
		int is_usrptr = event->data.ext.len & SNDRV_SEQ_EXT_USRPTR;
		struct snd_seq_event_cell *src, *tmp, *tail;
		char *buf;

		cell->event.data.ext.len = extlen | SNDRV_SEQ_EXT_CHAINED;
		cell->event.data.ext.ptr = NULL;

		src = (struct snd_seq_event_cell *)event->data.ext.ptr;
		buf = (char *)event->data.ext.ptr;
		tail = NULL;

		while (ncells-- > 0) {
			int size = sizeof(struct snd_seq_event);
			if (len < size)
				size = len;
			err = snd_seq_cell_alloc(pool, &tmp, nonblock, file);
			if (err < 0)
				goto __error;
			if (cell->event.data.ext.ptr == NULL)
				cell->event.data.ext.ptr = tmp;
			if (tail)
				tail->next = tmp;
			tail = tmp;
			/* copy chunk */
			if (is_chained && src) {
				tmp->event = src->event;
				src = src->next;
			} else if (is_usrptr) {
				if (copy_from_user(&tmp->event, (char __force __user *)buf, size)) {
					err = -EFAULT;
					goto __error;
				}
			} else {
				memcpy(&tmp->event, buf, size);
			}
			buf += size;
			len -= size;
		}
	}

	*cellp = cell;
	return 0;

__error:
	snd_seq_cell_free(cell);
	return err;
}
  

/* poll wait */
int snd_seq_pool_poll_wait(struct snd_seq_pool *pool, struct file *file,
			   poll_table *wait)
{
	poll_wait(file, &pool->output_sleep, wait);
	return snd_seq_output_ok(pool);
}


/* allocate room specified number of events */
int snd_seq_pool_init(struct snd_seq_pool *pool)
{
	int cell;
	struct snd_seq_event_cell *cellptr;
	unsigned long flags;

	if (snd_BUG_ON(!pool))
		return -EINVAL;

	cellptr = vmalloc(sizeof(struct snd_seq_event_cell) * pool->size);
	if (!cellptr)
		return -ENOMEM;

	/* add new cells to the free cell list */
	spin_lock_irqsave(&pool->lock, flags);
	if (pool->ptr) {
		spin_unlock_irqrestore(&pool->lock, flags);
		vfree(cellptr);
		return 0;
	}

	pool->ptr = cellptr;
	pool->free = NULL;

	for (cell = 0; cell < pool->size; cell++) {
		cellptr = pool->ptr + cell;
		cellptr->pool = pool;
		cellptr->next = pool->free;
		pool->free = cellptr;
	}
	pool->room = (pool->size + 1) / 2;

	/* init statistics */
	pool->max_used = 0;
	pool->total_elements = pool->size;
	spin_unlock_irqrestore(&pool->lock, flags);
	return 0;
}

/* refuse the further insertion to the pool */
void snd_seq_pool_mark_closing(struct snd_seq_pool *pool)
{
	unsigned long flags;

	if (snd_BUG_ON(!pool))
		return;
	spin_lock_irqsave(&pool->lock, flags);
	pool->closing = 1;
	spin_unlock_irqrestore(&pool->lock, flags);
}

/* remove events */
int snd_seq_pool_done(struct snd_seq_pool *pool)
{
	unsigned long flags;
	struct snd_seq_event_cell *ptr;

	if (snd_BUG_ON(!pool))
		return -EINVAL;

	/* wait for closing all threads */
	if (waitqueue_active(&pool->output_sleep))
		wake_up(&pool->output_sleep);

	while (atomic_read(&pool->counter) > 0)
		schedule_timeout_uninterruptible(1);
	
	/* release all resources */
	spin_lock_irqsave(&pool->lock, flags);
	ptr = pool->ptr;
	pool->ptr = NULL;
	pool->free = NULL;
	pool->total_elements = 0;
	spin_unlock_irqrestore(&pool->lock, flags);

	vfree(ptr);

	spin_lock_irqsave(&pool->lock, flags);
	pool->closing = 0;
	spin_unlock_irqrestore(&pool->lock, flags);

	return 0;
}


/* init new memory pool */
struct snd_seq_pool *snd_seq_pool_new(int poolsize)
{
	struct snd_seq_pool *pool;

	/* create pool block */
	pool = kzalloc(sizeof(*pool), GFP_KERNEL);
	if (!pool)
		return NULL;
	spin_lock_init(&pool->lock);
	pool->ptr = NULL;
	pool->free = NULL;
	pool->total_elements = 0;
	atomic_set(&pool->counter, 0);
	pool->closing = 0;
	init_waitqueue_head(&pool->output_sleep);
	
	pool->size = poolsize;

	/* init statistics */
	pool->max_used = 0;
	return pool;
}

/* remove memory pool */
int snd_seq_pool_delete(struct snd_seq_pool **ppool)
{
	struct snd_seq_pool *pool = *ppool;

	*ppool = NULL;
	if (pool == NULL)
		return 0;
	snd_seq_pool_mark_closing(pool);
	snd_seq_pool_done(pool);
	kfree(pool);
	return 0;
}

/* initialize sequencer memory */
int __init snd_sequencer_memory_init(void)
{
	return 0;
}

/* release sequencer memory */
void __exit snd_sequencer_memory_done(void)
{
}


/* exported to seq_clientmgr.c */
void snd_seq_info_pool(struct snd_info_buffer *buffer,
		       struct snd_seq_pool *pool, char *space)
{
	if (pool == NULL)
		return;
	snd_iprintf(buffer, "%sPool size          : %d\n", space, pool->total_elements);
	snd_iprintf(buffer, "%sCells in use       : %d\n", space, atomic_read(&pool->counter));
	snd_iprintf(buffer, "%sPeak cells in use  : %d\n", space, pool->max_used);
	snd_iprintf(buffer, "%sAlloc success      : %d\n", space, pool->event_alloc_success);
	snd_iprintf(buffer, "%sAlloc failures     : %d\n", space, pool->event_alloc_failures);
}
	if (snd_seq_ev_is_variable(event)) {
		extlen = event->data.ext.len & ~SNDRV_SEQ_EXT_MASK;
		ncells = (extlen + sizeof(struct snd_seq_event) - 1) / sizeof(struct snd_seq_event);
	}
	if (ncells >= pool->total_elements)
		return -ENOMEM;

	err = snd_seq_cell_alloc(pool, &cell, nonblock, file);
	if (err < 0)
		return err;

	/* copy the event */
	cell->event = *event;

	/* decompose */
	if (snd_seq_ev_is_variable(event)) {
		int len = extlen;
		int is_chained = event->data.ext.len & SNDRV_SEQ_EXT_CHAINED;
		int is_usrptr = event->data.ext.len & SNDRV_SEQ_EXT_USRPTR;
		struct snd_seq_event_cell *src, *tmp, *tail;
		char *buf;

		cell->event.data.ext.len = extlen | SNDRV_SEQ_EXT_CHAINED;
		cell->event.data.ext.ptr = NULL;

		src = (struct snd_seq_event_cell *)event->data.ext.ptr;
		buf = (char *)event->data.ext.ptr;
		tail = NULL;

		while (ncells-- > 0) {
			int size = sizeof(struct snd_seq_event);
			if (len < size)
				size = len;
			err = snd_seq_cell_alloc(pool, &tmp, nonblock, file);
			if (err < 0)
				goto __error;
			if (cell->event.data.ext.ptr == NULL)
				cell->event.data.ext.ptr = tmp;
			if (tail)
				tail->next = tmp;
			tail = tmp;
			/* copy chunk */
			if (is_chained && src) {
				tmp->event = src->event;
				src = src->next;
			} else if (is_usrptr) {
				if (copy_from_user(&tmp->event, (char __force __user *)buf, size)) {
					err = -EFAULT;
					goto __error;
				}
			} else {
				memcpy(&tmp->event, buf, size);
			}
			buf += size;
			len -= size;
		}
	}

	*cellp = cell;
	return 0;

__error:
	snd_seq_cell_free(cell);
	return err;
}
  

/* poll wait */
int snd_seq_pool_poll_wait(struct snd_seq_pool *pool, struct file *file,
			   poll_table *wait)
{
	poll_wait(file, &pool->output_sleep, wait);
	return snd_seq_output_ok(pool);
}


/* allocate room specified number of events */
int snd_seq_pool_init(struct snd_seq_pool *pool)
{
	int cell;
	struct snd_seq_event_cell *cellptr;
	unsigned long flags;

	if (snd_BUG_ON(!pool))
		return -EINVAL;

	cellptr = vmalloc(sizeof(struct snd_seq_event_cell) * pool->size);
	if (!cellptr)
		return -ENOMEM;

	/* add new cells to the free cell list */
	spin_lock_irqsave(&pool->lock, flags);
	if (pool->ptr) {
		spin_unlock_irqrestore(&pool->lock, flags);
		vfree(cellptr);
		return 0;
	}

	pool->ptr = cellptr;
	pool->free = NULL;

	for (cell = 0; cell < pool->size; cell++) {
		cellptr = pool->ptr + cell;
		cellptr->pool = pool;
		cellptr->next = pool->free;
		pool->free = cellptr;
	}
	pool->room = (pool->size + 1) / 2;

	/* init statistics */
	pool->max_used = 0;
	pool->total_elements = pool->size;
	spin_unlock_irqrestore(&pool->lock, flags);
	return 0;
}

/* refuse the further insertion to the pool */
void snd_seq_pool_mark_closing(struct snd_seq_pool *pool)
{
	unsigned long flags;

	if (snd_BUG_ON(!pool))
		return;
	spin_lock_irqsave(&pool->lock, flags);
	pool->closing = 1;
	spin_unlock_irqrestore(&pool->lock, flags);
}

/* remove events */
int snd_seq_pool_done(struct snd_seq_pool *pool)
{
	unsigned long flags;
	struct snd_seq_event_cell *ptr;

	if (snd_BUG_ON(!pool))
		return -EINVAL;

	/* wait for closing all threads */
	if (waitqueue_active(&pool->output_sleep))
		wake_up(&pool->output_sleep);

	while (atomic_read(&pool->counter) > 0)
		schedule_timeout_uninterruptible(1);
	
	/* release all resources */
	spin_lock_irqsave(&pool->lock, flags);
	ptr = pool->ptr;
	pool->ptr = NULL;
	pool->free = NULL;
	pool->total_elements = 0;
	spin_unlock_irqrestore(&pool->lock, flags);

	vfree(ptr);

	spin_lock_irqsave(&pool->lock, flags);
	pool->closing = 0;
	spin_unlock_irqrestore(&pool->lock, flags);

	return 0;
}


/* init new memory pool */
struct snd_seq_pool *snd_seq_pool_new(int poolsize)
{
	struct snd_seq_pool *pool;

	/* create pool block */
	pool = kzalloc(sizeof(*pool), GFP_KERNEL);
	if (!pool)
		return NULL;
	spin_lock_init(&pool->lock);
	pool->ptr = NULL;
	pool->free = NULL;
	pool->total_elements = 0;
	atomic_set(&pool->counter, 0);
	pool->closing = 0;
	init_waitqueue_head(&pool->output_sleep);
	
	pool->size = poolsize;

	/* init statistics */
	pool->max_used = 0;
	return pool;
}

/* remove memory pool */
int snd_seq_pool_delete(struct snd_seq_pool **ppool)
{
	struct snd_seq_pool *pool = *ppool;

	*ppool = NULL;
	if (pool == NULL)
		return 0;
	snd_seq_pool_mark_closing(pool);
	snd_seq_pool_done(pool);
	kfree(pool);
	return 0;
}

/* initialize sequencer memory */
int __init snd_sequencer_memory_init(void)
{
	return 0;
}

/* release sequencer memory */
void __exit snd_sequencer_memory_done(void)
{
}


/* exported to seq_clientmgr.c */
void snd_seq_info_pool(struct snd_info_buffer *buffer,
		       struct snd_seq_pool *pool, char *space)
{
	if (pool == NULL)
		return;
	snd_iprintf(buffer, "%sPool size          : %d\n", space, pool->total_elements);
	snd_iprintf(buffer, "%sCells in use       : %d\n", space, atomic_read(&pool->counter));
	snd_iprintf(buffer, "%sPeak cells in use  : %d\n", space, pool->max_used);
	snd_iprintf(buffer, "%sAlloc success      : %d\n", space, pool->event_alloc_success);
	snd_iprintf(buffer, "%sAlloc failures     : %d\n", space, pool->event_alloc_failures);
}
		while (ncells-- > 0) {
			int size = sizeof(struct snd_seq_event);
			if (len < size)
				size = len;
			err = snd_seq_cell_alloc(pool, &tmp, nonblock, file);
			if (err < 0)
				goto __error;
			if (cell->event.data.ext.ptr == NULL)
				cell->event.data.ext.ptr = tmp;
			if (tail)
				tail->next = tmp;
			tail = tmp;
			/* copy chunk */
			if (is_chained && src) {
				tmp->event = src->event;
				src = src->next;
			} else if (is_usrptr) {
				if (copy_from_user(&tmp->event, (char __force __user *)buf, size)) {
					err = -EFAULT;
					goto __error;
				}
			} else {
				memcpy(&tmp->event, buf, size);
			}
			buf += size;
			len -= size;
		}
	}