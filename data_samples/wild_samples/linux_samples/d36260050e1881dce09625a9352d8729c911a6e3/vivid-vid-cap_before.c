	if (vivid_is_webcam(dev)) {
		const struct v4l2_frmsize_discrete *sz =
			v4l2_find_nearest_size(webcam_sizes, width, height,
					       mp->width, mp->height);

		w = sz->width;
		h = sz->height;
	} else if (vivid_is_sdtv_cap(dev)) {
		w = 720;
		h = (dev->std_cap & V4L2_STD_525_60) ? 480 : 576;
	} else {
		w = dev->src_rect.width;
		h = dev->src_rect.height;
	}
	if (V4L2_FIELD_HAS_T_OR_B(mp->field))
		factor = 2;
	if (vivid_is_webcam(dev) ||
	    (!dev->has_scaler_cap && !dev->has_crop_cap && !dev->has_compose_cap)) {
		mp->width = w;
		mp->height = h / factor;
	} else {
		struct v4l2_rect r = { 0, 0, mp->width, mp->height * factor };

		v4l2_rect_set_min_size(&r, &vivid_min_rect);
		v4l2_rect_set_max_size(&r, &vivid_max_rect);
		if (dev->has_scaler_cap && !dev->has_compose_cap) {
			struct v4l2_rect max_r = { 0, 0, MAX_ZOOM * w, MAX_ZOOM * h };

			v4l2_rect_set_max_size(&r, &max_r);
		} else if (!dev->has_scaler_cap && dev->has_crop_cap && !dev->has_compose_cap) {
			v4l2_rect_set_max_size(&r, &dev->src_rect);
		} else if (!dev->has_scaler_cap && !dev->has_crop_cap) {
			v4l2_rect_set_min_size(&r, &dev->src_rect);
		}
		mp->width = r.width;
		mp->height = r.height / factor;
	}

	/* This driver supports custom bytesperline values */

	mp->num_planes = fmt->buffers;
	for (p = 0; p < fmt->buffers; p++) {
		/* Calculate the minimum supported bytesperline value */
		bytesperline = (mp->width * fmt->bit_depth[p]) >> 3;
		/* Calculate the maximum supported bytesperline value */
		max_bpl = (MAX_ZOOM * MAX_WIDTH * fmt->bit_depth[p]) >> 3;

		if (pfmt[p].bytesperline > max_bpl)
			pfmt[p].bytesperline = max_bpl;
		if (pfmt[p].bytesperline < bytesperline)
			pfmt[p].bytesperline = bytesperline;

		pfmt[p].sizeimage = (pfmt[p].bytesperline * mp->height) /
				fmt->vdownsampling[p] + fmt->data_offset[p];

		memset(pfmt[p].reserved, 0, sizeof(pfmt[p].reserved));
	}
	for (p = fmt->buffers; p < fmt->planes; p++)
		pfmt[0].sizeimage += (pfmt[0].bytesperline * mp->height *
			(fmt->bit_depth[p] / fmt->vdownsampling[p])) /
			(fmt->bit_depth[0] / fmt->vdownsampling[0]);

	mp->colorspace = vivid_colorspace_cap(dev);
	if (fmt->color_enc == TGP_COLOR_ENC_HSV)
		mp->hsv_enc = vivid_hsv_enc_cap(dev);
	else
		mp->ycbcr_enc = vivid_ycbcr_enc_cap(dev);
	mp->xfer_func = vivid_xfer_func_cap(dev);
	mp->quantization = vivid_quantization_cap(dev);
	memset(mp->reserved, 0, sizeof(mp->reserved));
	return 0;
}

