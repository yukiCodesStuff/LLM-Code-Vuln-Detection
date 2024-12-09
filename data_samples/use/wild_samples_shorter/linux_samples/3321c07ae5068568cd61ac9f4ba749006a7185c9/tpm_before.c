{
	struct tpm_chip *chip = file->private_data;
	ssize_t ret_size;

	del_singleshot_timer_sync(&chip->user_read_timer);
	flush_work_sync(&chip->work);
	ret_size = atomic_read(&chip->data_pending);
			ret_size = size;

		mutex_lock(&chip->buffer_mutex);
		if (copy_to_user(buf, chip->data_buffer, ret_size))
			ret_size = -EFAULT;
		mutex_unlock(&chip->buffer_mutex);
	}

	return ret_size;