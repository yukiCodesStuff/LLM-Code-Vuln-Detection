{
	struct amdgpu_dm_connector *connector = file_inode(f)->i_private;
	struct dc_link *link = connector->dc_link;
	struct dc *dc = (struct dc *)link->dc;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 40;
	long param[3];
	bool use_prefer_link_setting;
	struct link_training_settings link_lane_settings;
	int max_param_num = 3;
	uint8_t param_nums = 0;
	int r = 0;


	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);
	if (!wr_buf)
		return -ENOSPC;

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   (long *)param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}
	uint8_t custom_pattern[10] = {
			0x1f, 0x7c, 0xf0, 0xc1, 0x07,
			0x1f, 0x7c, 0xf0, 0xc1, 0x07
			};
	struct dc_link_settings prefer_link_settings = {LANE_COUNT_UNKNOWN,
			LINK_RATE_UNKNOWN, LINK_SPREAD_DISABLED};
	struct dc_link_settings cur_link_settings = {LANE_COUNT_UNKNOWN,
			LINK_RATE_UNKNOWN, LINK_SPREAD_DISABLED};
	struct link_training_settings link_training_settings;
	int i;

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);
	if (!wr_buf)
		return -ENOSPC;

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   (long *)param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   &param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	aconnector->dsc_settings.dsc_force_disable_passthrough = param;

	kfree(wr_buf);
	return 0;
}

#ifdef CONFIG_DRM_AMD_DC_HDCP
/*
 * Returns the HDCP capability of the Display (1.4 for now).
 *
 * NOTE* Not all HDMI displays report their HDCP caps even when they are capable.
 * Since its rare for a display to not be HDCP 1.4 capable, we set HDMI as always capable.
 *
 * Example usage: cat /sys/kernel/debug/dri/0/DP-1/hdcp_sink_capability
 *		or cat /sys/kernel/debug/dri/0/HDMI-A-1/hdcp_sink_capability
 */
static int hdcp_sink_capability_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct amdgpu_dm_connector *aconnector = to_amdgpu_dm_connector(connector);
	bool hdcp_cap, hdcp2_cap;

	if (connector->status != connector_status_connected)
		return -ENODEV;

	seq_printf(m, "%s:%d HDCP version: ", connector->name, connector->base.id);

	hdcp_cap = dc_link_is_hdcp14(aconnector->dc_link, aconnector->dc_sink->sink_signal);
	hdcp2_cap = dc_link_is_hdcp22(aconnector->dc_link, aconnector->dc_sink->sink_signal);


	if (hdcp_cap)
		seq_printf(m, "%s ", "HDCP1.4");
	if (hdcp2_cap)
		seq_printf(m, "%s ", "HDCP2.2");

	if (!hdcp_cap && !hdcp2_cap)
		seq_printf(m, "%s ", "None");

	seq_puts(m, "\n");

	return 0;
}
#endif

/*
 * Returns whether the connected display is internal and not hotpluggable.
 * Example usage: cat /sys/kernel/debug/dri/0/DP-1/internal_display
 */
static int internal_display_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct amdgpu_dm_connector *aconnector = to_amdgpu_dm_connector(connector);
	struct dc_link *link = aconnector->dc_link;

	seq_printf(m, "Internal: %u\n", link->is_internal_display);

	return 0;
}

/* function description
 *
 * generic SDP message access for testing
 *
 * debugfs sdp_message is located at /syskernel/debug/dri/0/DP-x
 *
 * SDP header
 * Hb0 : Secondary-Data Packet ID
 * Hb1 : Secondary-Data Packet type
 * Hb2 : Secondary-Data-packet-specific header, Byte 0
 * Hb3 : Secondary-Data-packet-specific header, Byte 1
 *
 * for using custom sdp message: input 4 bytes SDP header and 32 bytes raw data
 */
static ssize_t dp_sdp_message_debugfs_write(struct file *f, const char __user *buf,
				 size_t size, loff_t *pos)
{
	int r;
	uint8_t data[36];
	struct amdgpu_dm_connector *connector = file_inode(f)->i_private;
	struct dm_crtc_state *acrtc_state;
	uint32_t write_size = 36;

	if (connector->base.status != connector_status_connected)
		return -ENODEV;

	if (size == 0)
		return 0;

	acrtc_state = to_dm_crtc_state(connector->base.state->crtc->state);

	r = copy_from_user(data, buf, write_size);

	write_size -= r;

	dc_stream_send_dp_sdp(acrtc_state->stream, data, write_size);

	return write_size;
}

static ssize_t dp_dpcd_address_write(struct file *f, const char __user *buf,
				 size_t size, loff_t *pos)
{
	int r;
	struct amdgpu_dm_connector *connector = file_inode(f)->i_private;

	if (size < sizeof(connector->debugfs_dpcd_address))
		return -EINVAL;

	r = copy_from_user(&connector->debugfs_dpcd_address,
			buf, sizeof(connector->debugfs_dpcd_address));

	return size - r;
}

static ssize_t dp_dpcd_size_write(struct file *f, const char __user *buf,
				 size_t size, loff_t *pos)
{
	int r;
	struct amdgpu_dm_connector *connector = file_inode(f)->i_private;

	if (size < sizeof(connector->debugfs_dpcd_size))
		return -EINVAL;

	r = copy_from_user(&connector->debugfs_dpcd_size,
			buf, sizeof(connector->debugfs_dpcd_size));

	if (connector->debugfs_dpcd_size > 256)
		connector->debugfs_dpcd_size = 0;

	return size - r;
}

static ssize_t dp_dpcd_data_write(struct file *f, const char __user *buf,
				 size_t size, loff_t *pos)
{
	int r;
	char *data;
	struct amdgpu_dm_connector *connector = file_inode(f)->i_private;
	struct dc_link *link = connector->dc_link;
	uint32_t write_size = connector->debugfs_dpcd_size;

	if (!write_size || size < write_size)
		return -EINVAL;

	data = kzalloc(write_size, GFP_KERNEL);
	if (!data)
		return 0;

	r = copy_from_user(data, buf, write_size);

	dm_helpers_dp_write_dpcd(link->ctx, link,
			connector->debugfs_dpcd_address, data, write_size - r);
	kfree(data);
	return write_size - r;
}

static ssize_t dp_dpcd_data_read(struct file *f, char __user *buf,
				 size_t size, loff_t *pos)
{
	int r;
	char *data;
	struct amdgpu_dm_connector *connector = file_inode(f)->i_private;
	struct dc_link *link = connector->dc_link;
	uint32_t read_size = connector->debugfs_dpcd_size;

	if (!read_size || size < read_size)
		return 0;

	data = kzalloc(read_size, GFP_KERNEL);
	if (!data)
		return 0;

	dm_helpers_dp_read_dpcd(link->ctx, link,
			connector->debugfs_dpcd_address, data, read_size);

	r = copy_to_user(buf, data, read_size);

	kfree(data);
	return read_size - r;
}

