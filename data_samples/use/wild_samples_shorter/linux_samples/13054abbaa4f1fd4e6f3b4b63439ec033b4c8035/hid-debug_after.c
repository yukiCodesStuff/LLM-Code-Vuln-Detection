
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/kfifo.h>
#include <linux/sched/signal.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
/* enqueue string to 'events' ring buffer */
void hid_debug_event(struct hid_device *hdev, char *buf)
{
	struct hid_debug_list *list;
	unsigned long flags;

	spin_lock_irqsave(&hdev->debug_list_lock, flags);
	list_for_each_entry(list, &hdev->debug_list, node)
		kfifo_in(&list->hid_debug_fifo, buf, strlen(buf));
	spin_unlock_irqrestore(&hdev->debug_list_lock, flags);

	wake_up_interruptible(&hdev->debug_wait);
}
	hid_debug_event(hdev, buf);

	kfree(buf);
	wake_up_interruptible(&hdev->debug_wait);
}
EXPORT_SYMBOL_GPL(hid_dump_input);

static const char *events[EV_MAX + 1] = {
		goto out;
	}

	err = kfifo_alloc(&list->hid_debug_fifo, HID_DEBUG_FIFOSIZE, GFP_KERNEL);
	if (err) {
		kfree(list);
		goto out;
	}
	list->hdev = (struct hid_device *) inode->i_private;
		size_t count, loff_t *ppos)
{
	struct hid_debug_list *list = file->private_data;
	int ret = 0, copied;
	DECLARE_WAITQUEUE(wait, current);

	mutex_lock(&list->read_mutex);
	if (kfifo_is_empty(&list->hid_debug_fifo)) {
		add_wait_queue(&list->hdev->debug_wait, &wait);
		set_current_state(TASK_INTERRUPTIBLE);

		while (kfifo_is_empty(&list->hid_debug_fifo)) {
			if (file->f_flags & O_NONBLOCK) {
				ret = -EAGAIN;
				break;
			}

			if (signal_pending(current)) {
				ret = -ERESTARTSYS;
				break;
			}

			/* if list->hdev is NULL we cannot remove_wait_queue().
			 * if list->hdev->debug is 0 then hid_debug_unregister()
			 * was already called and list->hdev is being destroyed.
			 * if we add remove_wait_queue() here we can hit a race.
			 */
			if (!list->hdev || !list->hdev->debug) {
				ret = -EIO;
				set_current_state(TASK_RUNNING);
				goto out;
			}

			/* allow O_NONBLOCK from other threads */
			mutex_unlock(&list->read_mutex);
			schedule();
			mutex_lock(&list->read_mutex);
			set_current_state(TASK_INTERRUPTIBLE);
		}

		__set_current_state(TASK_RUNNING);
		remove_wait_queue(&list->hdev->debug_wait, &wait);

		if (ret)
			goto out;
	}

	/* pass the fifo content to userspace, locking is not needed with only
	 * one concurrent reader and one concurrent writer
	 */
	ret = kfifo_to_user(&list->hid_debug_fifo, buffer, count, &copied);
	if (ret)
		goto out;
	ret = copied;
out:
	mutex_unlock(&list->read_mutex);
	return ret;
}
	struct hid_debug_list *list = file->private_data;

	poll_wait(file, &list->hdev->debug_wait, wait);
	if (!kfifo_is_empty(&list->hid_debug_fifo))
		return EPOLLIN | EPOLLRDNORM;
	if (!list->hdev->debug)
		return EPOLLERR | EPOLLHUP;
	return 0;
	spin_lock_irqsave(&list->hdev->debug_list_lock, flags);
	list_del(&list->node);
	spin_unlock_irqrestore(&list->hdev->debug_list_lock, flags);
	kfifo_free(&list->hid_debug_fifo);
	kfree(list);

	return 0;
}
{
	debugfs_remove_recursive(hid_debug_root);
}