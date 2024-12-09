struct ceph_x_authorize_b {
	__u8 struct_v;
	__le64 nonce;
} __attribute__ ((packed));

struct ceph_x_authorize_reply {
	__u8 struct_v;