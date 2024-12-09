{
	unsigned int i;

	isp->sw_contex.file_input = false;
	isp->need_gfx_throttle = true;
	isp->isp_fatal_error = false;
	isp->mipi_frame_size = 0;

	for (i = 0; i < isp->input_cnt; i++)
		isp->inputs[i].asd = NULL;
	/*
	 * For Merrifield, frequency is scalable.
	 * After boot-up, the default frequency is 200MHz.
	 */
	isp->sw_contex.running_freq = ISP_FREQ_200MHZ;
}

static void atomisp_subdev_init_struct(struct atomisp_sub_device *asd)
{
	v4l2_ctrl_s_ctrl(asd->run_mode, ATOMISP_RUN_MODE_STILL_CAPTURE);
	memset(&asd->params.css_param, 0, sizeof(asd->params.css_param));
	asd->params.color_effect = V4L2_COLORFX_NONE;
	asd->params.bad_pixel_en = true;
	asd->params.gdc_cac_en = false;
	asd->params.video_dis_en = false;
	asd->params.sc_en = false;
	asd->params.fpn_en = false;
	asd->params.xnr_en = false;
	asd->params.false_color = 0;
	asd->params.online_process = 1;
	asd->params.yuv_ds_en = 0;
	/* s3a grid not enabled for any pipe */
	asd->params.s3a_enabled_pipe = CSS_PIPE_ID_NUM;

	asd->params.offline_parm.num_captures = 1;
	asd->params.offline_parm.skip_frames = 0;
	asd->params.offline_parm.offset = 0;
	asd->delayed_init = ATOMISP_DELAYED_INIT_NOT_QUEUED;
	/* Add for channel */
	asd->input_curr = 0;

	asd->mipi_frame_size = 0;
	asd->copy_mode = false;
	asd->yuvpp_mode = false;

	asd->stream_prepared = false;
	asd->high_speed_mode = false;
	asd->sensor_array_res.height = 0;
	asd->sensor_array_res.width = 0;
	atomisp_css_init_struct(asd);
}
/*
 * file operation functions
 */
static unsigned int atomisp_subdev_users(struct atomisp_sub_device *asd)
{
	return asd->video_out_preview.users +
	       asd->video_out_vf.users +
	       asd->video_out_capture.users +
	       asd->video_out_video_capture.users +
	       asd->video_acc.users +
	       asd->video_in.users;
}

unsigned int atomisp_dev_users(struct atomisp_device *isp)
{
	unsigned int i, sum;
	for (i = 0, sum = 0; i < isp->num_of_streams; i++)
		sum += atomisp_subdev_users(&isp->asd[i]);

	return sum;
}

static int atomisp_open(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = NULL;
	struct atomisp_acc_pipe *acc_pipe = NULL;
	struct atomisp_sub_device *asd;
	bool acc_node = false;
	int ret;

	dev_dbg(isp->dev, "open device %s\n", vdev->name);

	rt_mutex_lock(&isp->mutex);

	acc_node = !strncmp(vdev->name, "ATOMISP ISP ACC",
			sizeof(vdev->name));
	if (acc_node) {
		acc_pipe = atomisp_to_acc_pipe(vdev);
		asd = acc_pipe->asd;
	} else {
		pipe = atomisp_to_video_pipe(vdev);
		asd = pipe->asd;
	}
	asd->subdev.devnode = vdev;
	/* Deferred firmware loading case. */
	if (isp->css_env.isp_css_fw.bytes == 0) {
		isp->firmware = atomisp_load_firmware(isp);
		if (!isp->firmware) {
			dev_err(isp->dev, "Failed to load ISP firmware.\n");
			ret = -ENOENT;
			goto error;
		}
		ret = atomisp_css_load_firmware(isp);
		if (ret) {
			dev_err(isp->dev, "Failed to init css.\n");
			goto error;
		}
		/* No need to keep FW in memory anymore. */
		release_firmware(isp->firmware);
		isp->firmware = NULL;
		isp->css_env.isp_css_fw.data = NULL;
	}

	if (acc_node && acc_pipe->users) {
		dev_dbg(isp->dev, "acc node already opened\n");
		rt_mutex_unlock(&isp->mutex);
		return -EBUSY;
	} else if (acc_node) {
		goto dev_init;
	}

	if (!isp->input_cnt) {
		dev_err(isp->dev, "no camera attached\n");
		ret = -EINVAL;
		goto error;
	}

	/*
	 * atomisp does not allow multiple open
	 */
	if (pipe->users) {
		dev_dbg(isp->dev, "video node already opened\n");
		rt_mutex_unlock(&isp->mutex);
		return -EBUSY;
	}

	ret = atomisp_init_pipe(pipe);
	if (ret)
		goto error;

dev_init:
	if (atomisp_dev_users(isp)) {
		dev_dbg(isp->dev, "skip init isp in open\n");
		goto init_subdev;
	}

	/* runtime power management, turn on ISP */
	ret = pm_runtime_get_sync(vdev->v4l2_dev->dev);
	if (ret < 0) {
		dev_err(isp->dev, "Failed to power on device\n");
		goto error;
	}

	if (dypool_enable) {
		ret = hmm_pool_register(dypool_pgnr, HMM_POOL_TYPE_DYNAMIC);
		if (ret)
			dev_err(isp->dev, "Failed to register dynamic memory pool.\n");
	}

	/* Init ISP */
	if (atomisp_css_init(isp)) {
		ret = -EINVAL;
		/* Need to clean up CSS init if it fails. */
		goto css_error;
	}

	atomisp_dev_init_struct(isp);

	ret = v4l2_subdev_call(isp->flash, core, s_power, 1);
	if (ret < 0 && ret != -ENODEV && ret != -ENOIOCTLCMD) {
		dev_err(isp->dev, "Failed to power-on flash\n");
		goto css_error;
	}

init_subdev:
	if (atomisp_subdev_users(asd))
		goto done;

	atomisp_subdev_init_struct(asd);

done:

	if (acc_node)
		acc_pipe->users++;
	else
		pipe->users++;
	rt_mutex_unlock(&isp->mutex);
	return 0;

css_error:
	atomisp_css_uninit(isp);
error:
	hmm_pool_unregister(HMM_POOL_TYPE_DYNAMIC);
	pm_runtime_put(vdev->v4l2_dev->dev);
	rt_mutex_unlock(&isp->mutex);
	return ret;
}