/* function: Read link's DSC & FEC capabilities
 *
 *
 * Access it with the following command (you need to specify
 * connector like DP-1):
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dp_dsc_fec_support
 *
 */
static int dp_dsc_fec_support_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct drm_modeset_acquire_ctx ctx;
	struct drm_device *dev = connector->dev;
	struct amdgpu_dm_connector *aconnector = to_amdgpu_dm_connector(connector);
	int ret = 0;
	bool try_again = false;
	bool is_fec_supported = false;
	bool is_dsc_supported = false;
	struct dpcd_caps dpcd_caps;

	drm_modeset_acquire_init(&ctx, DRM_MODESET_ACQUIRE_INTERRUPTIBLE);
	do {
		try_again = false;
		ret = drm_modeset_lock(&dev->mode_config.connection_mutex, &ctx);
		if (ret) {
			if (ret == -EDEADLK) {
				ret = drm_modeset_backoff(&ctx);
				if (!ret) {
					try_again = true;
					continue;
				}
			}
			break;
		}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
						(long *)param, buf,
						max_param_num,
						&param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param[0] == 1) {
		mutex_lock(&aconnector->hpd_lock);

		if (!dc_link_detect_sink(aconnector->dc_link, &new_connection_type) &&
			new_connection_type != dc_connection_none)
			goto unlock;

		if (!dc_link_detect(aconnector->dc_link, DETECT_REASON_HPD))
			goto unlock;

		amdgpu_dm_update_connector_after_detect(aconnector);

		drm_modeset_lock_all(dev);
		dm_restore_drm_connector_state(dev, connector);
		drm_modeset_unlock_all(dev);

		drm_kms_helper_hotplug_event(dev);
	} else if (param[0] == 0) {
		if (!aconnector->dc_link)
			goto unlock;

		link = aconnector->dc_link;

		if (link->local_sink) {
			dc_sink_release(link->local_sink);
			link->local_sink = NULL;
		}

		link->dpcd_sink_count = 0;
		link->type = dc_connection_none;
		link->dongle_max_pix_clk = 0;

		amdgpu_dm_update_connector_after_detect(aconnector);

		drm_modeset_lock_all(dev);
		dm_restore_drm_connector_state(dev, connector);
		drm_modeset_unlock_all(dev);

		drm_kms_helper_hotplug_event(dev);
	}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	if (param[0] == 1)
		aconnector->dsc_settings.dsc_force_enable = DSC_CLK_FORCE_ENABLE;
	else if (param[0] == 2)
		aconnector->dsc_settings.dsc_force_enable = DSC_CLK_FORCE_DISABLE;
	else
		aconnector->dsc_settings.dsc_force_enable = DSC_CLK_FORCE_DEFAULT;

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC slice width parameter on the connector
 *
 * The read function: dp_dsc_slice_width_read
 * returns dsc slice width used in the current configuration
 * The return is an integer: 0 or other positive number
 *
 * Access the status with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_width
 *
 * 0 - means that DSC is disabled
 *
 * Any other number more than zero represents the
 * slice width currently used by DSC in pixels
 *
 */
static ssize_t dp_dsc_slice_width_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_width);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: write DSC slice width parameter
 *
 * The write function: dp_dsc_slice_width_write
 * overwrites automatically generated DSC configuration
 * of slice width.
 *
 * The user has to write the slice width divisible by the
 * picture width.
 *
 * Also the user has to write width in hexidecimal
 * rather than in decimal.
 *
 * Writing DSC settings is done with the following command:
 * - To force overwrite slice width: (example sets to 1920 pixels)
 *
 *	echo 0x780 > /sys/kernel/debug/dri/0/DP-X/dsc_slice_width
 *
 *  - To stop overwriting and let driver find the optimal size,
 * set the width to zero:
 *
 *	echo 0x0 > /sys/kernel/debug/dri/0/DP-X/dsc_slice_width
 *
 */
static ssize_t dp_dsc_slice_width_write(struct file *f, const char __user *buf,
				     size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct pipe_ctx *pipe_ctx;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct drm_crtc *crtc = NULL;
	struct dm_crtc_state *dm_crtc_state = NULL;
	int i;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 42;
	int max_param_num = 1;
	long param[1] = {0};
	uint8_t param_nums = 0;

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);

	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Safely get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	if (param[0] > 0)
		aconnector->dsc_settings.dsc_num_slices_h = DIV_ROUND_UP(
					pipe_ctx->stream->timing.h_addressable,
					param[0]);
	else
		aconnector->dsc_settings.dsc_num_slices_h = 0;

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC slice height parameter on the connector
 *
 * The read function: dp_dsc_slice_height_read
 * returns dsc slice height used in the current configuration
 * The return is an integer: 0 or other positive number
 *
 * Access the status with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_height
 *
 * 0 - means that DSC is disabled
 *
 * Any other number more than zero represents the
 * slice height currently used by DSC in pixels
 *
 */
static ssize_t dp_dsc_slice_height_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_height);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: write DSC slice height parameter
 *
 * The write function: dp_dsc_slice_height_write
 * overwrites automatically generated DSC configuration
 * of slice height.
 *
 * The user has to write the slice height divisible by the
 * picture height.
 *
 * Also the user has to write height in hexidecimal
 * rather than in decimal.
 *
 * Writing DSC settings is done with the following command:
 * - To force overwrite slice height (example sets to 128 pixels):
 *
 *	echo 0x80 > /sys/kernel/debug/dri/0/DP-X/dsc_slice_height
 *
 *  - To stop overwriting and let driver find the optimal size,
 * set the height to zero:
 *
 *	echo 0x0 > /sys/kernel/debug/dri/0/DP-X/dsc_slice_height
 *
 */
static ssize_t dp_dsc_slice_height_write(struct file *f, const char __user *buf,
				     size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct drm_crtc *crtc = NULL;
	struct dm_crtc_state *dm_crtc_state = NULL;
	struct pipe_ctx *pipe_ctx;
	int i;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 42;
	int max_param_num = 1;
	uint8_t param_nums = 0;
	long param[1] = {0};

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);

	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	if (param[0] > 0)
		aconnector->dsc_settings.dsc_num_slices_v = DIV_ROUND_UP(
					pipe_ctx->stream->timing.v_addressable,
					param[0]);
	else
		aconnector->dsc_settings.dsc_num_slices_v = 0;

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC target rate on the connector in bits per pixel
 *
 * The read function: dp_dsc_bits_per_pixel_read
 * returns target rate of compression in bits per pixel
 * The return is an integer: 0 or other positive integer
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 *  0 - means that DSC is disabled
 */
static ssize_t dp_dsc_bits_per_pixel_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_bits_per_pixel);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: write DSC target rate in bits per pixel
 *
 * The write function: dp_dsc_bits_per_pixel_write
 * overwrites automatically generated DSC configuration
 * of DSC target bit rate.
 *
 * Also the user has to write bpp in hexidecimal
 * rather than in decimal.
 *
 * Writing DSC settings is done with the following command:
 * - To force overwrite rate (example sets to 256 bpp x 1/16):
 *
 *	echo 0x100 > /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 *  - To stop overwriting and let driver find the optimal rate,
 * set the rate to zero:
 *
 *	echo 0x0 > /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 */
