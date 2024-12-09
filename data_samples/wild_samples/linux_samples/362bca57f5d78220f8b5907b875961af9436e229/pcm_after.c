			if (substream == NULL) {
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
	case SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE:
		{
			int val;
			
			if (get_user(val, (int __user *)arg))
				return -EFAULT;
			control->preferred_subdevice[SND_CTL_SUBDEV_PCM] = val;
			return 0;
		}
	}
	return -ENOIOCTLCMD;
}

#define FORMAT(v) [SNDRV_PCM_FORMAT_##v] = #v

static char *snd_pcm_format_names[] = {
	FORMAT(S8),
	FORMAT(U8),
	FORMAT(S16_LE),
	FORMAT(S16_BE),
	FORMAT(U16_LE),
	FORMAT(U16_BE),
	FORMAT(S24_LE),
	FORMAT(S24_BE),
	FORMAT(U24_LE),
	FORMAT(U24_BE),
	FORMAT(S32_LE),
	FORMAT(S32_BE),
	FORMAT(U32_LE),
	FORMAT(U32_BE),
	FORMAT(FLOAT_LE),
	FORMAT(FLOAT_BE),
	FORMAT(FLOAT64_LE),
	FORMAT(FLOAT64_BE),
	FORMAT(IEC958_SUBFRAME_LE),
	FORMAT(IEC958_SUBFRAME_BE),
	FORMAT(MU_LAW),
	FORMAT(A_LAW),
	FORMAT(IMA_ADPCM),
	FORMAT(MPEG),
	FORMAT(GSM),
	FORMAT(SPECIAL),
	FORMAT(S24_3LE),
	FORMAT(S24_3BE),
	FORMAT(U24_3LE),
	FORMAT(U24_3BE),
	FORMAT(S20_3LE),
	FORMAT(S20_3BE),
	FORMAT(U20_3LE),
	FORMAT(U20_3BE),
	FORMAT(S18_3LE),
	FORMAT(S18_3BE),
	FORMAT(U18_3LE),
	FORMAT(U18_3BE),
	FORMAT(G723_24),
	FORMAT(G723_24_1B),
	FORMAT(G723_40),
	FORMAT(G723_40_1B),
	FORMAT(DSD_U8),
	FORMAT(DSD_U16_LE),
	FORMAT(DSD_U32_LE),
	FORMAT(DSD_U16_BE),
	FORMAT(DSD_U32_BE),
};

/**
 * snd_pcm_format_name - Return a name string for the given PCM format
 * @format: PCM format
 */
const char *snd_pcm_format_name(snd_pcm_format_t format)
{
	if ((__force unsigned int)format >= ARRAY_SIZE(snd_pcm_format_names))
		return "Unknown";
	return snd_pcm_format_names[(__force unsigned int)format];
}
EXPORT_SYMBOL_GPL(snd_pcm_format_name);

#ifdef CONFIG_SND_VERBOSE_PROCFS

#define STATE(v) [SNDRV_PCM_STATE_##v] = #v
#define STREAM(v) [SNDRV_PCM_STREAM_##v] = #v
#define READY(v) [SNDRV_PCM_READY_##v] = #v
#define XRUN(v) [SNDRV_PCM_XRUN_##v] = #v
#define SILENCE(v) [SNDRV_PCM_SILENCE_##v] = #v
#define TSTAMP(v) [SNDRV_PCM_TSTAMP_##v] = #v
#define ACCESS(v) [SNDRV_PCM_ACCESS_##v] = #v
#define START(v) [SNDRV_PCM_START_##v] = #v
#define SUBFORMAT(v) [SNDRV_PCM_SUBFORMAT_##v] = #v 

static char *snd_pcm_stream_names[] = {
	STREAM(PLAYBACK),
	STREAM(CAPTURE),
};

