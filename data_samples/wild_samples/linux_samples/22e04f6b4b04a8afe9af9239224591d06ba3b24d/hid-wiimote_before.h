enum wiimote_exttype {
	WIIMOTE_EXT_NONE,
	WIIMOTE_EXT_UNKNOWN,
	WIIMOTE_EXT_NUNCHUK,
	WIIMOTE_EXT_CLASSIC_CONTROLLER,
	WIIMOTE_EXT_BALANCE_BOARD,
	WIIMOTE_EXT_PRO_CONTROLLER,
	WIIMOTE_EXT_NUM,
};

enum wiimote_mptype {
	WIIMOTE_MP_NONE,
	WIIMOTE_MP_UNKNOWN,
	WIIMOTE_MP_SINGLE,
	WIIMOTE_MP_PASSTHROUGH_NUNCHUK,
	WIIMOTE_MP_PASSTHROUGH_CLASSIC,
};

struct wiimote_buf {
	__u8 data[HID_MAX_BUFFER_SIZE];
	size_t size;
};

struct wiimote_queue {
	spinlock_t lock;
	struct work_struct worker;
	__u8 head;
	__u8 tail;
	struct wiimote_buf outq[WIIMOTE_BUFSIZE];
};

struct wiimote_state {
	spinlock_t lock;
	__u32 flags;
	__u8 accel_split[2];
	__u8 drm;
	__u8 devtype;
	__u8 exttype;
	__u8 mp;

	/* synchronous cmd requests */
	struct mutex sync;
	struct completion ready;
	int cmd;
	__u32 opt;

	/* results of synchronous requests */
	__u8 cmd_battery;
	__u8 cmd_err;
	__u8 *cmd_read_buf;
	__u8 cmd_read_size;

	/* calibration data */
	__u16 calib_bboard[4][3];
};

struct wiimote_data {
	struct hid_device *hdev;
	struct input_dev *input;
	struct led_classdev *leds[4];
	struct input_dev *accel;
	struct input_dev *ir;
	struct power_supply battery;
	struct input_dev *mp;
	struct timer_list timer;
	struct wiimote_debug *debug;

	union {
		struct input_dev *input;
	} extension;

	struct wiimote_queue queue;
	struct wiimote_state state;
	struct work_struct init_worker;
};

/* wiimote modules */

enum wiimod_module {
	WIIMOD_KEYS,
	WIIMOD_RUMBLE,
	WIIMOD_BATTERY,
	WIIMOD_LED1,
	WIIMOD_LED2,
	WIIMOD_LED3,
	WIIMOD_LED4,
	WIIMOD_ACCEL,
	WIIMOD_IR,
	WIIMOD_BUILTIN_MP,
	WIIMOD_NO_MP,
	WIIMOD_NUM,
	WIIMOD_NULL = WIIMOD_NUM,
};

#define WIIMOD_FLAG_INPUT		0x0001
#define WIIMOD_FLAG_EXT8		0x0002
#define WIIMOD_FLAG_EXT16		0x0004

struct wiimod_ops {
	__u16 flags;
	unsigned long arg;
	int (*probe) (const struct wiimod_ops *ops,
		      struct wiimote_data *wdata);
	void (*remove) (const struct wiimod_ops *ops,
			struct wiimote_data *wdata);

	void (*in_keys) (struct wiimote_data *wdata, const __u8 *keys);
	void (*in_accel) (struct wiimote_data *wdata, const __u8 *accel);
	void (*in_ir) (struct wiimote_data *wdata, const __u8 *ir, bool packed,
		       unsigned int id);
	void (*in_mp) (struct wiimote_data *wdata, const __u8 *mp);
	void (*in_ext) (struct wiimote_data *wdata, const __u8 *ext);
};

extern const struct wiimod_ops *wiimod_table[WIIMOD_NUM];
extern const struct wiimod_ops *wiimod_ext_table[WIIMOTE_EXT_NUM];
extern const struct wiimod_ops wiimod_mp;

/* wiimote requests */