static int atomisp_release(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe;
	struct atomisp_acc_pipe *acc_pipe;
	struct atomisp_sub_device *asd;
	bool acc_node;
	struct v4l2_requestbuffers req;
	struct v4l2_subdev_fh fh;
	struct v4l2_rect clear_compose = {0};
	int ret = 0;

	v4l2_fh_init(&fh.vfh, vdev);

	req.count = 0;
	if (isp == NULL)
		return -EBADF;

	mutex_lock(&isp->streamoff_mutex);
	rt_mutex_lock(&isp->mutex);

	dev_dbg(isp->dev, "release device %s\n", vdev->name);
	acc_node = !strncmp(vdev->name, "ATOMISP ISP ACC",
			sizeof(vdev->name));
	if (acc_node) {
		acc_pipe = atomisp_to_acc_pipe(vdev);
		asd = acc_pipe->asd;
	} else {
		pipe = atomisp_to_video_pipe(vdev);
		asd = pipe->asd;
	}
	asd->subdev.devnode = vdev;
	if (acc_node) {
		acc_pipe->users--;
		goto subdev_uninit;
	}
	pipe->users--;

	if (pipe->capq.streaming)
		dev_warn(isp->dev,
				"%s: ISP still streaming while closing!",
				__func__);

	if (pipe->capq.streaming &&
	    __atomisp_streamoff(file, NULL, V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
		dev_err(isp->dev,
			"atomisp_streamoff failed on release, driver bug");
		goto done;
	}

	if (pipe->users)
		goto done;

	if (__atomisp_reqbufs(file, NULL, &req)) {
		dev_err(isp->dev,
			"atomisp_reqbufs failed on release, driver bug");
		goto done;
	}

	if (pipe->outq.bufs[0]) {
		mutex_lock(&pipe->outq.vb_lock);
		videobuf_queue_cancel(&pipe->outq);
		mutex_unlock(&pipe->outq.vb_lock);
	}

	/*
	 * A little trick here:
	 * file injection input resolution is recorded in the sink pad,
	 * therefore can not be cleared when releaseing one device node.
	 * The sink pad setting can only be cleared when all device nodes
	 * get released.
	 */
	if (!isp->sw_contex.file_input && asd->fmt_auto->val) {
		struct v4l2_mbus_framefmt isp_sink_fmt = { 0 };
		atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SINK, &isp_sink_fmt);
	}
subdev_uninit:
	if (atomisp_subdev_users(asd))
		goto done;

	/* clear the sink pad for file input */
	if (isp->sw_contex.file_input && asd->fmt_auto->val) {
		struct v4l2_mbus_framefmt isp_sink_fmt = { 0 };
		atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SINK, &isp_sink_fmt);
	}

	atomisp_css_free_stat_buffers(asd);
	atomisp_free_internal_buffers(asd);
	ret = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
				       core, s_power, 0);
	if (ret)
		dev_warn(isp->dev, "Failed to power-off sensor\n");

	/* clear the asd field to show this camera is not used */
	isp->inputs[asd->input_curr].asd = NULL;
	asd->streaming = ATOMISP_DEVICE_STREAMING_DISABLED;

	if (atomisp_dev_users(isp))
		goto done;

	atomisp_acc_release(asd);

	atomisp_destroy_pipes_stream_force(asd);
	atomisp_css_uninit(isp);

	if (defer_fw_load) {
		atomisp_css_unload_firmware(isp);
		isp->css_env.isp_css_fw.data = NULL;
		isp->css_env.isp_css_fw.bytes = 0;
	}

	hmm_pool_unregister(HMM_POOL_TYPE_DYNAMIC);

	ret = v4l2_subdev_call(isp->flash, core, s_power, 0);
	if (ret < 0 && ret != -ENODEV && ret != -ENOIOCTLCMD)
		dev_warn(isp->dev, "Failed to power-off flash\n");

	if (pm_runtime_put_sync(vdev->v4l2_dev->dev) < 0)
		dev_err(isp->dev, "Failed to power off device\n");