int vivid_s_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct v4l2_pix_format_mplane *mp = &f->fmt.pix_mp;
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_rect *crop = &dev->crop_cap;
	struct v4l2_rect *compose = &dev->compose_cap;
	struct vb2_queue *q = &dev->vb_vid_cap_q;
	int ret = vivid_try_fmt_vid_cap(file, priv, f);
	unsigned factor = 1;
	unsigned p;
	unsigned i;

	if (ret < 0)
		return ret;

	if (vb2_is_busy(q)) {
		dprintk(dev, 1, "%s device busy\n", __func__);
		return -EBUSY;
	}

	if (dev->overlay_cap_owner && dev->fb_cap.fmt.pixelformat != mp->pixelformat) {
		dprintk(dev, 1, "overlay is active, can't change pixelformat\n");
		return -EBUSY;
	}

	dev->fmt_cap = vivid_get_format(dev, mp->pixelformat);
	if (V4L2_FIELD_HAS_T_OR_B(mp->field))
		factor = 2;

	/* Note: the webcam input doesn't support scaling, cropping or composing */

	if (!vivid_is_webcam(dev) &&
	    (dev->has_scaler_cap || dev->has_crop_cap || dev->has_compose_cap)) {
		struct v4l2_rect r = { 0, 0, mp->width, mp->height };

		if (dev->has_scaler_cap) {
			if (dev->has_compose_cap)
				v4l2_rect_map_inside(compose, &r);
			else
				*compose = r;
			if (dev->has_crop_cap && !dev->has_compose_cap) {
				struct v4l2_rect min_r = {
					0, 0,
					r.width / MAX_ZOOM,
					factor * r.height / MAX_ZOOM
				};
				struct v4l2_rect max_r = {
					0, 0,
					r.width * MAX_ZOOM,
					factor * r.height * MAX_ZOOM
				};

				v4l2_rect_set_min_size(crop, &min_r);
				v4l2_rect_set_max_size(crop, &max_r);
				v4l2_rect_map_inside(crop, &dev->crop_bounds_cap);
			} else if (dev->has_crop_cap) {
				struct v4l2_rect min_r = {
					0, 0,
					compose->width / MAX_ZOOM,
					factor * compose->height / MAX_ZOOM
				};
				struct v4l2_rect max_r = {
					0, 0,
					compose->width * MAX_ZOOM,
					factor * compose->height * MAX_ZOOM
				};

				v4l2_rect_set_min_size(crop, &min_r);
				v4l2_rect_set_max_size(crop, &max_r);
				v4l2_rect_map_inside(crop, &dev->crop_bounds_cap);
			}
		} else if (dev->has_crop_cap && !dev->has_compose_cap) {
			r.height *= factor;
			v4l2_rect_set_size_to(crop, &r);
			v4l2_rect_map_inside(crop, &dev->crop_bounds_cap);
			r = *crop;
			r.height /= factor;
			v4l2_rect_set_size_to(compose, &r);
		} else if (!dev->has_crop_cap) {
			v4l2_rect_map_inside(compose, &r);
		} else {
			r.height *= factor;
			v4l2_rect_set_max_size(crop, &r);
			v4l2_rect_map_inside(crop, &dev->crop_bounds_cap);
			compose->top *= factor;
			compose->height *= factor;
			v4l2_rect_set_size_to(compose, crop);
			v4l2_rect_map_inside(compose, &r);
			compose->top /= factor;
			compose->height /= factor;
		}
	} else if (vivid_is_webcam(dev)) {
		/* Guaranteed to be a match */
		for (i = 0; i < ARRAY_SIZE(webcam_sizes); i++)
			if (webcam_sizes[i].width == mp->width &&
					webcam_sizes[i].height == mp->height)
				break;
		dev->webcam_size_idx = i;
		if (dev->webcam_ival_idx >= 2 * (VIVID_WEBCAM_SIZES - i))
			dev->webcam_ival_idx = 2 * (VIVID_WEBCAM_SIZES - i) - 1;
		vivid_update_format_cap(dev, false);
	} else {
		struct v4l2_rect r = { 0, 0, mp->width, mp->height };

		v4l2_rect_set_size_to(compose, &r);
		r.height *= factor;
		v4l2_rect_set_size_to(crop, &r);
	}

	dev->fmt_cap_rect.width = mp->width;
	dev->fmt_cap_rect.height = mp->height;
	tpg_s_buf_height(&dev->tpg, mp->height);
	tpg_s_fourcc(&dev->tpg, dev->fmt_cap->fourcc);
	for (p = 0; p < tpg_g_buffers(&dev->tpg); p++)
		tpg_s_bytesperline(&dev->tpg, p, mp->plane_fmt[p].bytesperline);
	dev->field_cap = mp->field;
	if (dev->field_cap == V4L2_FIELD_ALTERNATE)
		tpg_s_field(&dev->tpg, V4L2_FIELD_TOP, true);
	else
		tpg_s_field(&dev->tpg, dev->field_cap, false);
	tpg_s_crop_compose(&dev->tpg, &dev->crop_cap, &dev->compose_cap);
	if (vivid_is_sdtv_cap(dev))
		dev->tv_field_cap = mp->field;
	tpg_update_mv_step(&dev->tpg);
	return 0;
}

int vidioc_g_fmt_vid_cap_mplane(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->multiplanar)
		return -ENOTTY;
	return vivid_g_fmt_vid_cap(file, priv, f);
}

int vidioc_try_fmt_vid_cap_mplane(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->multiplanar)
		return -ENOTTY;
	return vivid_try_fmt_vid_cap(file, priv, f);
}

int vidioc_s_fmt_vid_cap_mplane(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->multiplanar)
		return -ENOTTY;
	return vivid_s_fmt_vid_cap(file, priv, f);
}

int vidioc_g_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;
	return fmt_sp2mp_func(file, priv, f, vivid_g_fmt_vid_cap);
}

int vidioc_try_fmt_vid_cap(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;
	return fmt_sp2mp_func(file, priv, f, vivid_try_fmt_vid_cap);
}

int vidioc_s_fmt_vid_cap(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;
	return fmt_sp2mp_func(file, priv, f, vivid_s_fmt_vid_cap);
}