enum wiiproto_reqs {
	WIIPROTO_REQ_NULL = 0x0,
	WIIPROTO_REQ_RUMBLE = 0x10,
	WIIPROTO_REQ_LED = 0x11,
	WIIPROTO_REQ_DRM = 0x12,
	WIIPROTO_REQ_IR1 = 0x13,
	WIIPROTO_REQ_SREQ = 0x15,
	WIIPROTO_REQ_WMEM = 0x16,
	WIIPROTO_REQ_RMEM = 0x17,
	WIIPROTO_REQ_IR2 = 0x1a,
	WIIPROTO_REQ_STATUS = 0x20,
	WIIPROTO_REQ_DATA = 0x21,
	WIIPROTO_REQ_RETURN = 0x22,

	/* DRM_K: BB*2 */
	WIIPROTO_REQ_DRM_K = 0x30,

	/* DRM_KA: BB*2 AA*3 */
	WIIPROTO_REQ_DRM_KA = 0x31,

	/* DRM_KE: BB*2 EE*8 */
	WIIPROTO_REQ_DRM_KE = 0x32,

	/* DRM_KAI: BB*2 AA*3 II*12 */
	WIIPROTO_REQ_DRM_KAI = 0x33,

	/* DRM_KEE: BB*2 EE*19 */
	WIIPROTO_REQ_DRM_KEE = 0x34,

	/* DRM_KAE: BB*2 AA*3 EE*16 */
	WIIPROTO_REQ_DRM_KAE = 0x35,

	/* DRM_KIE: BB*2 II*10 EE*9 */
	WIIPROTO_REQ_DRM_KIE = 0x36,

	/* DRM_KAIE: BB*2 AA*3 II*10 EE*6 */
	WIIPROTO_REQ_DRM_KAIE = 0x37,

	/* DRM_E: EE*21 */
	WIIPROTO_REQ_DRM_E = 0x3d,

	/* DRM_SKAI1: BB*2 AA*1 II*18 */
	WIIPROTO_REQ_DRM_SKAI1 = 0x3e,

	/* DRM_SKAI2: BB*2 AA*1 II*18 */
	WIIPROTO_REQ_DRM_SKAI2 = 0x3f,

	WIIPROTO_REQ_MAX
};

#define dev_to_wii(pdev) hid_get_drvdata(container_of(pdev, struct hid_device, \
									dev))

void __wiimote_schedule(struct wiimote_data *wdata);

extern void wiiproto_req_drm(struct wiimote_data *wdata, __u8 drm);
extern void wiiproto_req_rumble(struct wiimote_data *wdata, __u8 rumble);
extern void wiiproto_req_leds(struct wiimote_data *wdata, int leds);
extern void wiiproto_req_status(struct wiimote_data *wdata);
extern void wiiproto_req_accel(struct wiimote_data *wdata, __u8 accel);
extern void wiiproto_req_ir1(struct wiimote_data *wdata, __u8 flags);
extern void wiiproto_req_ir2(struct wiimote_data *wdata, __u8 flags);
extern int wiimote_cmd_write(struct wiimote_data *wdata, __u32 offset,
						const __u8 *wmem, __u8 size);
extern ssize_t wiimote_cmd_read(struct wiimote_data *wdata, __u32 offset,
							__u8 *rmem, __u8 size);

#define wiiproto_req_rreg(wdata, os, sz) \
				wiiproto_req_rmem((wdata), false, (os), (sz))
#define wiiproto_req_reeprom(wdata, os, sz) \
				wiiproto_req_rmem((wdata), true, (os), (sz))
extern void wiiproto_req_rmem(struct wiimote_data *wdata, bool eeprom,
						__u32 offset, __u16 size);

#ifdef CONFIG_DEBUG_FS

extern int wiidebug_init(struct wiimote_data *wdata);
extern void wiidebug_deinit(struct wiimote_data *wdata);

#else

static inline int wiidebug_init(void *u) { return 0; }
static inline void wiidebug_deinit(void *u) { }

#endif

