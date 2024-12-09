struct snd_seq_pool {
	struct snd_seq_event_cell *ptr;	/* pointer to first event chunk */
	struct snd_seq_event_cell *free;	/* pointer to the head of the free list */

	int total_elements;	/* pool size actually allocated */
	atomic_t counter;	/* cells free */

	int size;		/* pool size to be allocated */
	int room;		/* watermark for sleep/wakeup */

	int closing;

	/* statistics */
	int max_used;
	int event_alloc_nopool;
	int event_alloc_failures;
	int event_alloc_success;

	/* Write locking */
	wait_queue_head_t output_sleep;

	/* Pool lock */
	spinlock_t lock;
};

void snd_seq_cell_free(struct snd_seq_event_cell *cell);

int snd_seq_event_dup(struct snd_seq_pool *pool, struct snd_seq_event *event,
		      struct snd_seq_event_cell **cellp, int nonblock,
		      struct file *file, struct mutex *mutexp);

/* return number of unused (free) cells */
static inline int snd_seq_unused_cells(struct snd_seq_pool *pool)
{
	return pool ? pool->total_elements - atomic_read(&pool->counter) : 0;
}