int vivid_vid_cap_g_selection(struct file *file, void *priv,
			      struct v4l2_selection *sel)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->has_crop_cap && !dev->has_compose_cap)
		return -ENOTTY;
	if (sel->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (vivid_is_webcam(dev))
		return -ENODATA;

	sel->r.left = sel->r.top = 0;
	switch (sel->target) {
	case V4L2_SEL_TGT_CROP:
		if (!dev->has_crop_cap)
			return -EINVAL;
		sel->r = dev->crop_cap;
		break;
	case V4L2_SEL_TGT_CROP_DEFAULT:
	case V4L2_SEL_TGT_CROP_BOUNDS:
		if (!dev->has_crop_cap)
			return -EINVAL;
		sel->r = dev->src_rect;
		break;
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
		if (!dev->has_compose_cap)
			return -EINVAL;
		sel->r = vivid_max_rect;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		if (!dev->has_compose_cap)
			return -EINVAL;
		sel->r = dev->compose_cap;
		break;
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
		if (!dev->has_compose_cap)
			return -EINVAL;
		sel->r = dev->fmt_cap_rect;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int vivid_vid_cap_s_selection(struct file *file, void *fh, struct v4l2_selection *s)
{
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_rect *crop = &dev->crop_cap;
	struct v4l2_rect *compose = &dev->compose_cap;
	unsigned factor = V4L2_FIELD_HAS_T_OR_B(dev->field_cap) ? 2 : 1;
	int ret;

	if (!dev->has_crop_cap && !dev->has_compose_cap)
		return -ENOTTY;
	if (s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (vivid_is_webcam(dev))
		return -ENODATA;

	switch (s->target) {
	case V4L2_SEL_TGT_CROP:
		if (!dev->has_crop_cap)
			return -EINVAL;
		ret = vivid_vid_adjust_sel(s->flags, &s->r);
		if (ret)
			return ret;
		v4l2_rect_set_min_size(&s->r, &vivid_min_rect);
		v4l2_rect_set_max_size(&s->r, &dev->src_rect);
		v4l2_rect_map_inside(&s->r, &dev->crop_bounds_cap);
		s->r.top /= factor;
		s->r.height /= factor;
		if (dev->has_scaler_cap) {
			struct v4l2_rect fmt = dev->fmt_cap_rect;
			struct v4l2_rect max_rect = {
				0, 0,
				s->r.width * MAX_ZOOM,
				s->r.height * MAX_ZOOM
			};
			struct v4l2_rect min_rect = {
				0, 0,
				s->r.width / MAX_ZOOM,
				s->r.height / MAX_ZOOM
			};

			v4l2_rect_set_min_size(&fmt, &min_rect);
			if (!dev->has_compose_cap)
				v4l2_rect_set_max_size(&fmt, &max_rect);
			if (!v4l2_rect_same_size(&dev->fmt_cap_rect, &fmt) &&
			    vb2_is_busy(&dev->vb_vid_cap_q))
				return -EBUSY;
			if (dev->has_compose_cap) {
				v4l2_rect_set_min_size(compose, &min_rect);
				v4l2_rect_set_max_size(compose, &max_rect);
			}
			dev->fmt_cap_rect = fmt;
			tpg_s_buf_height(&dev->tpg, fmt.height);
		} else if (dev->has_compose_cap) {
			struct v4l2_rect fmt = dev->fmt_cap_rect;

			v4l2_rect_set_min_size(&fmt, &s->r);
			if (!v4l2_rect_same_size(&dev->fmt_cap_rect, &fmt) &&
			    vb2_is_busy(&dev->vb_vid_cap_q))
				return -EBUSY;
			dev->fmt_cap_rect = fmt;
			tpg_s_buf_height(&dev->tpg, fmt.height);
			v4l2_rect_set_size_to(compose, &s->r);
			v4l2_rect_map_inside(compose, &dev->fmt_cap_rect);
		} else {
			if (!v4l2_rect_same_size(&s->r, &dev->fmt_cap_rect) &&
			    vb2_is_busy(&dev->vb_vid_cap_q))
				return -EBUSY;
			v4l2_rect_set_size_to(&dev->fmt_cap_rect, &s->r);
			v4l2_rect_set_size_to(compose, &s->r);
			v4l2_rect_map_inside(compose, &dev->fmt_cap_rect);
			tpg_s_buf_height(&dev->tpg, dev->fmt_cap_rect.height);
		}
		s->r.top *= factor;
		s->r.height *= factor;
		*crop = s->r;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		if (!dev->has_compose_cap)
			return -EINVAL;
		ret = vivid_vid_adjust_sel(s->flags, &s->r);
		if (ret)
			return ret;
		v4l2_rect_set_min_size(&s->r, &vivid_min_rect);
		v4l2_rect_set_max_size(&s->r, &dev->fmt_cap_rect);
		if (dev->has_scaler_cap) {
			struct v4l2_rect max_rect = {
				0, 0,
				dev->src_rect.width * MAX_ZOOM,
				(dev->src_rect.height / factor) * MAX_ZOOM
			};

			v4l2_rect_set_max_size(&s->r, &max_rect);
			if (dev->has_crop_cap) {
				struct v4l2_rect min_rect = {
					0, 0,
					s->r.width / MAX_ZOOM,
					(s->r.height * factor) / MAX_ZOOM
				};
				struct v4l2_rect max_rect = {
					0, 0,
					s->r.width * MAX_ZOOM,
					(s->r.height * factor) * MAX_ZOOM
				};

				v4l2_rect_set_min_size(crop, &min_rect);
				v4l2_rect_set_max_size(crop, &max_rect);
				v4l2_rect_map_inside(crop, &dev->crop_bounds_cap);
			}
		} else if (dev->has_crop_cap) {
			s->r.top *= factor;
			s->r.height *= factor;
			v4l2_rect_set_max_size(&s->r, &dev->src_rect);
			v4l2_rect_set_size_to(crop, &s->r);
			v4l2_rect_map_inside(crop, &dev->crop_bounds_cap);
			s->r.top /= factor;
			s->r.height /= factor;
		} else {
			v4l2_rect_set_size_to(&s->r, &dev->src_rect);
			s->r.height /= factor;
		}
		v4l2_rect_map_inside(&s->r, &dev->fmt_cap_rect);
		if (dev->bitmap_cap && (compose->width != s->r.width ||
					compose->height != s->r.height)) {
			kfree(dev->bitmap_cap);
			dev->bitmap_cap = NULL;
		}
		*compose = s->r;
		break;
	default:
		return -EINVAL;
	}

	tpg_s_crop_compose(&dev->tpg, crop, compose);
	return 0;
}

int vivid_vid_cap_cropcap(struct file *file, void *priv,
			      struct v4l2_cropcap *cap)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (cap->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	switch (vivid_get_pixel_aspect(dev)) {
	case TPG_PIXEL_ASPECT_NTSC:
		cap->pixelaspect.numerator = 11;
		cap->pixelaspect.denominator = 10;
		break;
	case TPG_PIXEL_ASPECT_PAL:
		cap->pixelaspect.numerator = 54;
		cap->pixelaspect.denominator = 59;
		break;
	case TPG_PIXEL_ASPECT_SQUARE:
		cap->pixelaspect.numerator = 1;
		cap->pixelaspect.denominator = 1;
		break;
	}
	return 0;
}

int vidioc_enum_fmt_vid_overlay(struct file *file, void  *priv,
					struct v4l2_fmtdesc *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct vivid_fmt *fmt;

	if (dev->multiplanar)
		return -ENOTTY;

	if (f->index >= ARRAY_SIZE(formats_ovl))
		return -EINVAL;

	fmt = &formats_ovl[f->index];

	f->pixelformat = fmt->fourcc;
	return 0;
}

int vidioc_g_fmt_vid_overlay(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct v4l2_rect *compose = &dev->compose_cap;
	struct v4l2_window *win = &f->fmt.win;
	unsigned clipcount = win->clipcount;

	if (dev->multiplanar)
		return -ENOTTY;

	win->w.top = dev->overlay_cap_top;
	win->w.left = dev->overlay_cap_left;
	win->w.width = compose->width;
	win->w.height = compose->height;
	win->field = dev->overlay_cap_field;
	win->clipcount = dev->clipcount_cap;
	if (clipcount > dev->clipcount_cap)
		clipcount = dev->clipcount_cap;
	if (dev->bitmap_cap == NULL)
		win->bitmap = NULL;
	else if (win->bitmap) {
		if (copy_to_user(win->bitmap, dev->bitmap_cap,
		    ((compose->width + 7) / 8) * compose->height))
			return -EFAULT;
	}
	if (clipcount && win->clips) {
		if (copy_to_user(win->clips, dev->clips_cap,
				 clipcount * sizeof(dev->clips_cap[0])))
			return -EFAULT;
	}
	return 0;
}

int vidioc_try_fmt_vid_overlay(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct v4l2_rect *compose = &dev->compose_cap;
	struct v4l2_window *win = &f->fmt.win;
	int i, j;

	if (dev->multiplanar)
		return -ENOTTY;

	win->w.left = clamp_t(int, win->w.left,
			      -dev->fb_cap.fmt.width, dev->fb_cap.fmt.width);
	win->w.top = clamp_t(int, win->w.top,
			     -dev->fb_cap.fmt.height, dev->fb_cap.fmt.height);
	win->w.width = compose->width;
	win->w.height = compose->height;
	if (win->field != V4L2_FIELD_BOTTOM && win->field != V4L2_FIELD_TOP)
		win->field = V4L2_FIELD_ANY;
	win->chromakey = 0;
	win->global_alpha = 0;
	if (win->clipcount && !win->clips)
		win->clipcount = 0;
	if (win->clipcount > MAX_CLIPS)
		win->clipcount = MAX_CLIPS;
	if (win->clipcount) {
		if (copy_from_user(dev->try_clips_cap, win->clips,
				   win->clipcount * sizeof(dev->clips_cap[0])))
			return -EFAULT;
		for (i = 0; i < win->clipcount; i++) {
			struct v4l2_rect *r = &dev->try_clips_cap[i].c;

			r->top = clamp_t(s32, r->top, 0, dev->fb_cap.fmt.height - 1);
			r->height = clamp_t(s32, r->height, 1, dev->fb_cap.fmt.height - r->top);
			r->left = clamp_t(u32, r->left, 0, dev->fb_cap.fmt.width - 1);
			r->width = clamp_t(u32, r->width, 1, dev->fb_cap.fmt.width - r->left);
		}
		/*
		 * Yeah, so sue me, it's an O(n^2) algorithm. But n is a small
		 * number and it's typically a one-time deal.
		 */
		for (i = 0; i < win->clipcount - 1; i++) {
			struct v4l2_rect *r1 = &dev->try_clips_cap[i].c;

			for (j = i + 1; j < win->clipcount; j++) {
				struct v4l2_rect *r2 = &dev->try_clips_cap[j].c;

				if (v4l2_rect_overlap(r1, r2))
					return -EINVAL;
			}
		}
		if (copy_to_user(win->clips, dev->try_clips_cap,
				 win->clipcount * sizeof(dev->clips_cap[0])))
			return -EFAULT;
	}
	return 0;
}

int vidioc_s_fmt_vid_overlay(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct v4l2_rect *compose = &dev->compose_cap;
	struct v4l2_window *win = &f->fmt.win;
	int ret = vidioc_try_fmt_vid_overlay(file, priv, f);
	unsigned bitmap_size = ((compose->width + 7) / 8) * compose->height;
	unsigned clips_size = win->clipcount * sizeof(dev->clips_cap[0]);
	void *new_bitmap = NULL;

	if (ret)
		return ret;

	if (win->bitmap) {
		new_bitmap = vzalloc(bitmap_size);

		if (new_bitmap == NULL)
			return -ENOMEM;
		if (copy_from_user(new_bitmap, win->bitmap, bitmap_size)) {
			vfree(new_bitmap);
			return -EFAULT;
		}
	}

	dev->overlay_cap_top = win->w.top;
	dev->overlay_cap_left = win->w.left;
	dev->overlay_cap_field = win->field;
	vfree(dev->bitmap_cap);
	dev->bitmap_cap = new_bitmap;
	dev->clipcount_cap = win->clipcount;
	if (dev->clipcount_cap)
		memcpy(dev->clips_cap, dev->try_clips_cap, clips_size);
	return 0;
}

int vivid_vid_cap_overlay(struct file *file, void *fh, unsigned i)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;

	if (i && dev->fb_vbase_cap == NULL)
		return -EINVAL;

	if (i && dev->fb_cap.fmt.pixelformat != dev->fmt_cap->fourcc) {
		dprintk(dev, 1, "mismatch between overlay and video capture pixelformats\n");
		return -EINVAL;
	}

	if (dev->overlay_cap_owner && dev->overlay_cap_owner != fh)
		return -EBUSY;
	dev->overlay_cap_owner = i ? fh : NULL;
	return 0;
}

int vivid_vid_cap_g_fbuf(struct file *file, void *fh,
				struct v4l2_framebuffer *a)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;

	*a = dev->fb_cap;
	a->capability = V4L2_FBUF_CAP_BITMAP_CLIPPING |
			V4L2_FBUF_CAP_LIST_CLIPPING;
	a->flags = V4L2_FBUF_FLAG_PRIMARY;
	a->fmt.field = V4L2_FIELD_NONE;
	a->fmt.colorspace = V4L2_COLORSPACE_SRGB;
	a->fmt.priv = 0;
	return 0;
}

int vivid_vid_cap_s_fbuf(struct file *file, void *fh,
				const struct v4l2_framebuffer *a)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct vivid_fmt *fmt;

	if (dev->multiplanar)
		return -ENOTTY;

	if (!capable(CAP_SYS_ADMIN) && !capable(CAP_SYS_RAWIO))
		return -EPERM;

	if (dev->overlay_cap_owner)
		return -EBUSY;

	if (a->base == NULL) {
		dev->fb_cap.base = NULL;
		dev->fb_vbase_cap = NULL;
		return 0;
	}

	if (a->fmt.width < 48 || a->fmt.height < 32)
		return -EINVAL;
	fmt = vivid_get_format(dev, a->fmt.pixelformat);
	if (!fmt || !fmt->can_do_overlay)
		return -EINVAL;
	if (a->fmt.bytesperline < (a->fmt.width * fmt->bit_depth[0]) / 8)
		return -EINVAL;
	if (a->fmt.height * a->fmt.bytesperline < a->fmt.sizeimage)
		return -EINVAL;

	dev->fb_vbase_cap = phys_to_virt((unsigned long)a->base);
	dev->fb_cap = *a;
	dev->overlay_cap_left = clamp_t(int, dev->overlay_cap_left,
				    -dev->fb_cap.fmt.width, dev->fb_cap.fmt.width);
	dev->overlay_cap_top = clamp_t(int, dev->overlay_cap_top,
				   -dev->fb_cap.fmt.height, dev->fb_cap.fmt.height);
	return 0;
}