done:
	if (!acc_node) {
		atomisp_subdev_set_selection(&asd->subdev, fh.pad,
				V4L2_SUBDEV_FORMAT_ACTIVE,
				atomisp_subdev_source_pad(vdev),
				V4L2_SEL_TGT_COMPOSE, 0,
				&clear_compose);
	}
	rt_mutex_unlock(&isp->mutex);
	mutex_unlock(&isp->streamoff_mutex);

	return 0;
}

/*
 * Memory help functions for image frame and private parameters
 */
static int do_isp_mm_remap(struct atomisp_device *isp,
			   struct vm_area_struct *vma,
			   ia_css_ptr isp_virt, u32 host_virt, u32 pgnr)
{
	u32 pfn;

	while (pgnr) {
		pfn = hmm_virt_to_phys(isp_virt) >> PAGE_SHIFT;
		if (remap_pfn_range(vma, host_virt, pfn,
				    PAGE_SIZE, PAGE_SHARED)) {
			dev_err(isp->dev, "remap_pfn_range err.\n");
			return -EAGAIN;
		}

		isp_virt += PAGE_SIZE;
		host_virt += PAGE_SIZE;
		pgnr--;
	}

	return 0;
}

static int frame_mmap(struct atomisp_device *isp,
	const struct atomisp_css_frame *frame, struct vm_area_struct *vma)
{
	ia_css_ptr isp_virt;
	u32 host_virt;
	u32 pgnr;

	if (!frame) {
		dev_err(isp->dev, "%s: NULL frame pointer.\n", __func__);
		return -EINVAL;
	}

	host_virt = vma->vm_start;
	isp_virt = frame->data;
	atomisp_get_frame_pgnr(isp, frame, &pgnr);

	if (do_isp_mm_remap(isp, vma, isp_virt, host_virt, pgnr))
		return -EAGAIN;

	return 0;
}

int atomisp_videobuf_mmap_mapper(struct videobuf_queue *q,
	struct vm_area_struct *vma)
{
	u32 offset = vma->vm_pgoff << PAGE_SHIFT;
	int ret = -EINVAL, i;
	struct atomisp_device *isp =
		((struct atomisp_video_pipe *)(q->priv_data))->isp;
	struct videobuf_vmalloc_memory *vm_mem;
	struct videobuf_mapping *map;

	MAGIC_CHECK(q->int_ops->magic, MAGIC_QTYPE_OPS);
	if (!(vma->vm_flags & VM_WRITE) || !(vma->vm_flags & VM_SHARED)) {
		dev_err(isp->dev, "map appl bug: PROT_WRITE and MAP_SHARED are required\n");
		return -EINVAL;
	}

	mutex_lock(&q->vb_lock);
	for (i = 0; i < VIDEO_MAX_FRAME; i++) {
		struct videobuf_buffer *buf = q->bufs[i];
		if (buf == NULL)
			continue;

		map = kzalloc(sizeof(struct videobuf_mapping), GFP_KERNEL);
		if (!map) {
			mutex_unlock(&q->vb_lock);
			return -ENOMEM;
		}

		buf->map = map;
		map->q = q;

		buf->baddr = vma->vm_start;

		if (buf && buf->memory == V4L2_MEMORY_MMAP &&
		    buf->boff == offset) {
			vm_mem = buf->priv;
			ret = frame_mmap(isp, vm_mem->vaddr, vma);
			vma->vm_flags |= VM_IO|VM_DONTEXPAND|VM_DONTDUMP;
			break;
		}
	}
	mutex_unlock(&q->vb_lock);

	return ret;
}

/* The input frame contains left and right padding that need to be removed.
 * There is always ISP_LEFT_PAD padding on the left side.
 * There is also padding on the right (padded_width - width).
 */
static int remove_pad_from_frame(struct atomisp_device *isp,
		struct atomisp_css_frame *in_frame, __u32 width, __u32 height)
{
	unsigned int i;
	unsigned short *buffer;
	int ret = 0;
	ia_css_ptr load = in_frame->data;
	ia_css_ptr store = load;

