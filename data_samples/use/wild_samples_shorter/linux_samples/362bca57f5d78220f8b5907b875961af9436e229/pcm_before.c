				err = -ENXIO;
				goto _error;
			}
			err = snd_pcm_info_user(substream, info);
		_error:
			mutex_unlock(&register_mutex);
			return err;
		}