static const struct v4l2_audio vivid_audio_inputs[] = {
	{ 0, "TV", V4L2_AUDCAP_STEREO },
	{ 1, "Line-In", V4L2_AUDCAP_STEREO },
};

int vidioc_enum_input(struct file *file, void *priv,
				struct v4l2_input *inp)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (inp->index >= dev->num_inputs)
		return -EINVAL;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	switch (dev->input_type[inp->index]) {
	case WEBCAM:
		snprintf(inp->name, sizeof(inp->name), "Webcam %u",
				dev->input_name_counter[inp->index]);
		inp->capabilities = 0;
		break;
	case TV:
		snprintf(inp->name, sizeof(inp->name), "TV %u",
				dev->input_name_counter[inp->index]);
		inp->type = V4L2_INPUT_TYPE_TUNER;
		inp->std = V4L2_STD_ALL;
		if (dev->has_audio_inputs)
			inp->audioset = (1 << ARRAY_SIZE(vivid_audio_inputs)) - 1;
		inp->capabilities = V4L2_IN_CAP_STD;
		break;
	case SVID:
		snprintf(inp->name, sizeof(inp->name), "S-Video %u",
				dev->input_name_counter[inp->index]);
		inp->std = V4L2_STD_ALL;
		if (dev->has_audio_inputs)
			inp->audioset = (1 << ARRAY_SIZE(vivid_audio_inputs)) - 1;
		inp->capabilities = V4L2_IN_CAP_STD;
		break;
	case HDMI:
		snprintf(inp->name, sizeof(inp->name), "HDMI %u",
				dev->input_name_counter[inp->index]);
		inp->capabilities = V4L2_IN_CAP_DV_TIMINGS;
		if (dev->edid_blocks == 0 ||
		    dev->dv_timings_signal_mode == NO_SIGNAL)
			inp->status |= V4L2_IN_ST_NO_SIGNAL;
		else if (dev->dv_timings_signal_mode == NO_LOCK ||
			 dev->dv_timings_signal_mode == OUT_OF_RANGE)
			inp->status |= V4L2_IN_ST_NO_H_LOCK;
		break;
	}
	if (dev->sensor_hflip)
		inp->status |= V4L2_IN_ST_HFLIP;
	if (dev->sensor_vflip)
		inp->status |= V4L2_IN_ST_VFLIP;
	if (dev->input == inp->index && vivid_is_sdtv_cap(dev)) {
		if (dev->std_signal_mode == NO_SIGNAL) {
			inp->status |= V4L2_IN_ST_NO_SIGNAL;
		} else if (dev->std_signal_mode == NO_LOCK) {
			inp->status |= V4L2_IN_ST_NO_H_LOCK;
		} else if (vivid_is_tv_cap(dev)) {
			switch (tpg_g_quality(&dev->tpg)) {
			case TPG_QUAL_GRAY:
				inp->status |= V4L2_IN_ST_COLOR_KILL;
				break;
			case TPG_QUAL_NOISE:
				inp->status |= V4L2_IN_ST_NO_H_LOCK;
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

int vidioc_g_input(struct file *file, void *priv, unsigned *i)
{
	struct vivid_dev *dev = video_drvdata(file);

	*i = dev->input;
	return 0;
}

int vidioc_s_input(struct file *file, void *priv, unsigned i)
{
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_bt_timings *bt = &dev->dv_timings_cap.bt;
	unsigned brightness;

	if (i >= dev->num_inputs)
		return -EINVAL;

	if (i == dev->input)
		return 0;

	if (vb2_is_busy(&dev->vb_vid_cap_q) || vb2_is_busy(&dev->vb_vbi_cap_q))
		return -EBUSY;

	dev->input = i;
	dev->vid_cap_dev.tvnorms = 0;
	if (dev->input_type[i] == TV || dev->input_type[i] == SVID) {
		dev->tv_audio_input = (dev->input_type[i] == TV) ? 0 : 1;
		dev->vid_cap_dev.tvnorms = V4L2_STD_ALL;
	}
	dev->vbi_cap_dev.tvnorms = dev->vid_cap_dev.tvnorms;
	vivid_update_format_cap(dev, false);

	if (dev->colorspace) {
		switch (dev->input_type[i]) {
		case WEBCAM:
			v4l2_ctrl_s_ctrl(dev->colorspace, VIVID_CS_SRGB);
			break;
		case TV:
		case SVID:
			v4l2_ctrl_s_ctrl(dev->colorspace, VIVID_CS_170M);
			break;
		case HDMI:
			if (bt->flags & V4L2_DV_FL_IS_CE_VIDEO) {
				if (dev->src_rect.width == 720 && dev->src_rect.height <= 576)
					v4l2_ctrl_s_ctrl(dev->colorspace, VIVID_CS_170M);
				else
					v4l2_ctrl_s_ctrl(dev->colorspace, VIVID_CS_709);
			} else {
				v4l2_ctrl_s_ctrl(dev->colorspace, VIVID_CS_SRGB);
			}
			break;
		}
	}

	/*
	 * Modify the brightness range depending on the input.
	 * This makes it easy to use vivid to test if applications can
	 * handle control range modifications and is also how this is
	 * typically used in practice as different inputs may be hooked
	 * up to different receivers with different control ranges.
	 */
	brightness = 128 * i + dev->input_brightness[i];
	v4l2_ctrl_modify_range(dev->brightness,
			128 * i, 255 + 128 * i, 1, 128 + 128 * i);
	v4l2_ctrl_s_ctrl(dev->brightness, brightness);
	return 0;
}

int vidioc_enumaudio(struct file *file, void *fh, struct v4l2_audio *vin)
{
	if (vin->index >= ARRAY_SIZE(vivid_audio_inputs))
		return -EINVAL;
	*vin = vivid_audio_inputs[vin->index];
	return 0;
}

int vidioc_g_audio(struct file *file, void *fh, struct v4l2_audio *vin)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_sdtv_cap(dev))
		return -EINVAL;
	*vin = vivid_audio_inputs[dev->tv_audio_input];
	return 0;
}

int vidioc_s_audio(struct file *file, void *fh, const struct v4l2_audio *vin)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_sdtv_cap(dev))
		return -EINVAL;
	if (vin->index >= ARRAY_SIZE(vivid_audio_inputs))
		return -EINVAL;
	dev->tv_audio_input = vin->index;
	return 0;
}

