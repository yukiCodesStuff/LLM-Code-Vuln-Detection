
#ifdef CONFIG_DEBUG_FS

#include <linux/kfifo.h>

#define HID_DEBUG_BUFSIZE 512
#define HID_DEBUG_FIFOSIZE 512

void hid_dump_input(struct hid_device *, struct hid_usage *, __s32);
void hid_dump_report(struct hid_device *, int , u8 *, int);
void hid_dump_device(struct hid_device *, struct seq_file *);
void hid_debug_exit(void);
void hid_debug_event(struct hid_device *, char *);

struct hid_debug_list {
	DECLARE_KFIFO_PTR(hid_debug_fifo, char);
	struct fasync_struct *fasync;
	struct hid_device *hdev;
	struct list_head node;
	struct mutex read_mutex;
#endif

#endif