	buffer = kmalloc(width*sizeof(load), GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	load += ISP_LEFT_PAD;
	for (i = 0; i < height; i++) {
		ret = hmm_load(load, buffer, width*sizeof(load));
		if (ret < 0)
			goto remove_pad_error;

		ret = hmm_store(store, buffer, width*sizeof(store));
		if (ret < 0)
			goto remove_pad_error;

		load  += in_frame->info.padded_width;
		store += width;
	}

remove_pad_error:
	kfree(buffer);
	return ret;
}

static int atomisp_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	struct atomisp_css_frame *raw_virt_addr;
	u32 start = vma->vm_start;
	u32 end = vma->vm_end;
	u32 size = end - start;
	u32 origin_size, new_size;
	int ret;

	if (!(vma->vm_flags & (VM_WRITE | VM_READ)))
		return -EACCES;

	rt_mutex_lock(&isp->mutex);

	if (!(vma->vm_flags & VM_SHARED)) {
		/* Map private buffer.
		 * Set VM_SHARED to the flags since we need
		 * to map the buffer page by page.
		 * Without VM_SHARED, remap_pfn_range() treats
		 * this kind of mapping as invalid.
		 */
		vma->vm_flags |= VM_SHARED;
		ret = hmm_mmap(vma, vma->vm_pgoff << PAGE_SHIFT);
		rt_mutex_unlock(&isp->mutex);
		return ret;
	}

	/* mmap for ISP offline raw data */
	if (atomisp_subdev_source_pad(vdev)
	    == ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE &&
	    vma->vm_pgoff == (ISP_PARAM_MMAP_OFFSET >> PAGE_SHIFT)) {
		new_size = pipe->pix.width * pipe->pix.height * 2;
		if (asd->params.online_process != 0) {
			ret = -EINVAL;
			goto error;
		}
		raw_virt_addr = asd->raw_output_frame;
		if (raw_virt_addr == NULL) {
			dev_err(isp->dev, "Failed to request RAW frame\n");
			ret = -EINVAL;
			goto error;
		}

		ret = remove_pad_from_frame(isp, raw_virt_addr,
				      pipe->pix.width, pipe->pix.height);
		if (ret < 0) {
			dev_err(isp->dev, "remove pad failed.\n");
			goto error;
		}
		origin_size = raw_virt_addr->data_bytes;
		raw_virt_addr->data_bytes = new_size;

		if (size != PAGE_ALIGN(new_size)) {
			dev_err(isp->dev, "incorrect size for mmap ISP  Raw Frame\n");
			ret = -EINVAL;
			goto error;
		}

		if (frame_mmap(isp, raw_virt_addr, vma)) {
			dev_err(isp->dev, "frame_mmap failed.\n");
			raw_virt_addr->data_bytes = origin_size;
			ret = -EAGAIN;
			goto error;
		}
		raw_virt_addr->data_bytes = origin_size;
		vma->vm_flags |= VM_IO|VM_DONTEXPAND|VM_DONTDUMP;
		rt_mutex_unlock(&isp->mutex);
		return 0;
	}

	/*
	 * mmap for normal frames
	 */
	if (size != pipe->pix.sizeimage) {
		dev_err(isp->dev, "incorrect size for mmap ISP frames\n");
		ret = -EINVAL;
		goto error;
	}
	rt_mutex_unlock(&isp->mutex);

	return atomisp_videobuf_mmap_mapper(&pipe->capq, vma);

error:
	rt_mutex_unlock(&isp->mutex);

	return ret;
}

static int atomisp_file_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);

	return videobuf_mmap_mapper(&pipe->outq, vma);
}

static __poll_t atomisp_poll(struct file *file,
				 struct poll_table_struct *pt)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);

	rt_mutex_lock(&isp->mutex);
	if (pipe->capq.streaming != 1) {
		rt_mutex_unlock(&isp->mutex);
		return EPOLLERR;
	}
	rt_mutex_unlock(&isp->mutex);

	return videobuf_poll_stream(file, &pipe->capq, pt);
}

const struct v4l2_file_operations atomisp_fops = {
	.owner = THIS_MODULE,
	.open = atomisp_open,
	.release = atomisp_release,
	.mmap = atomisp_mmap,
	.unlocked_ioctl = video_ioctl2,
#ifdef CONFIG_COMPAT
	/*
	 * There are problems with this code. Disable this for now.
	.compat_ioctl32 = atomisp_compat_ioctl32,
	 */
#endif
	.poll = atomisp_poll,
};

const struct v4l2_file_operations atomisp_file_fops = {
	.owner = THIS_MODULE,
	.open = atomisp_open,
	.release = atomisp_release,
	.mmap = atomisp_file_mmap,
	.unlocked_ioctl = video_ioctl2,
#ifdef CONFIG_COMPAT
	/*
	 * There are problems with this code. Disable this for now.
	.compat_ioctl32 = atomisp_compat_ioctl32,
	 */
#endif
	.poll = atomisp_poll,
};