int vivid_video_g_frequency(struct file *file, void *fh, struct v4l2_frequency *vf)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (vf->tuner != 0)
		return -EINVAL;
	vf->frequency = dev->tv_freq;
	return 0;
}

int vivid_video_s_frequency(struct file *file, void *fh, const struct v4l2_frequency *vf)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (vf->tuner != 0)
		return -EINVAL;
	dev->tv_freq = clamp_t(unsigned, vf->frequency, MIN_TV_FREQ, MAX_TV_FREQ);
	if (vivid_is_tv_cap(dev))
		vivid_update_quality(dev);
	return 0;
}

int vivid_video_s_tuner(struct file *file, void *fh, const struct v4l2_tuner *vt)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (vt->index != 0)
		return -EINVAL;
	if (vt->audmode > V4L2_TUNER_MODE_LANG1_LANG2)
		return -EINVAL;
	dev->tv_audmode = vt->audmode;
	return 0;
}

int vivid_video_g_tuner(struct file *file, void *fh, struct v4l2_tuner *vt)
{
	struct vivid_dev *dev = video_drvdata(file);
	enum tpg_quality qual;

	if (vt->index != 0)
		return -EINVAL;

	vt->capability = V4L2_TUNER_CAP_NORM | V4L2_TUNER_CAP_STEREO |
			 V4L2_TUNER_CAP_LANG1 | V4L2_TUNER_CAP_LANG2;
	vt->audmode = dev->tv_audmode;
	vt->rangelow = MIN_TV_FREQ;
	vt->rangehigh = MAX_TV_FREQ;
	qual = vivid_get_quality(dev, &vt->afc);
	if (qual == TPG_QUAL_COLOR)
		vt->signal = 0xffff;
	else if (qual == TPG_QUAL_GRAY)
		vt->signal = 0x8000;
	else
		vt->signal = 0;
	if (qual == TPG_QUAL_NOISE) {
		vt->rxsubchans = 0;
	} else if (qual == TPG_QUAL_GRAY) {
		vt->rxsubchans = V4L2_TUNER_SUB_MONO;
	} else {
		unsigned channel_nr = dev->tv_freq / (6 * 16);
		unsigned options = (dev->std_cap & V4L2_STD_NTSC_M) ? 4 : 3;

		switch (channel_nr % options) {
		case 0:
			vt->rxsubchans = V4L2_TUNER_SUB_MONO;
			break;
		case 1:
			vt->rxsubchans = V4L2_TUNER_SUB_STEREO;
			break;
		case 2:
			if (dev->std_cap & V4L2_STD_NTSC_M)
				vt->rxsubchans = V4L2_TUNER_SUB_MONO | V4L2_TUNER_SUB_SAP;
			else
				vt->rxsubchans = V4L2_TUNER_SUB_LANG1 | V4L2_TUNER_SUB_LANG2;
			break;
		case 3:
			vt->rxsubchans = V4L2_TUNER_SUB_STEREO | V4L2_TUNER_SUB_SAP;
			break;
		}
	}
	strlcpy(vt->name, "TV Tuner", sizeof(vt->name));
	return 0;
}

