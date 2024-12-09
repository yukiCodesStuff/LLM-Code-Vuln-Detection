struct v4l2_priv_tun_config {
	int tuner;
	void *priv;
};
#define TUNER_SET_CONFIG           _IOW('d', 92, struct v4l2_priv_tun_config)

#define VIDIOC_INT_RESET		_IOW ('d', 102, u32)

/* ------------------------------------------------------------------------- */

/* Miscellaneous helper functions */

/**
 * v4l_bound_align_image - adjust video dimensions according to
 *	a given constraints.
 *
 * @width:	pointer to width that will be adjusted if needed.
 * @wmin:	minimum width.
 * @wmax:	maximum width.
 * @walign:	least significant bit on width.
 * @height:	pointer to height that will be adjusted if needed.
 * @hmin:	minimum height.
 * @hmax:	maximum height.
 * @halign:	least significant bit on width.
 * @salign:	least significant bit for the image size (e. g.
 *		:math:`width * height`).
 *
 * Clip an image to have @width between @wmin and @wmax, and @height between
 * @hmin and @hmax, inclusive.
 *
 * Additionally, the @width will be a multiple of :math:`2^{walign}`,
 * the @height will be a multiple of :math:`2^{halign}`, and the overall
 * size :math:`width * height` will be a multiple of :math:`2^{salign}`.
 *
 * .. note::
 *
 *    #. The clipping rectangle may be shrunk or enlarged to fit the alignment
 *       constraints.
 *    #. @wmax must not be smaller than @wmin.
 *    #. @hmax must not be smaller than @hmin.
 *    #. The alignments must not be so high there are no possible image
 *       sizes within the allowed bounds.
 *    #. @wmin and @hmin must be at least 1 (don't use 0).
 *    #. For @walign, @halign and @salign, if you don't care about a certain
 *       alignment, specify ``0``, as :math:`2^0 = 1` and one byte alignment
 *       is equivalent to no alignment.
 *    #. If you only want to adjust downward, specify a maximum that's the
 *       same as the initial value.
 */
void v4l_bound_align_image(unsigned int *width, unsigned int wmin,
			   unsigned int wmax, unsigned int walign,
			   unsigned int *height, unsigned int hmin,
			   unsigned int hmax, unsigned int halign,
			   unsigned int salign);

/**
 * v4l2_find_nearest_size - Find the nearest size among a discrete
 *	set of resolutions contained in an array of a driver specific struct.
 *
 * @array: a driver specific array of image sizes
 * @array_size: the length of the driver specific array of image sizes
 * @width_field: the name of the width field in the driver specific struct
 * @height_field: the name of the height field in the driver specific struct
 * @width: desired width.
 * @height: desired height.
 *
 * Finds the closest resolution to minimize the width and height differences
 * between what requested and the supported resolutions. The size of the width
 * and height fields in the driver specific must equal to that of u32, i.e. four
 * bytes.
 *
 * Returns the best match or NULL if the length of the array is zero.
 */
#define v4l2_find_nearest_size(array, array_size, width_field, height_field, \
			       width, height)				\
	({								\
		BUILD_BUG_ON(sizeof((array)->width_field) != sizeof(u32) || \
			     sizeof((array)->height_field) != sizeof(u32)); \
		(typeof(&(*(array))))__v4l2_find_nearest_size(		\
			(array), array_size, sizeof(*(array)),		\
			offsetof(typeof(*(array)), width_field),	\
			offsetof(typeof(*(array)), height_field),	\
			width, height);					\
	})
const void *
__v4l2_find_nearest_size(const void *array, size_t array_size,
			 size_t entry_size, size_t width_offset,
			 size_t height_offset, s32 width, s32 height);

/**
 * v4l2_get_timestamp - helper routine to get a timestamp to be used when
 *	filling streaming metadata. Internally, it uses ktime_get_ts(),
 *	which is the recommended way to get it.
 *
 * @tv: pointer to &struct timeval to be filled.
 */
void v4l2_get_timestamp(struct timeval *tv);

/**
 * v4l2_g_parm_cap - helper routine for vidioc_g_parm to fill this in by
 *      calling the g_frame_interval op of the given subdev. It only works
 *      for V4L2_BUF_TYPE_VIDEO_CAPTURE(_MPLANE), hence the _cap in the
 *      function name.
 *
 * @vdev: the struct video_device pointer. Used to determine the device caps.
 * @sd: the sub-device pointer.
 * @a: the VIDIOC_G_PARM argument.
 */
int v4l2_g_parm_cap(struct video_device *vdev,
		    struct v4l2_subdev *sd, struct v4l2_streamparm *a);

/**
 * v4l2_s_parm_cap - helper routine for vidioc_s_parm to fill this in by
 *      calling the s_frame_interval op of the given subdev. It only works
 *      for V4L2_BUF_TYPE_VIDEO_CAPTURE(_MPLANE), hence the _cap in the
 *      function name.
 *
 * @vdev: the struct video_device pointer. Used to determine the device caps.
 * @sd: the sub-device pointer.
 * @a: the VIDIOC_S_PARM argument.
 */
int v4l2_s_parm_cap(struct video_device *vdev,
		    struct v4l2_subdev *sd, struct v4l2_streamparm *a);

