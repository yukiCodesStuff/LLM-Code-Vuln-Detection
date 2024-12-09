{
	dev_set_drvdata(&hdev->dev, data);
}

struct hci_dev *hci_dev_get(int index);
struct hci_dev *hci_get_route(bdaddr_t *dst, bdaddr_t *src, u8 src_type);

struct hci_dev *hci_alloc_dev(void);
void hci_free_dev(struct hci_dev *hdev);
int hci_register_dev(struct hci_dev *hdev);
void hci_unregister_dev(struct hci_dev *hdev);
void hci_cleanup_dev(struct hci_dev *hdev);
int hci_suspend_dev(struct hci_dev *hdev);
int hci_resume_dev(struct hci_dev *hdev);
int hci_reset_dev(struct hci_dev *hdev);
int hci_recv_frame(struct hci_dev *hdev, struct sk_buff *skb);
int hci_recv_diag(struct hci_dev *hdev, struct sk_buff *skb);
__printf(2, 3) void hci_set_hw_info(struct hci_dev *hdev, const char *fmt, ...);
__printf(2, 3) void hci_set_fw_info(struct hci_dev *hdev, const char *fmt, ...);

static inline void hci_set_msft_opcode(struct hci_dev *hdev, __u16 opcode)
{
#if IS_ENABLED(CONFIG_BT_MSFTEXT)
	hdev->msft_opcode = opcode;
#endif
}