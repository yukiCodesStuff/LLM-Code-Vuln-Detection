	mp->field = vivid_field_cap(dev, mp->field);
	if (vivid_is_webcam(dev)) {
		const struct v4l2_frmsize_discrete *sz =
			v4l2_find_nearest_size(webcam_sizes, width, height,
					       mp->width, mp->height);

		w = sz->width;
		h = sz->height;
	} else if (vivid_is_sdtv_cap(dev)) {