#endif /* V4L2_COMMON_H_ */
struct v4l2_priv_tun_config {
	int tuner;
	void *priv;
};
#define TUNER_SET_CONFIG           _IOW('d', 92, struct v4l2_priv_tun_config)

#define VIDIOC_INT_RESET		_IOW ('d', 102, u32)

/* ------------------------------------------------------------------------- */

/* Miscellaneous helper functions */

/**
 * v4l_bound_align_image - adjust video dimensions according to
 *	a given constraints.
 *
 * @width:	pointer to width that will be adjusted if needed.
 * @wmin:	minimum width.
 * @wmax:	maximum width.
 * @walign:	least significant bit on width.
 * @height:	pointer to height that will be adjusted if needed.
 * @hmin:	minimum height.
 * @hmax:	maximum height.
 * @halign:	least significant bit on width.
 * @salign:	least significant bit for the image size (e. g.
 *		:math:`width * height`).
 *
 * Clip an image to have @width between @wmin and @wmax, and @height between
 * @hmin and @hmax, inclusive.
 *
 * Additionally, the @width will be a multiple of :math:`2^{walign}`,
 * the @height will be a multiple of :math:`2^{halign}`, and the overall
 * size :math:`width * height` will be a multiple of :math:`2^{salign}`.
 *
 * .. note::
 *
 *    #. The clipping rectangle may be shrunk or enlarged to fit the alignment
 *       constraints.
 *    #. @wmax must not be smaller than @wmin.
 *    #. @hmax must not be smaller than @hmin.
 *    #. The alignments must not be so high there are no possible image
 *       sizes within the allowed bounds.
 *    #. @wmin and @hmin must be at least 1 (don't use 0).
 *    #. For @walign, @halign and @salign, if you don't care about a certain
 *       alignment, specify ``0``, as :math:`2^0 = 1` and one byte alignment
 *       is equivalent to no alignment.
 *    #. If you only want to adjust downward, specify a maximum that's the
 *       same as the initial value.
 */
void v4l_bound_align_image(unsigned int *width, unsigned int wmin,
			   unsigned int wmax, unsigned int walign,
			   unsigned int *height, unsigned int hmin,
			   unsigned int hmax, unsigned int halign,
			   unsigned int salign);

/**
 * v4l2_find_nearest_size - Find the nearest size among a discrete
 *	set of resolutions contained in an array of a driver specific struct.
 *
 * @array: a driver specific array of image sizes
 * @array_size: the length of the driver specific array of image sizes
 * @width_field: the name of the width field in the driver specific struct
 * @height_field: the name of the height field in the driver specific struct
 * @width: desired width.
 * @height: desired height.
 *
 * Finds the closest resolution to minimize the width and height differences
 * between what requested and the supported resolutions. The size of the width
 * and height fields in the driver specific must equal to that of u32, i.e. four
 * bytes.
 *
 * Returns the best match or NULL if the length of the array is zero.
 */
#define v4l2_find_nearest_size(array, array_size, width_field, height_field, \
			       width, height)				\
	({								\
		BUILD_BUG_ON(sizeof((array)->width_field) != sizeof(u32) || \
			     sizeof((array)->height_field) != sizeof(u32)); \
		(typeof(&(*(array))))__v4l2_find_nearest_size(		\
			(array), array_size, sizeof(*(array)),		\
			offsetof(typeof(*(array)), width_field),	\
			offsetof(typeof(*(array)), height_field),	\
			width, height);					\
	})
const void *
__v4l2_find_nearest_size(const void *array, size_t array_size,
			 size_t entry_size, size_t width_offset,
			 size_t height_offset, s32 width, s32 height);

/**
 * v4l2_get_timestamp - helper routine to get a timestamp to be used when
 *	filling streaming metadata. Internally, it uses ktime_get_ts(),
 *	which is the recommended way to get it.
 *
 * @tv: pointer to &struct timeval to be filled.
 */
void v4l2_get_timestamp(struct timeval *tv);

/**
 * v4l2_g_parm_cap - helper routine for vidioc_g_parm to fill this in by
 *      calling the g_frame_interval op of the given subdev. It only works
 *      for V4L2_BUF_TYPE_VIDEO_CAPTURE(_MPLANE), hence the _cap in the
 *      function name.
 *
 * @vdev: the struct video_device pointer. Used to determine the device caps.
 * @sd: the sub-device pointer.
 * @a: the VIDIOC_G_PARM argument.
 */
int v4l2_g_parm_cap(struct video_device *vdev,
		    struct v4l2_subdev *sd, struct v4l2_streamparm *a);

/**
 * v4l2_s_parm_cap - helper routine for vidioc_s_parm to fill this in by
 *      calling the s_frame_interval op of the given subdev. It only works
 *      for V4L2_BUF_TYPE_VIDEO_CAPTURE(_MPLANE), hence the _cap in the
 *      function name.
 *
 * @vdev: the struct video_device pointer. Used to determine the device caps.
 * @sd: the sub-device pointer.
 * @a: the VIDIOC_S_PARM argument.
 */
int v4l2_s_parm_cap(struct video_device *vdev,
		    struct v4l2_subdev *sd, struct v4l2_streamparm *a);

#endif /* V4L2_COMMON_H_ */