/* Must remain in sync with the vivid_ctrl_standard_strings array */
const v4l2_std_id vivid_standard[] = {
	V4L2_STD_NTSC_M,
	V4L2_STD_NTSC_M_JP,
	V4L2_STD_NTSC_M_KR,
	V4L2_STD_NTSC_443,
	V4L2_STD_PAL_BG | V4L2_STD_PAL_H,
	V4L2_STD_PAL_I,
	V4L2_STD_PAL_DK,
	V4L2_STD_PAL_M,
	V4L2_STD_PAL_N,
	V4L2_STD_PAL_Nc,
	V4L2_STD_PAL_60,
	V4L2_STD_SECAM_B | V4L2_STD_SECAM_G | V4L2_STD_SECAM_H,
	V4L2_STD_SECAM_DK,
	V4L2_STD_SECAM_L,
	V4L2_STD_SECAM_LC,
	V4L2_STD_UNKNOWN
};

/* Must remain in sync with the vivid_standard array */
const char * const vivid_ctrl_standard_strings[] = {
	"NTSC-M",
	"NTSC-M-JP",
	"NTSC-M-KR",
	"NTSC-443",
	"PAL-BGH",
	"PAL-I",
	"PAL-DK",
	"PAL-M",
	"PAL-N",
	"PAL-Nc",
	"PAL-60",
	"SECAM-BGH",
	"SECAM-DK",
	"SECAM-L",
	"SECAM-Lc",
	NULL,
};

