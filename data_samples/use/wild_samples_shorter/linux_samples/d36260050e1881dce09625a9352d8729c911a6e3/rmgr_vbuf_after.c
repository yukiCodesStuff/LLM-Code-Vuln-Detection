 * @brief VBUF resource handles
 */
#define NUM_HANDLES 1000
static struct ia_css_rmgr_vbuf_handle handle_table[NUM_HANDLES];

/*
 * @brief VBUF resource pool - refpool
 */
static struct ia_css_rmgr_vbuf_pool refpool = {
	false,			/* copy_on_write */
	false,			/* recycle */
	0,			/* size */
	0,			/* index */
/*
 * @brief VBUF resource pool - writepool
 */
static struct ia_css_rmgr_vbuf_pool writepool = {
	true,			/* copy_on_write */
	false,			/* recycle */
	0,			/* size */
	0,			/* index */
/*
 * @brief VBUF resource pool - hmmbufferpool
 */
static struct ia_css_rmgr_vbuf_pool hmmbufferpool = {
	true,			/* copy_on_write */
	true,			/* recycle */
	32,			/* size */
	0,			/* index */