				err = -ENXIO;
				goto _error;
			}
			mutex_lock(&pcm->open_mutex);
			err = snd_pcm_info_user(substream, info);
			mutex_unlock(&pcm->open_mutex);
		_error:
			mutex_unlock(&register_mutex);
			return err;
		}