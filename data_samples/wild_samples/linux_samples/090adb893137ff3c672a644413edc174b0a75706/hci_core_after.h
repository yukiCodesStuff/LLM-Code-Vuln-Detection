struct hci_request {
	struct hci_dev		*hdev;
	struct sk_buff_head	cmd_q;

	/* If something goes wrong when building the HCI request, the error
	 * value is stored in this field.
	 */
	int			err;
};

void hci_req_init(struct hci_request *req, struct hci_dev *hdev);
int hci_req_run(struct hci_request *req, hci_req_complete_t complete);
void hci_req_add(struct hci_request *req, u16 opcode, u32 plen,
		 const void *param);
void hci_req_add_ev(struct hci_request *req, u16 opcode, u32 plen,
		    const void *param, u8 event);
void hci_req_cmd_complete(struct hci_dev *hdev, u16 opcode, u8 status);

struct sk_buff *__hci_cmd_sync(struct hci_dev *hdev, u16 opcode, u32 plen,
			       const void *param, u32 timeout);
struct sk_buff *__hci_cmd_sync_ev(struct hci_dev *hdev, u16 opcode, u32 plen,
				  const void *param, u8 event, u32 timeout);

int hci_send_cmd(struct hci_dev *hdev, __u16 opcode, __u32 plen,
		 const void *param);
void hci_send_acl(struct hci_chan *chan, struct sk_buff *skb, __u16 flags);
void hci_send_sco(struct hci_conn *conn, struct sk_buff *skb);

void *hci_sent_cmd_data(struct hci_dev *hdev, __u16 opcode);

/* ----- HCI Sockets ----- */
void hci_send_to_sock(struct hci_dev *hdev, struct sk_buff *skb);
void hci_send_to_control(struct sk_buff *skb, struct sock *skip_sk);
void hci_send_to_monitor(struct hci_dev *hdev, struct sk_buff *skb);

void hci_sock_dev_event(struct hci_dev *hdev, int event);

/* Management interface */
#define DISCOV_TYPE_BREDR		(BIT(BDADDR_BREDR))
#define DISCOV_TYPE_LE			(BIT(BDADDR_LE_PUBLIC) | \
					 BIT(BDADDR_LE_RANDOM))
#define DISCOV_TYPE_INTERLEAVED		(BIT(BDADDR_BREDR) | \
					 BIT(BDADDR_LE_PUBLIC) | \
					 BIT(BDADDR_LE_RANDOM))

int mgmt_control(struct sock *sk, struct msghdr *msg, size_t len);
int mgmt_index_added(struct hci_dev *hdev);
int mgmt_index_removed(struct hci_dev *hdev);
int mgmt_set_powered_failed(struct hci_dev *hdev, int err);
int mgmt_powered(struct hci_dev *hdev, u8 powered);
int mgmt_discoverable(struct hci_dev *hdev, u8 discoverable);
int mgmt_connectable(struct hci_dev *hdev, u8 connectable);
int mgmt_write_scan_failed(struct hci_dev *hdev, u8 scan, u8 status);
int mgmt_new_link_key(struct hci_dev *hdev, struct link_key *key,
		      bool persistent);
int mgmt_device_connected(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 link_type,
			  u8 addr_type, u32 flags, u8 *name, u8 name_len,
			  u8 *dev_class);
int mgmt_device_disconnected(struct hci_dev *hdev, bdaddr_t *bdaddr,
			     u8 link_type, u8 addr_type, u8 reason);
int mgmt_disconnect_failed(struct hci_dev *hdev, bdaddr_t *bdaddr,
			   u8 link_type, u8 addr_type, u8 status);
int mgmt_connect_failed(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 link_type,
			u8 addr_type, u8 status);
int mgmt_pin_code_request(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 secure);
int mgmt_pin_code_reply_complete(struct hci_dev *hdev, bdaddr_t *bdaddr,
				 u8 status);
int mgmt_pin_code_neg_reply_complete(struct hci_dev *hdev, bdaddr_t *bdaddr,
				     u8 status);
int mgmt_user_confirm_request(struct hci_dev *hdev, bdaddr_t *bdaddr,
			      u8 link_type, u8 addr_type, __le32 value,
			      u8 confirm_hint);