/* requires the state.lock spinlock to be held */
static inline bool wiimote_cmd_pending(struct wiimote_data *wdata, int cmd,
								__u32 opt)
{
	return wdata->state.cmd == cmd && wdata->state.opt == opt;
}

/* requires the state.lock spinlock to be held */
static inline void wiimote_cmd_complete(struct wiimote_data *wdata)
{
	wdata->state.cmd = WIIPROTO_REQ_NULL;
	complete(&wdata->state.ready);
}

/* requires the state.lock spinlock to be held */
static inline void wiimote_cmd_abort(struct wiimote_data *wdata)
{
	/* Abort synchronous request by waking up the sleeping caller. But
	 * reset the state.cmd field to an invalid value so no further event
	 * handlers will work with it. */
	wdata->state.cmd = WIIPROTO_REQ_MAX;
	complete(&wdata->state.ready);
}

static inline int wiimote_cmd_acquire(struct wiimote_data *wdata)
{
	return mutex_lock_interruptible(&wdata->state.sync) ? -ERESTARTSYS : 0;
}

static inline void wiimote_cmd_acquire_noint(struct wiimote_data *wdata)
{
	mutex_lock(&wdata->state.sync);
}

/* requires the state.lock spinlock to be held */
static inline void wiimote_cmd_set(struct wiimote_data *wdata, int cmd,
								__u32 opt)
{
	INIT_COMPLETION(wdata->state.ready);
	wdata->state.cmd = cmd;
	wdata->state.opt = opt;
}

static inline void wiimote_cmd_release(struct wiimote_data *wdata)
{
	mutex_unlock(&wdata->state.sync);
}

static inline int wiimote_cmd_wait(struct wiimote_data *wdata)
{
	int ret;

	/* The completion acts as implicit memory barrier so we can safely
	 * assume that state.cmd is set on success/failure and isn't accessed
	 * by any other thread, anymore. */

	ret = wait_for_completion_interruptible_timeout(&wdata->state.ready, HZ);
	if (ret < 0)
		return -ERESTARTSYS;
	else if (ret == 0)
		return -EIO;
	else if (wdata->state.cmd != WIIPROTO_REQ_NULL)
		return -EIO;
	else
		return 0;
}

static inline int wiimote_cmd_wait_noint(struct wiimote_data *wdata)
{
	unsigned long ret;

	/* no locking needed; see wiimote_cmd_wait() */
	ret = wait_for_completion_timeout(&wdata->state.ready, HZ);
	if (!ret)
		return -EIO;
	else if (wdata->state.cmd != WIIPROTO_REQ_NULL)
		return -EIO;
	else
		return 0;
}

#endif
struct wiimote_state {
	spinlock_t lock;
	__u32 flags;
	__u8 accel_split[2];
	__u8 drm;
	__u8 devtype;
	__u8 exttype;
	__u8 mp;

	/* synchronous cmd requests */
	struct mutex sync;
	struct completion ready;
	int cmd;
	__u32 opt;

	/* results of synchronous requests */
	__u8 cmd_battery;
	__u8 cmd_err;
	__u8 *cmd_read_buf;
	__u8 cmd_read_size;

	/* calibration data */
	__u16 calib_bboard[4][3];
};

struct wiimote_data {
	struct hid_device *hdev;
	struct input_dev *input;
	struct led_classdev *leds[4];
	struct input_dev *accel;
	struct input_dev *ir;
	struct power_supply battery;
	struct input_dev *mp;
	struct timer_list timer;
	struct wiimote_debug *debug;

	union {
		struct input_dev *input;
	} extension;

	struct wiimote_queue queue;
	struct wiimote_state state;
	struct work_struct init_worker;
};

/* wiimote modules */

enum wiimod_module {
	WIIMOD_KEYS,
	WIIMOD_RUMBLE,
	WIIMOD_BATTERY,
	WIIMOD_LED1,
	WIIMOD_LED2,
	WIIMOD_LED3,
	WIIMOD_LED4,
	WIIMOD_ACCEL,
	WIIMOD_IR,
	WIIMOD_BUILTIN_MP,
	WIIMOD_NO_MP,
	WIIMOD_NUM,
	WIIMOD_NULL = WIIMOD_NUM,
};