static char *snd_pcm_state_names[] = {
	STATE(OPEN),
	STATE(SETUP),
	STATE(PREPARED),
	STATE(RUNNING),
	STATE(XRUN),
	STATE(DRAINING),
	STATE(PAUSED),
	STATE(SUSPENDED),
};

static char *snd_pcm_access_names[] = {
	ACCESS(MMAP_INTERLEAVED), 
	ACCESS(MMAP_NONINTERLEAVED),
	ACCESS(MMAP_COMPLEX),
	ACCESS(RW_INTERLEAVED),
	ACCESS(RW_NONINTERLEAVED),
};

static char *snd_pcm_subformat_names[] = {
	SUBFORMAT(STD), 
};

static char *snd_pcm_tstamp_mode_names[] = {
	TSTAMP(NONE),
	TSTAMP(ENABLE),
};

static const char *snd_pcm_stream_name(int stream)
{
	return snd_pcm_stream_names[stream];
}

static const char *snd_pcm_access_name(snd_pcm_access_t access)
{
	return snd_pcm_access_names[(__force int)access];
}

static const char *snd_pcm_subformat_name(snd_pcm_subformat_t subformat)
{
	return snd_pcm_subformat_names[(__force int)subformat];
}

static const char *snd_pcm_tstamp_mode_name(int mode)
{
	return snd_pcm_tstamp_mode_names[mode];
}

static const char *snd_pcm_state_name(snd_pcm_state_t state)
{
	return snd_pcm_state_names[(__force int)state];
}

#if IS_ENABLED(CONFIG_SND_PCM_OSS)
#include <linux/soundcard.h>

static const char *snd_pcm_oss_format_name(int format)
{
	switch (format) {
	case AFMT_MU_LAW:
		return "MU_LAW";
	case AFMT_A_LAW:
		return "A_LAW";
	case AFMT_IMA_ADPCM:
		return "IMA_ADPCM";
	case AFMT_U8:
		return "U8";
	case AFMT_S16_LE:
		return "S16_LE";
	case AFMT_S16_BE:
		return "S16_BE";
	case AFMT_S8:
		return "S8";
	case AFMT_U16_LE:
		return "U16_LE";
	case AFMT_U16_BE:
		return "U16_BE";
	case AFMT_MPEG:
		return "MPEG";
	default:
		return "unknown";
	}
}
#endif

static void snd_pcm_proc_info_read(struct snd_pcm_substream *substream,
				   struct snd_info_buffer *buffer)
{
	struct snd_pcm_info *info;
	int err;

	if (! substream)
		return;

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return;

	err = snd_pcm_info(substream, info);
	if (err < 0) {
		snd_iprintf(buffer, "error %d\n", err);
		kfree(info);
		return;
	}
	snd_iprintf(buffer, "card: %d\n", info->card);
	snd_iprintf(buffer, "device: %d\n", info->device);
	snd_iprintf(buffer, "subdevice: %d\n", info->subdevice);
	snd_iprintf(buffer, "stream: %s\n", snd_pcm_stream_name(info->stream));
	snd_iprintf(buffer, "id: %s\n", info->id);
	snd_iprintf(buffer, "name: %s\n", info->name);
	snd_iprintf(buffer, "subname: %s\n", info->subname);
	snd_iprintf(buffer, "class: %d\n", info->dev_class);
	snd_iprintf(buffer, "subclass: %d\n", info->dev_subclass);
	snd_iprintf(buffer, "subdevices_count: %d\n", info->subdevices_count);
	snd_iprintf(buffer, "subdevices_avail: %d\n", info->subdevices_avail);
	kfree(info);
}

static void snd_pcm_stream_proc_info_read(struct snd_info_entry *entry,
					  struct snd_info_buffer *buffer)
{
	snd_pcm_proc_info_read(((struct snd_pcm_str *)entry->private_data)->substream,
			       buffer);
}

static void snd_pcm_substream_proc_info_read(struct snd_info_entry *entry,
					     struct snd_info_buffer *buffer)
{
	snd_pcm_proc_info_read(entry->private_data, buffer);
}