int vidioc_querystd(struct file *file, void *priv, v4l2_std_id *id)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_sdtv_cap(dev))
		return -ENODATA;
	if (dev->std_signal_mode == NO_SIGNAL ||
	    dev->std_signal_mode == NO_LOCK) {
		*id = V4L2_STD_UNKNOWN;
		return 0;
	}
	if (vivid_is_tv_cap(dev) && tpg_g_quality(&dev->tpg) == TPG_QUAL_NOISE) {
		*id = V4L2_STD_UNKNOWN;
	} else if (dev->std_signal_mode == CURRENT_STD) {
		*id = dev->std_cap;
	} else if (dev->std_signal_mode == SELECTED_STD) {
		*id = dev->query_std;
	} else {
		*id = vivid_standard[dev->query_std_last];
		dev->query_std_last = (dev->query_std_last + 1) % ARRAY_SIZE(vivid_standard);
	}

	return 0;
}

int vivid_vid_cap_s_std(struct file *file, void *priv, v4l2_std_id id)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_sdtv_cap(dev))
		return -ENODATA;
	if (dev->std_cap == id)
		return 0;
	if (vb2_is_busy(&dev->vb_vid_cap_q) || vb2_is_busy(&dev->vb_vbi_cap_q))
		return -EBUSY;
	dev->std_cap = id;
	vivid_update_format_cap(dev, false);
	return 0;
}

static void find_aspect_ratio(u32 width, u32 height,
			       u32 *num, u32 *denom)
{
	if (!(height % 3) && ((height * 4 / 3) == width)) {
		*num = 4;
		*denom = 3;
	} else if (!(height % 9) && ((height * 16 / 9) == width)) {
		*num = 16;
		*denom = 9;
	} else if (!(height % 10) && ((height * 16 / 10) == width)) {
		*num = 16;
		*denom = 10;
	} else if (!(height % 4) && ((height * 5 / 4) == width)) {
		*num = 5;
		*denom = 4;
	} else if (!(height % 9) && ((height * 15 / 9) == width)) {
		*num = 15;
		*denom = 9;
	} else { /* default to 16:9 */
		*num = 16;
		*denom = 9;
	}
}

static bool valid_cvt_gtf_timings(struct v4l2_dv_timings *timings)
{
	struct v4l2_bt_timings *bt = &timings->bt;
	u32 total_h_pixel;
	u32 total_v_lines;
	u32 h_freq;

	if (!v4l2_valid_dv_timings(timings, &vivid_dv_timings_cap,
				NULL, NULL))
		return false;

	total_h_pixel = V4L2_DV_BT_FRAME_WIDTH(bt);
	total_v_lines = V4L2_DV_BT_FRAME_HEIGHT(bt);

	h_freq = (u32)bt->pixelclock / total_h_pixel;

	if (bt->standards == 0 || (bt->standards & V4L2_DV_BT_STD_CVT)) {
		if (v4l2_detect_cvt(total_v_lines, h_freq, bt->vsync, bt->width,
				    bt->polarities, bt->interlaced, timings))
			return true;
	}

	if (bt->standards == 0 || (bt->standards & V4L2_DV_BT_STD_GTF)) {
		struct v4l2_fract aspect_ratio;

		find_aspect_ratio(bt->width, bt->height,
				  &aspect_ratio.numerator,
				  &aspect_ratio.denominator);
		if (v4l2_detect_gtf(total_v_lines, h_freq, bt->vsync,
				    bt->polarities, bt->interlaced,
				    aspect_ratio, timings))
			return true;
	}
	return false;
}

int vivid_vid_cap_s_dv_timings(struct file *file, void *_fh,
				    struct v4l2_dv_timings *timings)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_hdmi_cap(dev))
		return -ENODATA;
	if (!v4l2_find_dv_timings_cap(timings, &vivid_dv_timings_cap,
				      0, NULL, NULL) &&
	    !valid_cvt_gtf_timings(timings))
		return -EINVAL;

	if (v4l2_match_dv_timings(timings, &dev->dv_timings_cap, 0, false))
		return 0;
	if (vb2_is_busy(&dev->vb_vid_cap_q))
		return -EBUSY;

	dev->dv_timings_cap = *timings;
	vivid_update_format_cap(dev, false);
	return 0;
}

int vidioc_query_dv_timings(struct file *file, void *_fh,
				    struct v4l2_dv_timings *timings)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_hdmi_cap(dev))
		return -ENODATA;
	if (dev->dv_timings_signal_mode == NO_SIGNAL ||
	    dev->edid_blocks == 0)
		return -ENOLINK;
	if (dev->dv_timings_signal_mode == NO_LOCK)
		return -ENOLCK;
	if (dev->dv_timings_signal_mode == OUT_OF_RANGE) {
		timings->bt.pixelclock = vivid_dv_timings_cap.bt.max_pixelclock * 2;
		return -ERANGE;
	}
	if (dev->dv_timings_signal_mode == CURRENT_DV_TIMINGS) {
		*timings = dev->dv_timings_cap;
	} else if (dev->dv_timings_signal_mode == SELECTED_DV_TIMINGS) {
		*timings = v4l2_dv_timings_presets[dev->query_dv_timings];
	} else {
		*timings = v4l2_dv_timings_presets[dev->query_dv_timings_last];
		dev->query_dv_timings_last = (dev->query_dv_timings_last + 1) %
						dev->query_dv_timings_size;
	}
	return 0;
}

int vidioc_s_edid(struct file *file, void *_fh,
			 struct v4l2_edid *edid)
{
	struct vivid_dev *dev = video_drvdata(file);
	u16 phys_addr;
	unsigned int i;
	int ret;

