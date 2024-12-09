
#ifdef CONFIG_DEBUG_FS

#define HID_DEBUG_BUFSIZE 512

void hid_dump_input(struct hid_device *, struct hid_usage *, __s32);
void hid_dump_report(struct hid_device *, int , u8 *, int);
void hid_dump_device(struct hid_device *, struct seq_file *);
void hid_debug_exit(void);
void hid_debug_event(struct hid_device *, char *);


struct hid_debug_list {
	char *hid_debug_buf;
	int head;
	int tail;
	struct fasync_struct *fasync;
	struct hid_device *hdev;
	struct list_head node;
	struct mutex read_mutex;
#endif

#endif
