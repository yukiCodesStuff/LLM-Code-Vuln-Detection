
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
/* enqueue string to 'events' ring buffer */
void hid_debug_event(struct hid_device *hdev, char *buf)
{
	unsigned i;
	struct hid_debug_list *list;
	unsigned long flags;

	spin_lock_irqsave(&hdev->debug_list_lock, flags);
	list_for_each_entry(list, &hdev->debug_list, node) {
		for (i = 0; buf[i]; i++)
			list->hid_debug_buf[(list->tail + i) % HID_DEBUG_BUFSIZE] =
				buf[i];
		list->tail = (list->tail + i) % HID_DEBUG_BUFSIZE;
        }
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

	if (!(list->hid_debug_buf = kzalloc(HID_DEBUG_BUFSIZE, GFP_KERNEL))) {
		err = -ENOMEM;
		kfree(list);
		goto out;
	}
	list->hdev = (struct hid_device *) inode->i_private;
		size_t count, loff_t *ppos)
{
	struct hid_debug_list *list = file->private_data;
	int ret = 0, len;
	DECLARE_WAITQUEUE(wait, current);

	mutex_lock(&list->read_mutex);
	while (ret == 0) {
		if (list->head == list->tail) {
			add_wait_queue(&list->hdev->debug_wait, &wait);
			set_current_state(TASK_INTERRUPTIBLE);

			while (list->head == list->tail) {
				if (file->f_flags & O_NONBLOCK) {
					ret = -EAGAIN;
					break;
				}
				if (signal_pending(current)) {
					ret = -ERESTARTSYS;
					break;
				}

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

			set_current_state(TASK_RUNNING);
			remove_wait_queue(&list->hdev->debug_wait, &wait);
		}

		if (ret)
			goto out;

		/* pass the ringbuffer contents to userspace */
copy_rest:
		if (list->tail == list->head)
			goto out;
		if (list->tail > list->head) {
			len = list->tail - list->head;
			if (len > count)
				len = count;

			if (copy_to_user(buffer + ret, &list->hid_debug_buf[list->head], len)) {
				ret = -EFAULT;
				goto out;
			}
			ret += len;
			list->head += len;
		} else {
			len = HID_DEBUG_BUFSIZE - list->head;
			if (len > count)
				len = count;

			if (copy_to_user(buffer, &list->hid_debug_buf[list->head], len)) {
				ret = -EFAULT;
				goto out;
			}
			list->head = 0;
			ret += len;
			count -= len;
			if (count > 0)
				goto copy_rest;
		}

	}
out:
	mutex_unlock(&list->read_mutex);
	return ret;
}
	struct hid_debug_list *list = file->private_data;

	poll_wait(file, &list->hdev->debug_wait, wait);
	if (list->head != list->tail)
		return EPOLLIN | EPOLLRDNORM;
	if (!list->hdev->debug)
		return EPOLLERR | EPOLLHUP;
	return 0;
	spin_lock_irqsave(&list->hdev->debug_list_lock, flags);
	list_del(&list->node);
	spin_unlock_irqrestore(&list->hdev->debug_list_lock, flags);
	kfree(list->hid_debug_buf);
	kfree(list);

	return 0;
}
{
	debugfs_remove_recursive(hid_debug_root);
}