static ssize_t dp_dsc_bits_per_pixel_write(struct file *f, const char __user *buf,
				     size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct drm_crtc *crtc = NULL;
	struct dm_crtc_state *dm_crtc_state = NULL;
	struct pipe_ctx *pipe_ctx;
	int i;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 42;
	int max_param_num = 1;
	uint8_t param_nums = 0;
	long param[1] = {0};

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);

	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	aconnector->dsc_settings.dsc_bits_per_pixel = param[0];

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC picture width parameter on the connector
 *
 * The read function: dp_dsc_pic_width_read
 * returns dsc picture width used in the current configuration
 * It is the same as h_addressable of the current
 * display's timing
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_pic_width
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_pic_width_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_width);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

static ssize_t dp_dsc_pic_height_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_height);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC chunk size parameter on the connector
 *
 * The read function: dp_dsc_chunk_size_read
 * returns dsc chunk size set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_chunk_size
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_chunk_size_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_chunk_size);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC slice bpg offset on the connector
 *
 * The read function: dp_dsc_slice_bpg_offset_read
 * returns dsc bpg slice offset set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_bpg_offset
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_slice_bpg_offset_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_bpg_offset);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}


/*
 * function description: Read max_requested_bpc property from the connector
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/max_bpc
 *
 */
static ssize_t dp_max_bpc_read(struct file *f, char __user *buf,
		size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct dm_connector_state *state;
	ssize_t result = 0;
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	const uint32_t rd_buf_size = 10;
	int r;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	state = to_dm_connector_state(connector->state);

	rd_buf_ptr = rd_buf;
	snprintf(rd_buf_ptr, rd_buf_size,
		"%u\n",
		state->base.max_requested_bpc);

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r) {
			result = r; /* r = -EFAULT */
			goto unlock;
		}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Safely get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	if (param[0] > 0)
		aconnector->dsc_settings.dsc_num_slices_h = DIV_ROUND_UP(
					pipe_ctx->stream->timing.h_addressable,
					param[0]);
	else
		aconnector->dsc_settings.dsc_num_slices_h = 0;

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC slice height parameter on the connector
 *
 * The read function: dp_dsc_slice_height_read
 * returns dsc slice height used in the current configuration
 * The return is an integer: 0 or other positive number
 *
 * Access the status with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_height
 *
 * 0 - means that DSC is disabled
 *
 * Any other number more than zero represents the
 * slice height currently used by DSC in pixels
 *
 */
static ssize_t dp_dsc_slice_height_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_height);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: write DSC slice height parameter
 *
 * The write function: dp_dsc_slice_height_write
 * overwrites automatically generated DSC configuration
 * of slice height.
 *
 * The user has to write the slice height divisible by the
 * picture height.
 *
 * Also the user has to write height in hexidecimal
 * rather than in decimal.
 *
 * Writing DSC settings is done with the following command:
 * - To force overwrite slice height (example sets to 128 pixels):
 *
 *	echo 0x80 > /sys/kernel/debug/dri/0/DP-X/dsc_slice_height
 *
 *  - To stop overwriting and let driver find the optimal size,
 * set the height to zero:
 *
 *	echo 0x0 > /sys/kernel/debug/dri/0/DP-X/dsc_slice_height
 *
 */
static ssize_t dp_dsc_slice_height_write(struct file *f, const char __user *buf,
				     size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct drm_crtc *crtc = NULL;
	struct dm_crtc_state *dm_crtc_state = NULL;
	struct pipe_ctx *pipe_ctx;
	int i;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 42;
	int max_param_num = 1;
	uint8_t param_nums = 0;
	long param[1] = {0};

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);

	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	if (param[0] > 0)
		aconnector->dsc_settings.dsc_num_slices_v = DIV_ROUND_UP(
					pipe_ctx->stream->timing.v_addressable,
					param[0]);
	else
		aconnector->dsc_settings.dsc_num_slices_v = 0;

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC target rate on the connector in bits per pixel
 *
 * The read function: dp_dsc_bits_per_pixel_read
 * returns target rate of compression in bits per pixel
 * The return is an integer: 0 or other positive integer
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 *  0 - means that DSC is disabled
 */
static ssize_t dp_dsc_bits_per_pixel_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_bits_per_pixel);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: write DSC target rate in bits per pixel
 *
 * The write function: dp_dsc_bits_per_pixel_write
 * overwrites automatically generated DSC configuration
 * of DSC target bit rate.
 *
 * Also the user has to write bpp in hexidecimal
 * rather than in decimal.
 *
 * Writing DSC settings is done with the following command:
 * - To force overwrite rate (example sets to 256 bpp x 1/16):
 *
 *	echo 0x100 > /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 *  - To stop overwriting and let driver find the optimal rate,
 * set the rate to zero:
 *
 *	echo 0x0 > /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 */
static ssize_t dp_dsc_bits_per_pixel_write(struct file *f, const char __user *buf,
				     size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct drm_crtc *crtc = NULL;
	struct dm_crtc_state *dm_crtc_state = NULL;
	struct pipe_ctx *pipe_ctx;
	int i;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 42;
	int max_param_num = 1;
	uint8_t param_nums = 0;
	long param[1] = {0};

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);

	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	aconnector->dsc_settings.dsc_bits_per_pixel = param[0];

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC picture width parameter on the connector
 *
 * The read function: dp_dsc_pic_width_read
 * returns dsc picture width used in the current configuration
 * It is the same as h_addressable of the current
 * display's timing
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_pic_width
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_pic_width_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_width);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

static ssize_t dp_dsc_pic_height_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_height);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC chunk size parameter on the connector
 *
 * The read function: dp_dsc_chunk_size_read
 * returns dsc chunk size set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_chunk_size
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_chunk_size_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_chunk_size);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC slice bpg offset on the connector
 *
 * The read function: dp_dsc_slice_bpg_offset_read
 * returns dsc bpg slice offset set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_bpg_offset
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_slice_bpg_offset_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_bpg_offset);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}


/*
 * function description: Read max_requested_bpc property from the connector
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/max_bpc
 *
 */
static ssize_t dp_max_bpc_read(struct file *f, char __user *buf,
		size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct dm_connector_state *state;
	ssize_t result = 0;
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	const uint32_t rd_buf_size = 10;
	int r;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	state = to_dm_connector_state(connector->state);

	rd_buf_ptr = rd_buf;
	snprintf(rd_buf_ptr, rd_buf_size,
		"%u\n",
		state->base.max_requested_bpc);

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r) {
			result = r; /* r = -EFAULT */
			goto unlock;
		}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	if (param[0] > 0)
		aconnector->dsc_settings.dsc_num_slices_v = DIV_ROUND_UP(
					pipe_ctx->stream->timing.v_addressable,
					param[0]);
	else
		aconnector->dsc_settings.dsc_num_slices_v = 0;

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC target rate on the connector in bits per pixel
 *
 * The read function: dp_dsc_bits_per_pixel_read
 * returns target rate of compression in bits per pixel
 * The return is an integer: 0 or other positive integer
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 *  0 - means that DSC is disabled
 */
