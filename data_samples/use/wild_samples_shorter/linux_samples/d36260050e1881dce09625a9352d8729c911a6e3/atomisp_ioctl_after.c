
/* FIXME! */
#ifndef ISP2401
static void __wdt_on_master_slave_sensor(struct atomisp_device *isp,
					 unsigned int wdt_duration)
#else
static void __wdt_on_master_slave_sensor(struct atomisp_video_pipe *pipe,
					 unsigned int wdt_duration,
					 bool enable)
#endif
{
#ifndef ISP2401
	if (atomisp_buffers_queued(&isp->asd[0]))
	}

	rt_mutex_lock(&isp->mutex);
	isp->sw_contex.file_input = true;
	rt_mutex_unlock(&isp->mutex);

	return 0;
}