	memset(edid->reserved, 0, sizeof(edid->reserved));
	if (edid->pad >= dev->num_inputs)
		return -EINVAL;
	if (dev->input_type[edid->pad] != HDMI || edid->start_block)
		return -EINVAL;
	if (edid->blocks == 0) {
		dev->edid_blocks = 0;
		phys_addr = CEC_PHYS_ADDR_INVALID;
		goto set_phys_addr;
	}
	if (edid->blocks > dev->edid_max_blocks) {
		edid->blocks = dev->edid_max_blocks;
		return -E2BIG;
	}
	phys_addr = cec_get_edid_phys_addr(edid->edid, edid->blocks * 128, NULL);
	ret = cec_phys_addr_validate(phys_addr, &phys_addr, NULL);
	if (ret)
		return ret;

	if (vb2_is_busy(&dev->vb_vid_cap_q))
		return -EBUSY;

	dev->edid_blocks = edid->blocks;
	memcpy(dev->edid, edid->edid, edid->blocks * 128);

set_phys_addr:
	/* TODO: a proper hotplug detect cycle should be emulated here */
	cec_s_phys_addr(dev->cec_rx_adap, phys_addr, false);

	for (i = 0; i < MAX_OUTPUTS && dev->cec_tx_adap[i]; i++)
		cec_s_phys_addr(dev->cec_tx_adap[i],
				cec_phys_addr_for_input(phys_addr, i + 1),
				false);
	return 0;
}

int vidioc_enum_framesizes(struct file *file, void *fh,
					 struct v4l2_frmsizeenum *fsize)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_webcam(dev) && !dev->has_scaler_cap)
		return -EINVAL;
	if (vivid_get_format(dev, fsize->pixel_format) == NULL)
		return -EINVAL;
	if (vivid_is_webcam(dev)) {
		if (fsize->index >= ARRAY_SIZE(webcam_sizes))
			return -EINVAL;
		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		fsize->discrete = webcam_sizes[fsize->index];
		return 0;
	}
	if (fsize->index)
		return -EINVAL;
	fsize->type = V4L2_FRMSIZE_TYPE_STEPWISE;
	fsize->stepwise.min_width = MIN_WIDTH;
	fsize->stepwise.max_width = MAX_WIDTH * MAX_ZOOM;
	fsize->stepwise.step_width = 2;
	fsize->stepwise.min_height = MIN_HEIGHT;
	fsize->stepwise.max_height = MAX_HEIGHT * MAX_ZOOM;
	fsize->stepwise.step_height = 2;
	return 0;
}

/* timeperframe is arbitrary and continuous */
int vidioc_enum_frameintervals(struct file *file, void *priv,
					     struct v4l2_frmivalenum *fival)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct vivid_fmt *fmt;
	int i;

	fmt = vivid_get_format(dev, fival->pixel_format);
	if (!fmt)
		return -EINVAL;

	if (!vivid_is_webcam(dev)) {
		if (fival->index)
			return -EINVAL;
		if (fival->width < MIN_WIDTH || fival->width > MAX_WIDTH * MAX_ZOOM)
			return -EINVAL;
		if (fival->height < MIN_HEIGHT || fival->height > MAX_HEIGHT * MAX_ZOOM)
			return -EINVAL;
		fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
		fival->discrete = dev->timeperframe_vid_cap;
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(webcam_sizes); i++)
		if (fival->width == webcam_sizes[i].width &&
		    fival->height == webcam_sizes[i].height)
			break;
	if (i == ARRAY_SIZE(webcam_sizes))
		return -EINVAL;
	if (fival->index >= 2 * (VIVID_WEBCAM_SIZES - i))
		return -EINVAL;
	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	fival->discrete = webcam_intervals[fival->index];
	return 0;
}

int vivid_vid_cap_g_parm(struct file *file, void *priv,
			  struct v4l2_streamparm *parm)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (parm->type != (dev->multiplanar ?
			   V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE :
			   V4L2_BUF_TYPE_VIDEO_CAPTURE))
		return -EINVAL;

	parm->parm.capture.capability   = V4L2_CAP_TIMEPERFRAME;
	parm->parm.capture.timeperframe = dev->timeperframe_vid_cap;
	parm->parm.capture.readbuffers  = 1;
	return 0;
}

#define FRACT_CMP(a, OP, b)	\
	((u64)(a).numerator * (b).denominator  OP  (u64)(b).numerator * (a).denominator)

int vivid_vid_cap_s_parm(struct file *file, void *priv,
			  struct v4l2_streamparm *parm)
{
	struct vivid_dev *dev = video_drvdata(file);
	unsigned ival_sz = 2 * (VIVID_WEBCAM_SIZES - dev->webcam_size_idx);
	struct v4l2_fract tpf;
	unsigned i;

	if (parm->type != (dev->multiplanar ?
			   V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE :
			   V4L2_BUF_TYPE_VIDEO_CAPTURE))
		return -EINVAL;
	if (!vivid_is_webcam(dev))
		return vivid_vid_cap_g_parm(file, priv, parm);

	tpf = parm->parm.capture.timeperframe;

	if (tpf.denominator == 0)
		tpf = webcam_intervals[ival_sz - 1];
	for (i = 0; i < ival_sz; i++)
		if (FRACT_CMP(tpf, >=, webcam_intervals[i]))
			break;
	if (i == ival_sz)
		i = ival_sz - 1;
	dev->webcam_ival_idx = i;
	tpf = webcam_intervals[dev->webcam_ival_idx];
	tpf = FRACT_CMP(tpf, <, tpf_min) ? tpf_min : tpf;
	tpf = FRACT_CMP(tpf, >, tpf_max) ? tpf_max : tpf;

	/* resync the thread's timings */
	dev->cap_seq_resync = true;
	dev->timeperframe_vid_cap = tpf;
	parm->parm.capture.capability   = V4L2_CAP_TIMEPERFRAME;
	parm->parm.capture.timeperframe = tpf;
	parm->parm.capture.readbuffers  = 1;
	return 0;
}