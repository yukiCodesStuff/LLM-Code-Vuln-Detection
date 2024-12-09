{
	if (queue->task) {
		kthread_stop(queue->task);
		queue->task = NULL;
	}

	if (queue->dealloc_task) {
	if (IS_ERR(task))
		goto kthread_err;
	queue->task = task;

	task = kthread_run(xenvif_dealloc_kthread, queue,
			   "%s-dealloc", queue->name);
	if (IS_ERR(task))