static void snd_pcm_substream_proc_hw_params_read(struct snd_info_entry *entry,
						  struct snd_info_buffer *buffer)
{
	struct snd_pcm_substream *substream = entry->private_data;
	struct snd_pcm_runtime *runtime;

	mutex_lock(&substream->pcm->open_mutex);
	runtime = substream->runtime;
	if (!runtime) {
		snd_iprintf(buffer, "closed\n");
		goto unlock;
	}
	if (runtime->status->state == SNDRV_PCM_STATE_OPEN) {
		snd_iprintf(buffer, "no setup\n");
		goto unlock;
	}
	snd_iprintf(buffer, "access: %s\n", snd_pcm_access_name(runtime->access));
	snd_iprintf(buffer, "format: %s\n", snd_pcm_format_name(runtime->format));
	snd_iprintf(buffer, "subformat: %s\n", snd_pcm_subformat_name(runtime->subformat));
	snd_iprintf(buffer, "channels: %u\n", runtime->channels);	
	snd_iprintf(buffer, "rate: %u (%u/%u)\n", runtime->rate, runtime->rate_num, runtime->rate_den);	
	snd_iprintf(buffer, "period_size: %lu\n", runtime->period_size);	
	snd_iprintf(buffer, "buffer_size: %lu\n", runtime->buffer_size);	
#if IS_ENABLED(CONFIG_SND_PCM_OSS)
	if (substream->oss.oss) {
		snd_iprintf(buffer, "OSS format: %s\n", snd_pcm_oss_format_name(runtime->oss.format));
		snd_iprintf(buffer, "OSS channels: %u\n", runtime->oss.channels);	
		snd_iprintf(buffer, "OSS rate: %u\n", runtime->oss.rate);
		snd_iprintf(buffer, "OSS period bytes: %lu\n", (unsigned long)runtime->oss.period_bytes);
		snd_iprintf(buffer, "OSS periods: %u\n", runtime->oss.periods);
		snd_iprintf(buffer, "OSS period frames: %lu\n", (unsigned long)runtime->oss.period_frames);
	}
#endif
 unlock:
	mutex_unlock(&substream->pcm->open_mutex);
}

static void snd_pcm_substream_proc_sw_params_read(struct snd_info_entry *entry,
						  struct snd_info_buffer *buffer)
{
	struct snd_pcm_substream *substream = entry->private_data;
	struct snd_pcm_runtime *runtime;

	mutex_lock(&substream->pcm->open_mutex);
	runtime = substream->runtime;
	if (!runtime) {
		snd_iprintf(buffer, "closed\n");
		goto unlock;
	}
	if (runtime->status->state == SNDRV_PCM_STATE_OPEN) {
		snd_iprintf(buffer, "no setup\n");
		goto unlock;
	}
	snd_iprintf(buffer, "tstamp_mode: %s\n", snd_pcm_tstamp_mode_name(runtime->tstamp_mode));
	snd_iprintf(buffer, "period_step: %u\n", runtime->period_step);
	snd_iprintf(buffer, "avail_min: %lu\n", runtime->control->avail_min);
	snd_iprintf(buffer, "start_threshold: %lu\n", runtime->start_threshold);
	snd_iprintf(buffer, "stop_threshold: %lu\n", runtime->stop_threshold);
	snd_iprintf(buffer, "silence_threshold: %lu\n", runtime->silence_threshold);
	snd_iprintf(buffer, "silence_size: %lu\n", runtime->silence_size);
	snd_iprintf(buffer, "boundary: %lu\n", runtime->boundary);
 unlock:
	mutex_unlock(&substream->pcm->open_mutex);
}

static void snd_pcm_substream_proc_status_read(struct snd_info_entry *entry,
					       struct snd_info_buffer *buffer)
{
	struct snd_pcm_substream *substream = entry->private_data;
	struct snd_pcm_runtime *runtime;
	struct snd_pcm_status status;
	int err;

