		return NULL;

	for (i = 0; i < size; i++) {
		bool valid;

		if (fmt[i].type != type)
			continue;
		valid = type != V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
			venus_helper_check_codec(inst, fmt[i].pixfmt);
		if (k == index && valid)
			break;
		if (valid)
			k++;
	}

	if (i == size)
		return NULL;

	return &fmt[i];
}

static int venc_v4l2_to_hfi(int id, int value)