static ssize_t dp_dsc_bits_per_pixel_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_bits_per_pixel);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: write DSC target rate in bits per pixel
 *
 * The write function: dp_dsc_bits_per_pixel_write
 * overwrites automatically generated DSC configuration
 * of DSC target bit rate.
 *
 * Also the user has to write bpp in hexidecimal
 * rather than in decimal.
 *
 * Writing DSC settings is done with the following command:
 * - To force overwrite rate (example sets to 256 bpp x 1/16):
 *
 *	echo 0x100 > /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 *  - To stop overwriting and let driver find the optimal rate,
 * set the rate to zero:
 *
 *	echo 0x0 > /sys/kernel/debug/dri/0/DP-X/dsc_bits_per_pixel
 *
 */
static ssize_t dp_dsc_bits_per_pixel_write(struct file *f, const char __user *buf,
				     size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct drm_crtc *crtc = NULL;
	struct dm_crtc_state *dm_crtc_state = NULL;
	struct pipe_ctx *pipe_ctx;
	int i;
	char *wr_buf = NULL;
	uint32_t wr_buf_size = 42;
	int max_param_num = 1;
	uint8_t param_nums = 0;
	long param[1] = {0};

	if (size == 0)
		return -EINVAL;

	wr_buf = kcalloc(wr_buf_size, sizeof(char), GFP_KERNEL);

	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	aconnector->dsc_settings.dsc_bits_per_pixel = param[0];

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC picture width parameter on the connector
 *
 * The read function: dp_dsc_pic_width_read
 * returns dsc picture width used in the current configuration
 * It is the same as h_addressable of the current
 * display's timing
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_pic_width
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_pic_width_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_width);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

static ssize_t dp_dsc_pic_height_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_height);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC chunk size parameter on the connector
 *
 * The read function: dp_dsc_chunk_size_read
 * returns dsc chunk size set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_chunk_size
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_chunk_size_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_chunk_size);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC slice bpg offset on the connector
 *
 * The read function: dp_dsc_slice_bpg_offset_read
 * returns dsc bpg slice offset set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_bpg_offset
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_slice_bpg_offset_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_bpg_offset);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}


/*
 * function description: Read max_requested_bpc property from the connector
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/max_bpc
 *
 */
static ssize_t dp_max_bpc_read(struct file *f, char __user *buf,
		size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct dm_connector_state *state;
	ssize_t result = 0;
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	const uint32_t rd_buf_size = 10;
	int r;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	state = to_dm_connector_state(connector->state);

	rd_buf_ptr = rd_buf;
	snprintf(rd_buf_ptr, rd_buf_size,
		"%u\n",
		state->base.max_requested_bpc);

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r) {
			result = r; /* r = -EFAULT */
			goto unlock;
		}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx || !pipe_ctx->stream)
		goto done;

	// Get CRTC state
	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	crtc = connector->state->crtc;
	if (crtc == NULL)
		goto unlock;

	drm_modeset_lock(&crtc->mutex, NULL);
	if (crtc->state == NULL)
		goto unlock;

	dm_crtc_state = to_dm_crtc_state(crtc->state);
	if (dm_crtc_state->stream == NULL)
		goto unlock;

	aconnector->dsc_settings.dsc_bits_per_pixel = param[0];

	dm_crtc_state->dsc_force_changed = true;

unlock:
	if (crtc)
		drm_modeset_unlock(&crtc->mutex);
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

done:
	kfree(wr_buf);
	return size;
}

/* function: read DSC picture width parameter on the connector
 *
 * The read function: dp_dsc_pic_width_read
 * returns dsc picture width used in the current configuration
 * It is the same as h_addressable of the current
 * display's timing
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_pic_width
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_pic_width_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_width);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

static ssize_t dp_dsc_pic_height_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_pic_height);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC chunk size parameter on the connector
 *
 * The read function: dp_dsc_chunk_size_read
 * returns dsc chunk size set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_chunk_size
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_chunk_size_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_chunk_size);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

/* function: read DSC slice bpg offset on the connector
 *
 * The read function: dp_dsc_slice_bpg_offset_read
 * returns dsc bpg slice offset set in the current configuration
 * The value is calculated automatically by DSC code
 * and depends on slice parameters and bpp target rate
 * The return is an integer: 0 or other positive integer
 * If 0 then DSC is disabled.
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/dsc_slice_bpg_offset
 *
 * 0 - means that DSC is disabled
 */
static ssize_t dp_dsc_slice_bpg_offset_read(struct file *f, char __user *buf,
				    size_t size, loff_t *pos)
{
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct display_stream_compressor *dsc;
	struct dcn_dsc_state dsc_state = {0};
	const uint32_t rd_buf_size = 100;
	struct pipe_ctx *pipe_ctx;
	ssize_t result = 0;
	int i, r, str_len = 30;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	rd_buf_ptr = rd_buf;

	for (i = 0; i < MAX_PIPES; i++) {
		pipe_ctx = &aconnector->dc_link->dc->current_state->res_ctx.pipe_ctx[i];
			if (pipe_ctx && pipe_ctx->stream &&
			    pipe_ctx->stream->link == aconnector->dc_link)
				break;
	}

	if (!pipe_ctx)
		return -ENXIO;

	dsc = pipe_ctx->stream_res.dsc;
	if (dsc)
		dsc->funcs->dsc_read_state(dsc, &dsc_state);

	snprintf(rd_buf_ptr, str_len,
		"%d\n",
		dsc_state.dsc_slice_bpg_offset);
	rd_buf_ptr += str_len;

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */

		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}


/*
 * function description: Read max_requested_bpc property from the connector
 *
 * Access it with the following command:
 *
 *	cat /sys/kernel/debug/dri/0/DP-X/max_bpc
 *
 */
static ssize_t dp_max_bpc_read(struct file *f, char __user *buf,
		size_t size, loff_t *pos)
{
	struct amdgpu_dm_connector *aconnector = file_inode(f)->i_private;
	struct drm_connector *connector = &aconnector->base;
	struct drm_device *dev = connector->dev;
	struct dm_connector_state *state;
	ssize_t result = 0;
	char *rd_buf = NULL;
	char *rd_buf_ptr = NULL;
	const uint32_t rd_buf_size = 10;
	int r;

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);

	if (!rd_buf)
		return -ENOMEM;

	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	state = to_dm_connector_state(connector->state);

	rd_buf_ptr = rd_buf;
	snprintf(rd_buf_ptr, rd_buf_size,
		"%u\n",
		state->base.max_requested_bpc);

	while (size) {
		if (*pos >= rd_buf_size)
			break;

		r = put_user(*(rd_buf + result), buf);
		if (r) {
			result = r; /* r = -EFAULT */
			goto unlock;
		}
	if (!wr_buf) {
		DRM_DEBUG_DRIVER("no memory to allocate write buffer\n");
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   (long *)param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param_nums <= 0) {
		DRM_DEBUG_DRIVER("user data not be read\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	if (param[0] < 6 || param[0] > 16) {
		DRM_DEBUG_DRIVER("bad max_bpc value\n");
		kfree(wr_buf);
		return -EINVAL;
	}

	mutex_lock(&dev->mode_config.mutex);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);

	if (connector->state == NULL)
		goto unlock;

	state = to_dm_connector_state(connector->state);
	state->base.max_requested_bpc = param[0];
unlock:
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	mutex_unlock(&dev->mode_config.mutex);

	kfree(wr_buf);
	return size;
}

