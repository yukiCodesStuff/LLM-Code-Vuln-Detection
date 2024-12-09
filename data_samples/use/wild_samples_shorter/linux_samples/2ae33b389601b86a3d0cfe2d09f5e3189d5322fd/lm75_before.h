    which contains this code, we don't worry about the wasted space.
*/

#include <linux/hwmon.h>

/* straight from the datasheet */
#define LM75_TEMP_MIN (-55000)
#define LM75_TEMP_MAX 125000