{
	v4l2_ctrl_s_ctrl(asd->run_mode, ATOMISP_RUN_MODE_STILL_CAPTURE);
	memset(&asd->params.css_param, 0, sizeof(asd->params.css_param));
	asd->params.color_effect = V4L2_COLORFX_NONE;
	asd->params.bad_pixel_en = true;
	asd->params.gdc_cac_en = false;
	asd->params.video_dis_en = false;
	asd->params.sc_en = false;
	asd->params.fpn_en = false;
	asd->params.xnr_en = false;
	asd->params.false_color = 0;
	asd->params.online_process = 1;
	asd->params.yuv_ds_en = 0;
	/* s3a grid not enabled for any pipe */
	asd->params.s3a_enabled_pipe = CSS_PIPE_ID_NUM;

	asd->params.offline_parm.num_captures = 1;
	asd->params.offline_parm.skip_frames = 0;
	asd->params.offline_parm.offset = 0;
	asd->delayed_init = ATOMISP_DELAYED_INIT_NOT_QUEUED;
	/* Add for channel */
	asd->input_curr = 0;

	asd->mipi_frame_size = 0;
	asd->copy_mode = false;
	asd->yuvpp_mode = false;

	asd->stream_prepared = false;
	asd->high_speed_mode = false;
	asd->sensor_array_res.height = 0;
	asd->sensor_array_res.width = 0;
	atomisp_css_init_struct(asd);
}
/*
 * file operation functions
 */
static unsigned int atomisp_subdev_users(struct atomisp_sub_device *asd)
{
	return asd->video_out_preview.users +
	       asd->video_out_vf.users +
	       asd->video_out_capture.users +
	       asd->video_out_video_capture.users +
	       asd->video_acc.users +
	       asd->video_in.users;
}

unsigned int atomisp_dev_users(struct atomisp_device *isp)
{
	unsigned int i, sum;
	for (i = 0, sum = 0; i < isp->num_of_streams; i++)
		sum += atomisp_subdev_users(&isp->asd[i]);

	return sum;
}

static int atomisp_open(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = NULL;
	struct atomisp_acc_pipe *acc_pipe = NULL;
	struct atomisp_sub_device *asd;
	bool acc_node = false;
	int ret;

	dev_dbg(isp->dev, "open device %s\n", vdev->name);

	rt_mutex_lock(&isp->mutex);

	acc_node = !strncmp(vdev->name, "ATOMISP ISP ACC",
			sizeof(vdev->name));
	if (acc_node) {
		acc_pipe = atomisp_to_acc_pipe(vdev);
		asd = acc_pipe->asd;
	} else {
		pipe = atomisp_to_video_pipe(vdev);
		asd = pipe->asd;
	}
	asd->subdev.devnode = vdev;
	/* Deferred firmware loading case. */
	if (isp->css_env.isp_css_fw.bytes == 0) {
		isp->firmware = atomisp_load_firmware(isp);
		if (!isp->firmware) {
			dev_err(isp->dev, "Failed to load ISP firmware.\n");
			ret = -ENOENT;
			goto error;
		}
		ret = atomisp_css_load_firmware(isp);
		if (ret) {
			dev_err(isp->dev, "Failed to init css.\n");
			goto error;
		}
		/* No need to keep FW in memory anymore. */
		release_firmware(isp->firmware);
		isp->firmware = NULL;
		isp->css_env.isp_css_fw.data = NULL;
	}

	if (acc_node && acc_pipe->users) {
		dev_dbg(isp->dev, "acc node already opened\n");
		rt_mutex_unlock(&isp->mutex);
		return -EBUSY;
	} else if (acc_node) {
		goto dev_init;
	}

	if (!isp->input_cnt) {
		dev_err(isp->dev, "no camera attached\n");
		ret = -EINVAL;
		goto error;
	}

	/*
	 * atomisp does not allow multiple open
	 */
	if (pipe->users) {
		dev_dbg(isp->dev, "video node already opened\n");
		rt_mutex_unlock(&isp->mutex);
		return -EBUSY;
	}

	ret = atomisp_init_pipe(pipe);
	if (ret)
		goto error;

dev_init:
	if (atomisp_dev_users(isp)) {
		dev_dbg(isp->dev, "skip init isp in open\n");
		goto init_subdev;
	}

	/* runtime power management, turn on ISP */
	ret = pm_runtime_get_sync(vdev->v4l2_dev->dev);
	if (ret < 0) {
		dev_err(isp->dev, "Failed to power on device\n");
		goto error;
	}

	if (dypool_enable) {
		ret = hmm_pool_register(dypool_pgnr, HMM_POOL_TYPE_DYNAMIC);
		if (ret)
			dev_err(isp->dev, "Failed to register dynamic memory pool.\n");
	}

	/* Init ISP */
	if (atomisp_css_init(isp)) {
		ret = -EINVAL;
		/* Need to clean up CSS init if it fails. */
		goto css_error;
	}

	atomisp_dev_init_struct(isp);

	ret = v4l2_subdev_call(isp->flash, core, s_power, 1);
	if (ret < 0 && ret != -ENODEV && ret != -ENOIOCTLCMD) {
		dev_err(isp->dev, "Failed to power-on flash\n");
		goto css_error;
	}

init_subdev:
	if (atomisp_subdev_users(asd))
		goto done;

	atomisp_subdev_init_struct(asd);

done:

	if (acc_node)
		acc_pipe->users++;
	else
		pipe->users++;
	rt_mutex_unlock(&isp->mutex);
	return 0;

css_error:
	atomisp_css_uninit(isp);
error:
	hmm_pool_unregister(HMM_POOL_TYPE_DYNAMIC);
	pm_runtime_put(vdev->v4l2_dev->dev);
	rt_mutex_unlock(&isp->mutex);
	return ret;
}