	mutex_lock(&substream->pcm->open_mutex);
	runtime = substream->runtime;
	if (!runtime) {
		snd_iprintf(buffer, "closed\n");
		goto unlock;
	}
	memset(&status, 0, sizeof(status));
	err = snd_pcm_status(substream, &status);
	if (err < 0) {
		snd_iprintf(buffer, "error %d\n", err);
		goto unlock;
	}
	snd_iprintf(buffer, "state: %s\n", snd_pcm_state_name(status.state));
	snd_iprintf(buffer, "owner_pid   : %d\n", pid_vnr(substream->pid));
	snd_iprintf(buffer, "trigger_time: %ld.%09ld\n",
		status.trigger_tstamp.tv_sec, status.trigger_tstamp.tv_nsec);
	snd_iprintf(buffer, "tstamp      : %ld.%09ld\n",
		status.tstamp.tv_sec, status.tstamp.tv_nsec);
	snd_iprintf(buffer, "delay       : %ld\n", status.delay);
	snd_iprintf(buffer, "avail       : %ld\n", status.avail);
	snd_iprintf(buffer, "avail_max   : %ld\n", status.avail_max);
	snd_iprintf(buffer, "-----\n");
	snd_iprintf(buffer, "hw_ptr      : %ld\n", runtime->status->hw_ptr);
	snd_iprintf(buffer, "appl_ptr    : %ld\n", runtime->control->appl_ptr);
 unlock:
	mutex_unlock(&substream->pcm->open_mutex);
}

#ifdef CONFIG_SND_PCM_XRUN_DEBUG
static void snd_pcm_xrun_injection_write(struct snd_info_entry *entry,
					 struct snd_info_buffer *buffer)
{
	struct snd_pcm_substream *substream = entry->private_data;
	struct snd_pcm_runtime *runtime;

	snd_pcm_stream_lock_irq(substream);
	runtime = substream->runtime;
	if (runtime && runtime->status->state == SNDRV_PCM_STATE_RUNNING)
		snd_pcm_stop(substream, SNDRV_PCM_STATE_XRUN);
	snd_pcm_stream_unlock_irq(substream);
}

static void snd_pcm_xrun_debug_read(struct snd_info_entry *entry,
				    struct snd_info_buffer *buffer)
{
	struct snd_pcm_str *pstr = entry->private_data;
	snd_iprintf(buffer, "%d\n", pstr->xrun_debug);
}

static void snd_pcm_xrun_debug_write(struct snd_info_entry *entry,
				     struct snd_info_buffer *buffer)
{
	struct snd_pcm_str *pstr = entry->private_data;
	char line[64];
	if (!snd_info_get_line(buffer, line, sizeof(line)))
		pstr->xrun_debug = simple_strtoul(line, NULL, 10);
}
#endif