#define WIIMOD_FLAG_INPUT		0x0001
#define WIIMOD_FLAG_EXT8		0x0002
#define WIIMOD_FLAG_EXT16		0x0004

struct wiimod_ops {
	__u16 flags;
	unsigned long arg;
	int (*probe) (const struct wiimod_ops *ops,
		      struct wiimote_data *wdata);
	void (*remove) (const struct wiimod_ops *ops,
			struct wiimote_data *wdata);

	void (*in_keys) (struct wiimote_data *wdata, const __u8 *keys);
	void (*in_accel) (struct wiimote_data *wdata, const __u8 *accel);
	void (*in_ir) (struct wiimote_data *wdata, const __u8 *ir, bool packed,
		       unsigned int id);
	void (*in_mp) (struct wiimote_data *wdata, const __u8 *mp);
	void (*in_ext) (struct wiimote_data *wdata, const __u8 *ext);
};

extern const struct wiimod_ops *wiimod_table[WIIMOD_NUM];
extern const struct wiimod_ops *wiimod_ext_table[WIIMOTE_EXT_NUM];
extern const struct wiimod_ops wiimod_mp;

/* wiimote requests */

enum wiiproto_reqs {
	WIIPROTO_REQ_NULL = 0x0,
	WIIPROTO_REQ_RUMBLE = 0x10,
	WIIPROTO_REQ_LED = 0x11,
	WIIPROTO_REQ_DRM = 0x12,
	WIIPROTO_REQ_IR1 = 0x13,
	WIIPROTO_REQ_SREQ = 0x15,
	WIIPROTO_REQ_WMEM = 0x16,
	WIIPROTO_REQ_RMEM = 0x17,
	WIIPROTO_REQ_IR2 = 0x1a,
	WIIPROTO_REQ_STATUS = 0x20,
	WIIPROTO_REQ_DATA = 0x21,
	WIIPROTO_REQ_RETURN = 0x22,

	/* DRM_K: BB*2 */
	WIIPROTO_REQ_DRM_K = 0x30,

	/* DRM_KA: BB*2 AA*3 */
	WIIPROTO_REQ_DRM_KA = 0x31,

	/* DRM_KE: BB*2 EE*8 */
	WIIPROTO_REQ_DRM_KE = 0x32,

	/* DRM_KAI: BB*2 AA*3 II*12 */
	WIIPROTO_REQ_DRM_KAI = 0x33,

	/* DRM_KEE: BB*2 EE*19 */
	WIIPROTO_REQ_DRM_KEE = 0x34,

	/* DRM_KAE: BB*2 AA*3 EE*16 */
	WIIPROTO_REQ_DRM_KAE = 0x35,

	/* DRM_KIE: BB*2 II*10 EE*9 */
	WIIPROTO_REQ_DRM_KIE = 0x36,

	/* DRM_KAIE: BB*2 AA*3 II*10 EE*6 */
	WIIPROTO_REQ_DRM_KAIE = 0x37,

	/* DRM_E: EE*21 */
	WIIPROTO_REQ_DRM_E = 0x3d,

	/* DRM_SKAI1: BB*2 AA*1 II*18 */
	WIIPROTO_REQ_DRM_SKAI1 = 0x3e,

	/* DRM_SKAI2: BB*2 AA*1 II*18 */
	WIIPROTO_REQ_DRM_SKAI2 = 0x3f,

	WIIPROTO_REQ_MAX
};

#define dev_to_wii(pdev) hid_get_drvdata(container_of(pdev, struct hid_device, \
									dev))

void __wiimote_schedule(struct wiimote_data *wdata);