int mgmt_user_confirm_reply_complete(struct hci_dev *hdev, bdaddr_t *bdaddr,
				     u8 link_type, u8 addr_type, u8 status);
int mgmt_user_confirm_neg_reply_complete(struct hci_dev *hdev, bdaddr_t *bdaddr,
					 u8 link_type, u8 addr_type, u8 status);
int mgmt_user_passkey_request(struct hci_dev *hdev, bdaddr_t *bdaddr,
			      u8 link_type, u8 addr_type);
int mgmt_user_passkey_reply_complete(struct hci_dev *hdev, bdaddr_t *bdaddr,
				     u8 link_type, u8 addr_type, u8 status);
int mgmt_user_passkey_neg_reply_complete(struct hci_dev *hdev, bdaddr_t *bdaddr,
					 u8 link_type, u8 addr_type, u8 status);
int mgmt_user_passkey_notify(struct hci_dev *hdev, bdaddr_t *bdaddr,
			     u8 link_type, u8 addr_type, u32 passkey,
			     u8 entered);
int mgmt_auth_failed(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 link_type,
		     u8 addr_type, u8 status);
int mgmt_auth_enable_complete(struct hci_dev *hdev, u8 status);
int mgmt_ssp_enable_complete(struct hci_dev *hdev, u8 enable, u8 status);
int mgmt_set_class_of_dev_complete(struct hci_dev *hdev, u8 *dev_class,
				   u8 status);
int mgmt_set_local_name_complete(struct hci_dev *hdev, u8 *name, u8 status);
int mgmt_read_local_oob_data_reply_complete(struct hci_dev *hdev, u8 *hash,
					    u8 *randomizer, u8 status);
int mgmt_le_enable_complete(struct hci_dev *hdev, u8 enable, u8 status);
int mgmt_device_found(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 link_type,
		      u8 addr_type, u8 *dev_class, s8 rssi, u8 cfm_name,
		      u8 ssp, u8 *eir, u16 eir_len);
int mgmt_remote_name(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 link_type,
		     u8 addr_type, s8 rssi, u8 *name, u8 name_len);
int mgmt_start_discovery_failed(struct hci_dev *hdev, u8 status);
int mgmt_stop_discovery_failed(struct hci_dev *hdev, u8 status);
int mgmt_discovering(struct hci_dev *hdev, u8 discovering);
int mgmt_interleaved_discovery(struct hci_dev *hdev);
int mgmt_device_blocked(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 type);
int mgmt_device_unblocked(struct hci_dev *hdev, bdaddr_t *bdaddr, u8 type);
bool mgmt_valid_hdev(struct hci_dev *hdev);
int mgmt_new_ltk(struct hci_dev *hdev, struct smp_ltk *key, u8 persistent);

/* HCI info for socket */
#define hci_pi(sk) ((struct hci_pinfo *) sk)

struct hci_pinfo {
	struct bt_sock    bt;
	struct hci_dev    *hdev;
	struct hci_filter filter;
	__u32             cmsg_mask;
	unsigned short   channel;
};

/* HCI security filter */
#define HCI_SFLT_MAX_OGF  5

struct hci_sec_filter {
	__u32 type_mask;
	__u32 event_mask[2];
	__u32 ocf_mask[HCI_SFLT_MAX_OGF + 1][4];
};

/* ----- HCI requests ----- */
#define HCI_REQ_DONE	  0
#define HCI_REQ_PEND	  1
#define HCI_REQ_CANCELED  2

#define hci_req_lock(d)		mutex_lock(&d->req_lock)
#define hci_req_unlock(d)	mutex_unlock(&d->req_lock)

void hci_update_ad(struct hci_request *req);

void hci_le_conn_update(struct hci_conn *conn, u16 min, u16 max,
					u16 latency, u16 to_multiplier);
void hci_le_start_enc(struct hci_conn *conn, __le16 ediv, __u8 rand[8],
							__u8 ltk[16]);
int hci_do_inquiry(struct hci_dev *hdev, u8 length);
int hci_cancel_inquiry(struct hci_dev *hdev);
int hci_le_scan(struct hci_dev *hdev, u8 type, u16 interval, u16 window,
		int timeout);
int hci_cancel_le_scan(struct hci_dev *hdev);

u8 bdaddr_to_le(u8 bdaddr_type);

#endif /* __HCI_CORE_H */