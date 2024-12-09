int mgmt_control(struct sock *sk, struct msghdr *msg, size_t len);
int mgmt_index_added(struct hci_dev *hdev);
int mgmt_index_removed(struct hci_dev *hdev);
int mgmt_powered(struct hci_dev *hdev, u8 powered);
int mgmt_discoverable(struct hci_dev *hdev, u8 discoverable);
int mgmt_connectable(struct hci_dev *hdev, u8 connectable);
int mgmt_write_scan_failed(struct hci_dev *hdev, u8 scan, u8 status);