/*
 * Backlight at this moment.  Read only.
 * As written to display, taking ABM and backlight lut into account.
 * Ranges from 0x0 to 0x10000 (= 100% PWM)
 *
 * Example usage: cat /sys/kernel/debug/dri/0/eDP-1/current_backlight
 */
static int current_backlight_show(struct seq_file *m, void *unused)
{
	struct amdgpu_dm_connector *aconnector = to_amdgpu_dm_connector(m->private);
	struct dc_link *link = aconnector->dc_link;
	unsigned int backlight;

	backlight = dc_link_get_backlight_level(link);
	seq_printf(m, "0x%x\n", backlight);

	return 0;
}

/*
 * Backlight value that is being approached.  Read only.
 * As written to display, taking ABM and backlight lut into account.
 * Ranges from 0x0 to 0x10000 (= 100% PWM)
 *
 * Example usage: cat /sys/kernel/debug/dri/0/eDP-1/target_backlight
 */
static int target_backlight_show(struct seq_file *m, void *unused)
{
	struct amdgpu_dm_connector *aconnector = to_amdgpu_dm_connector(m->private);
	struct dc_link *link = aconnector->dc_link;
	unsigned int backlight;

	backlight = dc_link_get_target_backlight_pwm(link);
	seq_printf(m, "0x%x\n", backlight);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(dp_dsc_fec_support);
DEFINE_SHOW_ATTRIBUTE(dmub_fw_state);
DEFINE_SHOW_ATTRIBUTE(dmub_tracebuffer);
DEFINE_SHOW_ATTRIBUTE(output_bpc);
DEFINE_SHOW_ATTRIBUTE(dp_lttpr_status);
#ifdef CONFIG_DRM_AMD_DC_HDCP
DEFINE_SHOW_ATTRIBUTE(hdcp_sink_capability);
#endif
DEFINE_SHOW_ATTRIBUTE(internal_display);

static const struct file_operations dp_dsc_clock_en_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_clock_en_read,
	.write = dp_dsc_clock_en_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_slice_width_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_slice_width_read,
	.write = dp_dsc_slice_width_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_slice_height_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_slice_height_read,
	.write = dp_dsc_slice_height_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_bits_per_pixel_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_bits_per_pixel_read,
	.write = dp_dsc_bits_per_pixel_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_pic_width_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_pic_width_read,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_pic_height_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_pic_height_read,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_chunk_size_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_chunk_size_read,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_slice_bpg_offset_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dsc_slice_bpg_offset_read,
	.llseek = default_llseek
};

static const struct file_operations trigger_hotplug_debugfs_fops = {
	.owner = THIS_MODULE,
	.write = trigger_hotplug,
	.llseek = default_llseek
};

static const struct file_operations dp_link_settings_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_link_settings_read,
	.write = dp_link_settings_write,
	.llseek = default_llseek
};

static const struct file_operations dp_phy_settings_debugfs_fop = {
	.owner = THIS_MODULE,
	.read = dp_phy_settings_read,
	.write = dp_phy_settings_write,
	.llseek = default_llseek
};

static const struct file_operations dp_phy_test_pattern_fops = {
	.owner = THIS_MODULE,
	.write = dp_phy_test_pattern_debugfs_write,
	.llseek = default_llseek
};

