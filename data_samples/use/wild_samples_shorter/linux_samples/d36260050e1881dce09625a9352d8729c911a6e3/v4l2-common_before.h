 *	set of resolutions contained in an array of a driver specific struct.
 *
 * @array: a driver specific array of image sizes
 * @width_field: the name of the width field in the driver specific struct
 * @height_field: the name of the height field in the driver specific struct
 * @width: desired width.
 * @height: desired height.
 *
 * Returns the best match or NULL if the length of the array is zero.
 */
#define v4l2_find_nearest_size(array, width_field, height_field, \
			       width, height)				\
	({								\
		BUILD_BUG_ON(sizeof((array)->width_field) != sizeof(u32) || \
			     sizeof((array)->height_field) != sizeof(u32)); \
		(typeof(&(*(array))))__v4l2_find_nearest_size(		\
			(array), ARRAY_SIZE(array), sizeof(*(array)),	\
			offsetof(typeof(*(array)), width_field),	\
			offsetof(typeof(*(array)), height_field),	\
			width, height);					\
	})