		return -EFAULT;
	}

	if (in_size < 6 ||
	    in_size < be32_to_cpu(*((__be32 *) (priv->data_buffer + 2)))) {
		mutex_unlock(&priv->buffer_mutex);
		return -EINVAL;
	}

	/* atomic tpm command send and result receive. We only hold the ops
	 * lock during this period so that the tpm can be unregistered even if
	 * the char dev is held open.
	 */