static int atomisp_release(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe;
	struct atomisp_acc_pipe *acc_pipe;
	struct atomisp_sub_device *asd;
	bool acc_node;
	struct v4l2_requestbuffers req;
	struct v4l2_subdev_fh fh;
	struct v4l2_rect clear_compose = {0};
	int ret = 0;

	v4l2_fh_init(&fh.vfh, vdev);

	req.count = 0;
	if (isp == NULL)
		return -EBADF;

	mutex_lock(&isp->streamoff_mutex);
	rt_mutex_lock(&isp->mutex);

	dev_dbg(isp->dev, "release device %s\n", vdev->name);
	acc_node = !strncmp(vdev->name, "ATOMISP ISP ACC",
			sizeof(vdev->name));
	if (acc_node) {
		acc_pipe = atomisp_to_acc_pipe(vdev);
		asd = acc_pipe->asd;
	} else {
		pipe = atomisp_to_video_pipe(vdev);
		asd = pipe->asd;
	}
	asd->subdev.devnode = vdev;
	if (acc_node) {
		acc_pipe->users--;
		goto subdev_uninit;
	}
	pipe->users--;

	if (pipe->capq.streaming)
		dev_warn(isp->dev,
				"%s: ISP still streaming while closing!",
				__func__);

	if (pipe->capq.streaming &&
	    __atomisp_streamoff(file, NULL, V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
		dev_err(isp->dev,
			"atomisp_streamoff failed on release, driver bug");
		goto done;
	}

	if (pipe->users)
		goto done;

	if (__atomisp_reqbufs(file, NULL, &req)) {
		dev_err(isp->dev,
			"atomisp_reqbufs failed on release, driver bug");
		goto done;
	}

	if (pipe->outq.bufs[0]) {
		mutex_lock(&pipe->outq.vb_lock);
		videobuf_queue_cancel(&pipe->outq);
		mutex_unlock(&pipe->outq.vb_lock);
	}

	/*
	 * A little trick here:
	 * file injection input resolution is recorded in the sink pad,
	 * therefore can not be cleared when releaseing one device node.
	 * The sink pad setting can only be cleared when all device nodes
	 * get released.
	 */
	if (!isp->sw_contex.file_input && asd->fmt_auto->val) {
		struct v4l2_mbus_framefmt isp_sink_fmt = { 0 };
		atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SINK, &isp_sink_fmt);
	}
subdev_uninit:
	if (atomisp_subdev_users(asd))
		goto done;

	/* clear the sink pad for file input */
	if (isp->sw_contex.file_input && asd->fmt_auto->val) {
		struct v4l2_mbus_framefmt isp_sink_fmt = { 0 };
		atomisp_subdev_set_ffmt(&asd->subdev, fh.pad,
					V4L2_SUBDEV_FORMAT_ACTIVE,
					ATOMISP_SUBDEV_PAD_SINK, &isp_sink_fmt);
	}

	atomisp_css_free_stat_buffers(asd);
	atomisp_free_internal_buffers(asd);
	ret = v4l2_subdev_call(isp->inputs[asd->input_curr].camera,
				       core, s_power, 0);
	if (ret)
		dev_warn(isp->dev, "Failed to power-off sensor\n");

	/* clear the asd field to show this camera is not used */
	isp->inputs[asd->input_curr].asd = NULL;
	asd->streaming = ATOMISP_DEVICE_STREAMING_DISABLED;

	if (atomisp_dev_users(isp))
		goto done;

	atomisp_acc_release(asd);

	atomisp_destroy_pipes_stream_force(asd);
	atomisp_css_uninit(isp);

	if (defer_fw_load) {
		atomisp_css_unload_firmware(isp);
		isp->css_env.isp_css_fw.data = NULL;
		isp->css_env.isp_css_fw.bytes = 0;
	}

	hmm_pool_unregister(HMM_POOL_TYPE_DYNAMIC);

	ret = v4l2_subdev_call(isp->flash, core, s_power, 0);
	if (ret < 0 && ret != -ENODEV && ret != -ENOIOCTLCMD)
		dev_warn(isp->dev, "Failed to power-off flash\n");

	if (pm_runtime_put_sync(vdev->v4l2_dev->dev) < 0)
		dev_err(isp->dev, "Failed to power off device\n");

done:
	if (!acc_node) {
		atomisp_subdev_set_selection(&asd->subdev, fh.pad,
				V4L2_SUBDEV_FORMAT_ACTIVE,
				atomisp_subdev_source_pad(vdev),
				V4L2_SEL_TGT_COMPOSE, 0,
				&clear_compose);
	}
	rt_mutex_unlock(&isp->mutex);
	mutex_unlock(&isp->streamoff_mutex);

	return 0;
}

