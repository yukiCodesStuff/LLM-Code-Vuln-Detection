	    (priv->data_buffer, (void __user *) buf, in_size)) {
		mutex_unlock(&priv->buffer_mutex);
		return -EFAULT;
	}

	/* atomic tpm command send and result receive. We only hold the ops
	 * lock during this period so that the tpm can be unregistered even if
	 * the char dev is held open.
	 */
	if (tpm_try_get_ops(priv->chip)) {
		mutex_unlock(&priv->buffer_mutex);
		return -EPIPE;
	}
	out_size = tpm_transmit(priv->chip, space, priv->data_buffer,
				sizeof(priv->data_buffer), 0);

	tpm_put_ops(priv->chip);
	if (out_size < 0) {
		mutex_unlock(&priv->buffer_mutex);
		return out_size;
	}

	atomic_set(&priv->data_pending, out_size);
	mutex_unlock(&priv->buffer_mutex);

	/* Set a timeout by which the reader must come claim the result */
	mod_timer(&priv->user_read_timer, jiffies + (120 * HZ));

	return in_size;
}

/*
 * Called on file close
 */
void tpm_common_release(struct file *file, struct file_priv *priv)
{
	del_singleshot_timer_sync(&priv->user_read_timer);
	flush_work(&priv->work);
	file->private_data = NULL;
	atomic_set(&priv->data_pending, 0);
}