extern void wiiproto_req_drm(struct wiimote_data *wdata, __u8 drm);
extern void wiiproto_req_rumble(struct wiimote_data *wdata, __u8 rumble);
extern void wiiproto_req_leds(struct wiimote_data *wdata, int leds);
extern void wiiproto_req_status(struct wiimote_data *wdata);
extern void wiiproto_req_accel(struct wiimote_data *wdata, __u8 accel);
extern void wiiproto_req_ir1(struct wiimote_data *wdata, __u8 flags);
extern void wiiproto_req_ir2(struct wiimote_data *wdata, __u8 flags);
extern int wiimote_cmd_write(struct wiimote_data *wdata, __u32 offset,
						const __u8 *wmem, __u8 size);
extern ssize_t wiimote_cmd_read(struct wiimote_data *wdata, __u32 offset,
							__u8 *rmem, __u8 size);

#define wiiproto_req_rreg(wdata, os, sz) \
				wiiproto_req_rmem((wdata), false, (os), (sz))
#define wiiproto_req_reeprom(wdata, os, sz) \
				wiiproto_req_rmem((wdata), true, (os), (sz))
extern void wiiproto_req_rmem(struct wiimote_data *wdata, bool eeprom,
						__u32 offset, __u16 size);

#ifdef CONFIG_DEBUG_FS

extern int wiidebug_init(struct wiimote_data *wdata);
extern void wiidebug_deinit(struct wiimote_data *wdata);

#else

static inline int wiidebug_init(void *u) { return 0; }
static inline void wiidebug_deinit(void *u) { }

#endif

/* requires the state.lock spinlock to be held */
static inline bool wiimote_cmd_pending(struct wiimote_data *wdata, int cmd,
								__u32 opt)
{
	return wdata->state.cmd == cmd && wdata->state.opt == opt;
}

/* requires the state.lock spinlock to be held */
static inline void wiimote_cmd_complete(struct wiimote_data *wdata)
{
	wdata->state.cmd = WIIPROTO_REQ_NULL;
	complete(&wdata->state.ready);
}

/* requires the state.lock spinlock to be held */
static inline void wiimote_cmd_abort(struct wiimote_data *wdata)
{
	/* Abort synchronous request by waking up the sleeping caller. But
	 * reset the state.cmd field to an invalid value so no further event
	 * handlers will work with it. */
	wdata->state.cmd = WIIPROTO_REQ_MAX;
	complete(&wdata->state.ready);
}

static inline int wiimote_cmd_acquire(struct wiimote_data *wdata)
{
	return mutex_lock_interruptible(&wdata->state.sync) ? -ERESTARTSYS : 0;
}

static inline void wiimote_cmd_acquire_noint(struct wiimote_data *wdata)
{
	mutex_lock(&wdata->state.sync);
}

/* requires the state.lock spinlock to be held */
static inline void wiimote_cmd_set(struct wiimote_data *wdata, int cmd,
								__u32 opt)
{
	INIT_COMPLETION(wdata->state.ready);
	wdata->state.cmd = cmd;
	wdata->state.opt = opt;
}

static inline void wiimote_cmd_release(struct wiimote_data *wdata)
{
	mutex_unlock(&wdata->state.sync);
}

static inline int wiimote_cmd_wait(struct wiimote_data *wdata)
{
	int ret;

	/* The completion acts as implicit memory barrier so we can safely
	 * assume that state.cmd is set on success/failure and isn't accessed
	 * by any other thread, anymore. */

	ret = wait_for_completion_interruptible_timeout(&wdata->state.ready, HZ);
	if (ret < 0)
		return -ERESTARTSYS;
	else if (ret == 0)
		return -EIO;
	else if (wdata->state.cmd != WIIPROTO_REQ_NULL)
		return -EIO;
	else
		return 0;
}

static inline int wiimote_cmd_wait_noint(struct wiimote_data *wdata)
{
	unsigned long ret;

	/* no locking needed; see wiimote_cmd_wait() */
	ret = wait_for_completion_timeout(&wdata->state.ready, HZ);
	if (!ret)
		return -EIO;
	else if (wdata->state.cmd != WIIPROTO_REQ_NULL)
		return -EIO;
	else
		return 0;
}

#endif