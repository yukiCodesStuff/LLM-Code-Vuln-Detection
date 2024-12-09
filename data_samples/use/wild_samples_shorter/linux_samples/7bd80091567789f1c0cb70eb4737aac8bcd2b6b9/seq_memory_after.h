void snd_seq_cell_free(struct snd_seq_event_cell *cell);

int snd_seq_event_dup(struct snd_seq_pool *pool, struct snd_seq_event *event,
		      struct snd_seq_event_cell **cellp, int nonblock,
		      struct file *file, struct mutex *mutexp);

/* return number of unused (free) cells */
static inline int snd_seq_unused_cells(struct snd_seq_pool *pool)
{