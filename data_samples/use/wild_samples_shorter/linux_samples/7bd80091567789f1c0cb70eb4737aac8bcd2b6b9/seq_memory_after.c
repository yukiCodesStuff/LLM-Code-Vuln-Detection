 */
static int snd_seq_cell_alloc(struct snd_seq_pool *pool,
			      struct snd_seq_event_cell **cellp,
			      int nonblock, struct file *file,
			      struct mutex *mutexp)
{
	struct snd_seq_event_cell *cell;
	unsigned long flags;
	int err = -EAGAIN;
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&pool->output_sleep, &wait);
		spin_unlock_irq(&pool->lock);
		if (mutexp)
			mutex_unlock(mutexp);
		schedule();
		if (mutexp)
			mutex_lock(mutexp);
		spin_lock_irq(&pool->lock);
		remove_wait_queue(&pool->output_sleep, &wait);
		/* interrupted? */
		if (signal_pending(current)) {
 */
int snd_seq_event_dup(struct snd_seq_pool *pool, struct snd_seq_event *event,
		      struct snd_seq_event_cell **cellp, int nonblock,
		      struct file *file, struct mutex *mutexp)
{
	int ncells, err;
	unsigned int extlen;
	struct snd_seq_event_cell *cell;
	if (ncells >= pool->total_elements)
		return -ENOMEM;

	err = snd_seq_cell_alloc(pool, &cell, nonblock, file, mutexp);
	if (err < 0)
		return err;

	/* copy the event */
			int size = sizeof(struct snd_seq_event);
			if (len < size)
				size = len;
			err = snd_seq_cell_alloc(pool, &tmp, nonblock, file,
						 mutexp);
			if (err < 0)
				goto __error;
			if (cell->event.data.ext.ptr == NULL)
				cell->event.data.ext.ptr = tmp;