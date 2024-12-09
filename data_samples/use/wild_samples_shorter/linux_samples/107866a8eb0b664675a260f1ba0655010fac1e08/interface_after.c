{
	if (queue->task) {
		kthread_stop(queue->task);
		put_task_struct(queue->task);
		queue->task = NULL;
	}

	if (queue->dealloc_task) {
	if (IS_ERR(task))
		goto kthread_err;
	queue->task = task;
	/*
	 * Take a reference to the task in order to prevent it from being freed
	 * if the thread function returns before kthread_stop is called.
	 */
	get_task_struct(task);

	task = kthread_run(xenvif_dealloc_kthread, queue,
			   "%s-dealloc", queue->name);
	if (IS_ERR(task))