static const struct file_operations sdp_message_fops = {
	.owner = THIS_MODULE,
	.write = dp_sdp_message_debugfs_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dpcd_address_debugfs_fops = {
	.owner = THIS_MODULE,
	.write = dp_dpcd_address_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dpcd_size_debugfs_fops = {
	.owner = THIS_MODULE,
	.write = dp_dpcd_size_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dpcd_data_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_dpcd_data_read,
	.write = dp_dpcd_data_write,
	.llseek = default_llseek
};

static const struct file_operations dp_max_bpc_debugfs_fops = {
	.owner = THIS_MODULE,
	.read = dp_max_bpc_read,
	.write = dp_max_bpc_write,
	.llseek = default_llseek
};

static const struct file_operations dp_dsc_disable_passthrough_debugfs_fops = {
	.owner = THIS_MODULE,
	.write = dp_dsc_passthrough_set,
	.llseek = default_llseek
};

static const struct {
	char *name;
	const struct file_operations *fops;
} dp_debugfs_entries[] = {
		{"link_settings", &dp_link_settings_debugfs_fops},
		{"phy_settings", &dp_phy_settings_debugfs_fop},
		{"lttpr_status", &dp_lttpr_status_fops},
		{"test_pattern", &dp_phy_test_pattern_fops},
#ifdef CONFIG_DRM_AMD_DC_HDCP
		{"hdcp_sink_capability", &hdcp_sink_capability_fops},
#endif
		{"sdp_message", &sdp_message_fops},
		{"aux_dpcd_address", &dp_dpcd_address_debugfs_fops},
		{"aux_dpcd_size", &dp_dpcd_size_debugfs_fops},
		{"aux_dpcd_data", &dp_dpcd_data_debugfs_fops},
		{"dsc_clock_en", &dp_dsc_clock_en_debugfs_fops},
		{"dsc_slice_width", &dp_dsc_slice_width_debugfs_fops},
		{"dsc_slice_height", &dp_dsc_slice_height_debugfs_fops},
		{"dsc_bits_per_pixel", &dp_dsc_bits_per_pixel_debugfs_fops},
		{"dsc_pic_width", &dp_dsc_pic_width_debugfs_fops},
		{"dsc_pic_height", &dp_dsc_pic_height_debugfs_fops},
		{"dsc_chunk_size", &dp_dsc_chunk_size_debugfs_fops},
		{"dsc_slice_bpg", &dp_dsc_slice_bpg_offset_debugfs_fops},
		{"dp_dsc_fec_support", &dp_dsc_fec_support_fops},
		{"max_bpc", &dp_max_bpc_debugfs_fops},
		{"dsc_disable_passthrough", &dp_dsc_disable_passthrough_debugfs_fops},
};

#ifdef CONFIG_DRM_AMD_DC_HDCP
static const struct {
	char *name;
	const struct file_operations *fops;
} hdmi_debugfs_entries[] = {
		{"hdcp_sink_capability", &hdcp_sink_capability_fops}
};
#endif
/*
 * Force YUV420 output if available from the given mode
 */
static int force_yuv420_output_set(void *data, u64 val)
{
	struct amdgpu_dm_connector *connector = data;

	connector->force_yuv420_output = (bool)val;

	return 0;
}

/*
 * Check if YUV420 is forced when available from the given mode
 */
static int force_yuv420_output_get(void *data, u64 *val)
{
	struct amdgpu_dm_connector *connector = data;

	*val = connector->force_yuv420_output;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(force_yuv420_output_fops, force_yuv420_output_get,
			 force_yuv420_output_set, "%llu\n");

/*
 *  Read PSR state
 */
static int psr_get(void *data, u64 *val)
{
	struct amdgpu_dm_connector *connector = data;
	struct dc_link *link = connector->dc_link;
	enum dc_psr_state state = PSR_STATE0;

	dc_link_get_psr_state(link, &state);

	*val = state;

	return 0;
}

/*
 * Set dmcub trace event IRQ enable or disable.
 * Usage to enable dmcub trace event IRQ: echo 1 > /sys/kernel/debug/dri/0/amdgpu_dm_dmcub_trace_event_en
 * Usage to disable dmcub trace event IRQ: echo 0 > /sys/kernel/debug/dri/0/amdgpu_dm_dmcub_trace_event_en
 */
static int dmcub_trace_event_state_set(void *data, u64 val)
{
	struct amdgpu_device *adev = data;

	if (val == 1 || val == 0) {
		dc_dmub_trace_event_control(adev->dm.dc, val);
		adev->dm.dmcub_trace_event_en = (bool)val;
	} else
		return 0;

	return 0;
}

/*
 * The interface doesn't need get function, so it will return the
 * value of zero
 * Usage: cat /sys/kernel/debug/dri/0/amdgpu_dm_dmcub_trace_event_en
 */
static int dmcub_trace_event_state_get(void *data, u64 *val)
{
	struct amdgpu_device *adev = data;

	*val = adev->dm.dmcub_trace_event_en;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(dmcub_trace_event_state_fops, dmcub_trace_event_state_get,
			 dmcub_trace_event_state_set, "%llu\n");

DEFINE_DEBUGFS_ATTRIBUTE(psr_fops, psr_get, NULL, "%llu\n");

DEFINE_SHOW_ATTRIBUTE(current_backlight);
DEFINE_SHOW_ATTRIBUTE(target_backlight);

static const struct {
	char *name;
	const struct file_operations *fops;
} connector_debugfs_entries[] = {
		{"force_yuv420_output", &force_yuv420_output_fops},
		{"output_bpc", &output_bpc_fops},
		{"trigger_hotplug", &trigger_hotplug_debugfs_fops},
		{"internal_display", &internal_display_fops}
};

void connector_debugfs_init(struct amdgpu_dm_connector *connector)
{
	int i;
	struct dentry *dir = connector->base.debugfs_entry;

	if (connector->base.connector_type == DRM_MODE_CONNECTOR_DisplayPort ||
	    connector->base.connector_type == DRM_MODE_CONNECTOR_eDP) {
		for (i = 0; i < ARRAY_SIZE(dp_debugfs_entries); i++) {
			debugfs_create_file(dp_debugfs_entries[i].name,
					    0644, dir, connector,
					    dp_debugfs_entries[i].fops);
		}
	}
	if (connector->base.connector_type == DRM_MODE_CONNECTOR_eDP) {
		debugfs_create_file_unsafe("psr_state", 0444, dir, connector, &psr_fops);
		debugfs_create_file("amdgpu_current_backlight_pwm", 0444, dir, connector,
				    &current_backlight_fops);
		debugfs_create_file("amdgpu_target_backlight_pwm", 0444, dir, connector,
				    &target_backlight_fops);
	}

	for (i = 0; i < ARRAY_SIZE(connector_debugfs_entries); i++) {
		debugfs_create_file(connector_debugfs_entries[i].name,
				    0644, dir, connector,
				    connector_debugfs_entries[i].fops);
	}

	connector->debugfs_dpcd_address = 0;
	connector->debugfs_dpcd_size = 0;

#ifdef CONFIG_DRM_AMD_DC_HDCP
	if (connector->base.connector_type == DRM_MODE_CONNECTOR_HDMIA) {
		for (i = 0; i < ARRAY_SIZE(hdmi_debugfs_entries); i++) {
			debugfs_create_file(hdmi_debugfs_entries[i].name,
					    0644, dir, connector,
					    hdmi_debugfs_entries[i].fops);
		}
	}
#endif
}

#ifdef CONFIG_DRM_AMD_SECURE_DISPLAY
/*
 * Set crc window coordinate x start
 */
static int crc_win_x_start_set(void *data, u64 val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	acrtc->dm_irq_params.crc_window.x_start = (uint16_t) val;
	acrtc->dm_irq_params.crc_window.update_win = false;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

/*
 * Get crc window coordinate x start
 */
static int crc_win_x_start_get(void *data, u64 *val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	*val = acrtc->dm_irq_params.crc_window.x_start;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(crc_win_x_start_fops, crc_win_x_start_get,
			 crc_win_x_start_set, "%llu\n");


/*
 * Set crc window coordinate y start
 */
static int crc_win_y_start_set(void *data, u64 val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	acrtc->dm_irq_params.crc_window.y_start = (uint16_t) val;
	acrtc->dm_irq_params.crc_window.update_win = false;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

/*
 * Get crc window coordinate y start
 */
static int crc_win_y_start_get(void *data, u64 *val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	*val = acrtc->dm_irq_params.crc_window.y_start;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(crc_win_y_start_fops, crc_win_y_start_get,
			 crc_win_y_start_set, "%llu\n");

/*
 * Set crc window coordinate x end
 */
static int crc_win_x_end_set(void *data, u64 val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	acrtc->dm_irq_params.crc_window.x_end = (uint16_t) val;
	acrtc->dm_irq_params.crc_window.update_win = false;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

/*
 * Get crc window coordinate x end
 */
static int crc_win_x_end_get(void *data, u64 *val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	*val = acrtc->dm_irq_params.crc_window.x_end;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(crc_win_x_end_fops, crc_win_x_end_get,
			 crc_win_x_end_set, "%llu\n");

/*
 * Set crc window coordinate y end
 */
static int crc_win_y_end_set(void *data, u64 val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	acrtc->dm_irq_params.crc_window.y_end = (uint16_t) val;
	acrtc->dm_irq_params.crc_window.update_win = false;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

/*
 * Get crc window coordinate y end
 */
static int crc_win_y_end_get(void *data, u64 *val)
{
	struct drm_crtc *crtc = data;
	struct drm_device *drm_dev = crtc->dev;
	struct amdgpu_crtc *acrtc = to_amdgpu_crtc(crtc);

	spin_lock_irq(&drm_dev->event_lock);
	*val = acrtc->dm_irq_params.crc_window.y_end;
	spin_unlock_irq(&drm_dev->event_lock);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(crc_win_y_end_fops, crc_win_y_end_get,
			 crc_win_y_end_set, "%llu\n");
/*
 * Trigger to commit crc window
 */
static int crc_win_update_set(void *data, u64 val)
{
	struct drm_crtc *new_crtc = data;
	struct drm_crtc *old_crtc = NULL;
	struct amdgpu_crtc *new_acrtc, *old_acrtc;
	struct amdgpu_device *adev = drm_to_adev(new_crtc->dev);
	struct crc_rd_work *crc_rd_wrk = adev->dm.crc_rd_wrk;

	if (val) {
		spin_lock_irq(&adev_to_drm(adev)->event_lock);
		spin_lock_irq(&crc_rd_wrk->crc_rd_work_lock);
		if (crc_rd_wrk && crc_rd_wrk->crtc) {
			old_crtc = crc_rd_wrk->crtc;
			old_acrtc = to_amdgpu_crtc(old_crtc);
		}
		new_acrtc = to_amdgpu_crtc(new_crtc);

		if (old_crtc && old_crtc != new_crtc) {
			old_acrtc->dm_irq_params.crc_window.activated = false;
			old_acrtc->dm_irq_params.crc_window.update_win = false;
			old_acrtc->dm_irq_params.crc_window.skip_frame_cnt = 0;

			new_acrtc->dm_irq_params.crc_window.activated = true;
			new_acrtc->dm_irq_params.crc_window.update_win = true;
			new_acrtc->dm_irq_params.crc_window.skip_frame_cnt = 0;
			crc_rd_wrk->crtc = new_crtc;
		} else {
			new_acrtc->dm_irq_params.crc_window.activated = true;
			new_acrtc->dm_irq_params.crc_window.update_win = true;
			new_acrtc->dm_irq_params.crc_window.skip_frame_cnt = 0;
			crc_rd_wrk->crtc = new_crtc;
		}
		spin_unlock_irq(&crc_rd_wrk->crc_rd_work_lock);
		spin_unlock_irq(&adev_to_drm(adev)->event_lock);
	}

	return 0;
}

/*
 * Get crc window update flag
 */
static int crc_win_update_get(void *data, u64 *val)
{
	*val = 0;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(crc_win_update_fops, crc_win_update_get,
			 crc_win_update_set, "%llu\n");

void crtc_debugfs_init(struct drm_crtc *crtc)
{
	struct dentry *dir = debugfs_lookup("crc", crtc->debugfs_entry);

	if (!dir)
		return;

	debugfs_create_file_unsafe("crc_win_x_start", 0644, dir, crtc,
				   &crc_win_x_start_fops);
	debugfs_create_file_unsafe("crc_win_y_start", 0644, dir, crtc,
				   &crc_win_y_start_fops);
	debugfs_create_file_unsafe("crc_win_x_end", 0644, dir, crtc,
				   &crc_win_x_end_fops);
	debugfs_create_file_unsafe("crc_win_y_end", 0644, dir, crtc,
				   &crc_win_y_end_fops);
	debugfs_create_file_unsafe("crc_win_update", 0644, dir, crtc,
				   &crc_win_update_fops);

}
#endif
/*
 * Writes DTN log state to the user supplied buffer.
 * Example usage: cat /sys/kernel/debug/dri/0/amdgpu_dm_dtn_log
 */
static ssize_t dtn_log_read(
	struct file *f,
	char __user *buf,
	size_t size,
	loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	struct dc *dc = adev->dm.dc;
	struct dc_log_buffer_ctx log_ctx = { 0 };
	ssize_t result = 0;

	if (!buf || !size)
		return -EINVAL;

	if (!dc->hwss.log_hw_state)
		return 0;

	dc->hwss.log_hw_state(dc, &log_ctx);

	if (*pos < log_ctx.pos) {
		size_t to_copy = log_ctx.pos - *pos;

		to_copy = min(to_copy, size);

		if (!copy_to_user(buf, log_ctx.buf + *pos, to_copy)) {
			*pos += to_copy;
			result = to_copy;
		}
	}

	kfree(log_ctx.buf);

	return result;
}

/*
 * Writes DTN log state to dmesg when triggered via a write.
 * Example usage: echo 1 > /sys/kernel/debug/dri/0/amdgpu_dm_dtn_log
 */
static ssize_t dtn_log_write(
	struct file *f,
	const char __user *buf,
	size_t size,
	loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	struct dc *dc = adev->dm.dc;

	/* Write triggers log output via dmesg. */
	if (size == 0)
		return 0;

	if (dc->hwss.log_hw_state)
		dc->hwss.log_hw_state(dc, NULL);

	return size;
}

static int mst_topo_show(struct seq_file *m, void *unused)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)m->private;
	struct drm_device *dev = adev_to_drm(adev);
	struct drm_connector *connector;
	struct drm_connector_list_iter conn_iter;
	struct amdgpu_dm_connector *aconnector;

	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (connector->connector_type != DRM_MODE_CONNECTOR_DisplayPort)
			continue;

		aconnector = to_amdgpu_dm_connector(connector);

		/* Ensure we're only dumping the topology of a root mst node */
		if (!aconnector->mst_mgr.mst_state)
			continue;

		seq_printf(m, "\nMST topology for connector %d\n", aconnector->connector_id);
		drm_dp_mst_dump_topology(m, &aconnector->mst_mgr);
	}
	drm_connector_list_iter_end(&conn_iter);

	return 0;
}

/*
 * Sets trigger hpd for MST topologies.
 * All connected connectors will be rediscovered and re started as needed if val of 1 is sent.
 * All topologies will be disconnected if val of 0 is set .
 * Usage to enable topologies: echo 1 > /sys/kernel/debug/dri/0/amdgpu_dm_trigger_hpd_mst
 * Usage to disable topologies: echo 0 > /sys/kernel/debug/dri/0/amdgpu_dm_trigger_hpd_mst
 */
static int trigger_hpd_mst_set(void *data, u64 val)
{
	struct amdgpu_device *adev = data;
	struct drm_device *dev = adev_to_drm(adev);
	struct drm_connector_list_iter iter;
	struct amdgpu_dm_connector *aconnector;
	struct drm_connector *connector;
	struct dc_link *link = NULL;

	if (val == 1) {
		drm_connector_list_iter_begin(dev, &iter);
		drm_for_each_connector_iter(connector, &iter) {
			aconnector = to_amdgpu_dm_connector(connector);
			if (aconnector->dc_link->type == dc_connection_mst_branch &&
			    aconnector->mst_mgr.aux) {
				dc_link_detect(aconnector->dc_link, DETECT_REASON_HPD);
				drm_dp_mst_topology_mgr_set_mst(&aconnector->mst_mgr, true);
			}
		}
	} else if (val == 0) {
		drm_connector_list_iter_begin(dev, &iter);
		drm_for_each_connector_iter(connector, &iter) {
			aconnector = to_amdgpu_dm_connector(connector);
			if (!aconnector->dc_link)
				continue;

			if (!aconnector->mst_port)
				continue;

			link = aconnector->dc_link;
			dp_receiver_power_ctrl(link, false);
			drm_dp_mst_topology_mgr_set_mst(&aconnector->mst_port->mst_mgr, false);
			link->mst_stream_alloc_table.stream_count = 0;
			memset(link->mst_stream_alloc_table.stream_allocations, 0,
					sizeof(link->mst_stream_alloc_table.stream_allocations));
		}
	} else {
		return 0;
	}
	drm_kms_helper_hotplug_event(dev);

	return 0;
}

/*
 * The interface doesn't need get function, so it will return the
 * value of zero
 * Usage: cat /sys/kernel/debug/dri/0/amdgpu_dm_trigger_hpd_mst
 */
static int trigger_hpd_mst_get(void *data, u64 *val)
{
	*val = 0;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(trigger_hpd_mst_ops, trigger_hpd_mst_get,
			 trigger_hpd_mst_set, "%llu\n");


/*
 * Sets the force_timing_sync debug option from the given string.
 * All connected displays will be force synchronized immediately.
 * Usage: echo 1 > /sys/kernel/debug/dri/0/amdgpu_dm_force_timing_sync
 */
static int force_timing_sync_set(void *data, u64 val)
{
	struct amdgpu_device *adev = data;

	adev->dm.force_timing_sync = (bool)val;

	amdgpu_dm_trigger_timing_sync(adev_to_drm(adev));

	return 0;
}

/*
 * Gets the force_timing_sync debug option value into the given buffer.
 * Usage: cat /sys/kernel/debug/dri/0/amdgpu_dm_force_timing_sync
 */
static int force_timing_sync_get(void *data, u64 *val)
{
	struct amdgpu_device *adev = data;

	*val = adev->dm.force_timing_sync;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(force_timing_sync_ops, force_timing_sync_get,
			 force_timing_sync_set, "%llu\n");


/*
 * Disables all HPD and HPD RX interrupt handling in the
 * driver when set to 1. Default is 0.
 */
static int disable_hpd_set(void *data, u64 val)
{
	struct amdgpu_device *adev = data;

	adev->dm.disable_hpd_irq = (bool)val;

	return 0;
}


/*
 * Returns 1 if HPD and HPRX interrupt handling is disabled,
 * 0 otherwise.
 */
static int disable_hpd_get(void *data, u64 *val)
{
	struct amdgpu_device *adev = data;

	*val = adev->dm.disable_hpd_irq;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(disable_hpd_ops, disable_hpd_get,
			 disable_hpd_set, "%llu\n");

/*
 * Sets the DC visual confirm debug option from the given string.
 * Example usage: echo 1 > /sys/kernel/debug/dri/0/amdgpu_visual_confirm
 */
static int visual_confirm_set(void *data, u64 val)
{
	struct amdgpu_device *adev = data;

	adev->dm.dc->debug.visual_confirm = (enum visual_confirm)val;

	return 0;
}

/*
 * Reads the DC visual confirm debug option value into the given buffer.
 * Example usage: cat /sys/kernel/debug/dri/0/amdgpu_dm_visual_confirm
 */
static int visual_confirm_get(void *data, u64 *val)
{
	struct amdgpu_device *adev = data;

	*val = adev->dm.dc->debug.visual_confirm;

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mst_topo);
DEFINE_DEBUGFS_ATTRIBUTE(visual_confirm_fops, visual_confirm_get,
			 visual_confirm_set, "%llu\n");

/*
 * Dumps the DCC_EN bit for each pipe.
 * Example usage: cat /sys/kernel/debug/dri/0/amdgpu_dm_dcc_en
 */
static ssize_t dcc_en_bits_read(
	struct file *f,
	char __user *buf,
	size_t size,
	loff_t *pos)
{
	struct amdgpu_device *adev = file_inode(f)->i_private;
	struct dc *dc = adev->dm.dc;
	char *rd_buf = NULL;
	const uint32_t rd_buf_size = 32;
	uint32_t result = 0;
	int offset = 0;
	int num_pipes = dc->res_pool->pipe_count;
	int *dcc_en_bits;
	int i, r;

	dcc_en_bits = kcalloc(num_pipes, sizeof(int), GFP_KERNEL);
	if (!dcc_en_bits)
		return -ENOMEM;

	if (!dc->hwss.get_dcc_en_bits) {
		kfree(dcc_en_bits);
		return 0;
	}

	dc->hwss.get_dcc_en_bits(dc, dcc_en_bits);

	rd_buf = kcalloc(rd_buf_size, sizeof(char), GFP_KERNEL);
	if (!rd_buf)
		return -ENOMEM;

	for (i = 0; i < num_pipes; i++)
		offset += snprintf(rd_buf + offset, rd_buf_size - offset,
				   "%d  ", dcc_en_bits[i]);
	rd_buf[strlen(rd_buf)] = '\n';

	kfree(dcc_en_bits);

	while (size) {
		if (*pos >= rd_buf_size)
			break;
		r = put_user(*(rd_buf + result), buf);
		if (r)
			return r; /* r = -EFAULT */
		buf += 1;
		size -= 1;
		*pos += 1;
		result += 1;
	}

	kfree(rd_buf);
	return result;
}

void dtn_debugfs_init(struct amdgpu_device *adev)
{
	static const struct file_operations dtn_log_fops = {
		.owner = THIS_MODULE,
		.read = dtn_log_read,
		.write = dtn_log_write,
		.llseek = default_llseek
	};
	static const struct file_operations dcc_en_bits_fops = {
		.owner = THIS_MODULE,
		.read = dcc_en_bits_read,
		.llseek = default_llseek
	};

	struct drm_minor *minor = adev_to_drm(adev)->primary;
	struct dentry *root = minor->debugfs_root;

	debugfs_create_file("amdgpu_mst_topology", 0444, root,
			    adev, &mst_topo_fops);
	debugfs_create_file("amdgpu_dm_dtn_log", 0644, root, adev,
			    &dtn_log_fops);

	debugfs_create_file_unsafe("amdgpu_dm_visual_confirm", 0644, root, adev,
				   &visual_confirm_fops);

	debugfs_create_file_unsafe("amdgpu_dm_dmub_tracebuffer", 0644, root,
				   adev, &dmub_tracebuffer_fops);

	debugfs_create_file_unsafe("amdgpu_dm_dmub_fw_state", 0644, root,
				   adev, &dmub_fw_state_fops);

	debugfs_create_file_unsafe("amdgpu_dm_force_timing_sync", 0644, root,
				   adev, &force_timing_sync_ops);

	debugfs_create_file_unsafe("amdgpu_dm_dmcub_trace_event_en", 0644, root,
				   adev, &dmcub_trace_event_state_fops);

	debugfs_create_file_unsafe("amdgpu_dm_trigger_hpd_mst", 0644, root,
				   adev, &trigger_hpd_mst_ops);

	debugfs_create_file_unsafe("amdgpu_dm_dcc_en", 0644, root, adev,
				   &dcc_en_bits_fops);

	debugfs_create_file_unsafe("amdgpu_dm_disable_hpd", 0644, root, adev,
				   &disable_hpd_ops);

}