/*
 * Memory help functions for image frame and private parameters
 */
static int do_isp_mm_remap(struct atomisp_device *isp,
			   struct vm_area_struct *vma,
			   ia_css_ptr isp_virt, u32 host_virt, u32 pgnr)
{
	u32 pfn;

	while (pgnr) {
		pfn = hmm_virt_to_phys(isp_virt) >> PAGE_SHIFT;
		if (remap_pfn_range(vma, host_virt, pfn,
				    PAGE_SIZE, PAGE_SHARED)) {
			dev_err(isp->dev, "remap_pfn_range err.\n");
			return -EAGAIN;
		}

		isp_virt += PAGE_SIZE;
		host_virt += PAGE_SIZE;
		pgnr--;
	}

	return 0;
}

static int frame_mmap(struct atomisp_device *isp,
	const struct atomisp_css_frame *frame, struct vm_area_struct *vma)
{
	ia_css_ptr isp_virt;
	u32 host_virt;
	u32 pgnr;

	if (!frame) {
		dev_err(isp->dev, "%s: NULL frame pointer.\n", __func__);
		return -EINVAL;
	}

	host_virt = vma->vm_start;
	isp_virt = frame->data;
	atomisp_get_frame_pgnr(isp, frame, &pgnr);

	if (do_isp_mm_remap(isp, vma, isp_virt, host_virt, pgnr))
		return -EAGAIN;

	return 0;
}

int atomisp_videobuf_mmap_mapper(struct videobuf_queue *q,
	struct vm_area_struct *vma)
{
	u32 offset = vma->vm_pgoff << PAGE_SHIFT;
	int ret = -EINVAL, i;
	struct atomisp_device *isp =
		((struct atomisp_video_pipe *)(q->priv_data))->isp;
	struct videobuf_vmalloc_memory *vm_mem;
	struct videobuf_mapping *map;

	MAGIC_CHECK(q->int_ops->magic, MAGIC_QTYPE_OPS);
	if (!(vma->vm_flags & VM_WRITE) || !(vma->vm_flags & VM_SHARED)) {
		dev_err(isp->dev, "map appl bug: PROT_WRITE and MAP_SHARED are required\n");
		return -EINVAL;
	}

	mutex_lock(&q->vb_lock);
	for (i = 0; i < VIDEO_MAX_FRAME; i++) {
		struct videobuf_buffer *buf = q->bufs[i];
		if (buf == NULL)
			continue;

		map = kzalloc(sizeof(struct videobuf_mapping), GFP_KERNEL);
		if (!map) {
			mutex_unlock(&q->vb_lock);
			return -ENOMEM;
		}

		buf->map = map;
		map->q = q;

		buf->baddr = vma->vm_start;

		if (buf && buf->memory == V4L2_MEMORY_MMAP &&
		    buf->boff == offset) {
			vm_mem = buf->priv;
			ret = frame_mmap(isp, vm_mem->vaddr, vma);
			vma->vm_flags |= VM_IO|VM_DONTEXPAND|VM_DONTDUMP;
			break;
		}
	}
	mutex_unlock(&q->vb_lock);

	return ret;
}

/* The input frame contains left and right padding that need to be removed.
 * There is always ISP_LEFT_PAD padding on the left side.
 * There is also padding on the right (padded_width - width).
 */
static int remove_pad_from_frame(struct atomisp_device *isp,
		struct atomisp_css_frame *in_frame, __u32 width, __u32 height)
{
	unsigned int i;
	unsigned short *buffer;
	int ret = 0;
	ia_css_ptr load = in_frame->data;
	ia_css_ptr store = load;