static int snd_pcm_stream_proc_init(struct snd_pcm_str *pstr)
{
	struct snd_pcm *pcm = pstr->pcm;
	struct snd_info_entry *entry;
	char name[16];

	sprintf(name, "pcm%i%c", pcm->device, 
		pstr->stream == SNDRV_PCM_STREAM_PLAYBACK ? 'p' : 'c');
	entry = snd_info_create_card_entry(pcm->card, name,
					   pcm->card->proc_root);
	if (!entry)
		return -ENOMEM;
	entry->mode = S_IFDIR | S_IRUGO | S_IXUGO;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	pstr->proc_root = entry;
	entry = snd_info_create_card_entry(pcm->card, "info", pstr->proc_root);
	if (entry) {
		snd_info_set_text_ops(entry, pstr, snd_pcm_stream_proc_info_read);
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	pstr->proc_info_entry = entry;

#ifdef CONFIG_SND_PCM_XRUN_DEBUG
	entry = snd_info_create_card_entry(pcm->card, "xrun_debug",
					   pstr->proc_root);
	if (entry) {
		entry->c.text.read = snd_pcm_xrun_debug_read;
		entry->c.text.write = snd_pcm_xrun_debug_write;
		entry->mode |= S_IWUSR;
		entry->private_data = pstr;
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	pstr->proc_xrun_debug_entry = entry;
#endif
	return 0;
}

static int snd_pcm_stream_proc_done(struct snd_pcm_str *pstr)
{
#ifdef CONFIG_SND_PCM_XRUN_DEBUG
	snd_info_free_entry(pstr->proc_xrun_debug_entry);
	pstr->proc_xrun_debug_entry = NULL;
#endif
	snd_info_free_entry(pstr->proc_info_entry);
	pstr->proc_info_entry = NULL;
	snd_info_free_entry(pstr->proc_root);
	pstr->proc_root = NULL;
	return 0;
}

static int snd_pcm_substream_proc_init(struct snd_pcm_substream *substream)
{
	struct snd_info_entry *entry;
	struct snd_card *card;
	char name[16];

	card = substream->pcm->card;

	sprintf(name, "sub%i", substream->number);
	entry = snd_info_create_card_entry(card, name,
					   substream->pstr->proc_root);
	if (!entry)
		return -ENOMEM;
	entry->mode = S_IFDIR | S_IRUGO | S_IXUGO;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	substream->proc_root = entry;
	entry = snd_info_create_card_entry(card, "info", substream->proc_root);
	if (entry) {
		snd_info_set_text_ops(entry, substream,
				      snd_pcm_substream_proc_info_read);
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	substream->proc_info_entry = entry;
	entry = snd_info_create_card_entry(card, "hw_params",
					   substream->proc_root);
	if (entry) {
		snd_info_set_text_ops(entry, substream,
				      snd_pcm_substream_proc_hw_params_read);
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	substream->proc_hw_params_entry = entry;
	entry = snd_info_create_card_entry(card, "sw_params",
					   substream->proc_root);
	if (entry) {
		snd_info_set_text_ops(entry, substream,
				      snd_pcm_substream_proc_sw_params_read);
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	substream->proc_sw_params_entry = entry;
	entry = snd_info_create_card_entry(card, "status",
					   substream->proc_root);
	if (entry) {
		snd_info_set_text_ops(entry, substream,
				      snd_pcm_substream_proc_status_read);
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	substream->proc_status_entry = entry;

#ifdef CONFIG_SND_PCM_XRUN_DEBUG
	entry = snd_info_create_card_entry(card, "xrun_injection",
					   substream->proc_root);
	if (entry) {
		entry->private_data = substream;
		entry->c.text.read = NULL;
		entry->c.text.write = snd_pcm_xrun_injection_write;
		entry->mode = S_IFREG | S_IWUSR;
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	substream->proc_xrun_injection_entry = entry;
#endif /* CONFIG_SND_PCM_XRUN_DEBUG */

	return 0;
}

static int snd_pcm_substream_proc_done(struct snd_pcm_substream *substream)
{
	snd_info_free_entry(substream->proc_info_entry);
	substream->proc_info_entry = NULL;
	snd_info_free_entry(substream->proc_hw_params_entry);
	substream->proc_hw_params_entry = NULL;
	snd_info_free_entry(substream->proc_sw_params_entry);
	substream->proc_sw_params_entry = NULL;
	snd_info_free_entry(substream->proc_status_entry);
	substream->proc_status_entry = NULL;
#ifdef CONFIG_SND_PCM_XRUN_DEBUG
	snd_info_free_entry(substream->proc_xrun_injection_entry);
	substream->proc_xrun_injection_entry = NULL;
#endif
	snd_info_free_entry(substream->proc_root);
	substream->proc_root = NULL;
	return 0;
}
#else /* !CONFIG_SND_VERBOSE_PROCFS */
static inline int snd_pcm_stream_proc_init(struct snd_pcm_str *pstr) { return 0; }
static inline int snd_pcm_stream_proc_done(struct snd_pcm_str *pstr) { return 0; }