	buffer = kmalloc(width*sizeof(load), GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	load += ISP_LEFT_PAD;
	for (i = 0; i < height; i++) {
		ret = hmm_load(load, buffer, width*sizeof(load));
		if (ret < 0)
			goto remove_pad_error;

		ret = hmm_store(store, buffer, width*sizeof(store));
		if (ret < 0)
			goto remove_pad_error;

		load  += in_frame->info.padded_width;
		store += width;
	}

remove_pad_error:
	kfree(buffer);
	return ret;
}

static int atomisp_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);
	struct atomisp_sub_device *asd = pipe->asd;
	struct atomisp_css_frame *raw_virt_addr;
	u32 start = vma->vm_start;
	u32 end = vma->vm_end;
	u32 size = end - start;
	u32 origin_size, new_size;
	int ret;

	if (!(vma->vm_flags & (VM_WRITE | VM_READ)))
		return -EACCES;

	rt_mutex_lock(&isp->mutex);

	if (!(vma->vm_flags & VM_SHARED)) {
		/* Map private buffer.
		 * Set VM_SHARED to the flags since we need
		 * to map the buffer page by page.
		 * Without VM_SHARED, remap_pfn_range() treats
		 * this kind of mapping as invalid.
		 */
		vma->vm_flags |= VM_SHARED;
		ret = hmm_mmap(vma, vma->vm_pgoff << PAGE_SHIFT);
		rt_mutex_unlock(&isp->mutex);
		return ret;
	}

	/* mmap for ISP offline raw data */
	if (atomisp_subdev_source_pad(vdev)
	    == ATOMISP_SUBDEV_PAD_SOURCE_CAPTURE &&
	    vma->vm_pgoff == (ISP_PARAM_MMAP_OFFSET >> PAGE_SHIFT)) {
		new_size = pipe->pix.width * pipe->pix.height * 2;
		if (asd->params.online_process != 0) {
			ret = -EINVAL;
			goto error;
		}
		raw_virt_addr = asd->raw_output_frame;
		if (raw_virt_addr == NULL) {
			dev_err(isp->dev, "Failed to request RAW frame\n");
			ret = -EINVAL;
			goto error;
		}

		ret = remove_pad_from_frame(isp, raw_virt_addr,
				      pipe->pix.width, pipe->pix.height);
		if (ret < 0) {
			dev_err(isp->dev, "remove pad failed.\n");
			goto error;
		}
		origin_size = raw_virt_addr->data_bytes;
		raw_virt_addr->data_bytes = new_size;

		if (size != PAGE_ALIGN(new_size)) {
			dev_err(isp->dev, "incorrect size for mmap ISP  Raw Frame\n");
			ret = -EINVAL;
			goto error;
		}

		if (frame_mmap(isp, raw_virt_addr, vma)) {
			dev_err(isp->dev, "frame_mmap failed.\n");
			raw_virt_addr->data_bytes = origin_size;
			ret = -EAGAIN;
			goto error;
		}
		raw_virt_addr->data_bytes = origin_size;
		vma->vm_flags |= VM_IO|VM_DONTEXPAND|VM_DONTDUMP;
		rt_mutex_unlock(&isp->mutex);
		return 0;
	}

	/*
	 * mmap for normal frames
	 */
	if (size != pipe->pix.sizeimage) {
		dev_err(isp->dev, "incorrect size for mmap ISP frames\n");
		ret = -EINVAL;
		goto error;
	}
	rt_mutex_unlock(&isp->mutex);

	return atomisp_videobuf_mmap_mapper(&pipe->capq, vma);

error:
	rt_mutex_unlock(&isp->mutex);

	return ret;
}

static int atomisp_file_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);

	return videobuf_mmap_mapper(&pipe->outq, vma);
}

static __poll_t atomisp_poll(struct file *file,
				 struct poll_table_struct *pt)
{
	struct video_device *vdev = video_devdata(file);
	struct atomisp_device *isp = video_get_drvdata(vdev);
	struct atomisp_video_pipe *pipe = atomisp_to_video_pipe(vdev);

	rt_mutex_lock(&isp->mutex);
	if (pipe->capq.streaming != 1) {
		rt_mutex_unlock(&isp->mutex);
		return EPOLLERR;
	}
	rt_mutex_unlock(&isp->mutex);

	return videobuf_poll_stream(file, &pipe->capq, pt);
}

const struct v4l2_file_operations atomisp_fops = {
	.owner = THIS_MODULE,
	.open = atomisp_open,
	.release = atomisp_release,
	.mmap = atomisp_mmap,
	.unlocked_ioctl = video_ioctl2,
#ifdef CONFIG_COMPAT
	/*
	 * There are problems with this code. Disable this for now.
	.compat_ioctl32 = atomisp_compat_ioctl32,
	 */
#endif
	.poll = atomisp_poll,
};

const struct v4l2_file_operations atomisp_file_fops = {
	.owner = THIS_MODULE,
	.open = atomisp_open,
	.release = atomisp_release,
	.mmap = atomisp_file_mmap,
	.unlocked_ioctl = video_ioctl2,
#ifdef CONFIG_COMPAT
	/*
	 * There are problems with this code. Disable this for now.
	.compat_ioctl32 = atomisp_compat_ioctl32,
	 */
#endif
	.poll = atomisp_poll,
};
