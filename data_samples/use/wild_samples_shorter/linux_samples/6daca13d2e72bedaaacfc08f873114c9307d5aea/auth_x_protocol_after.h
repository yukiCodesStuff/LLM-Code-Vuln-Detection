struct ceph_x_authorize_b {
	__u8 struct_v;
	__le64 nonce;
	__u8 have_challenge;
	__le64 server_challenge_plus_one;
} __attribute__ ((packed));

struct ceph_x_authorize_challenge {
	__u8 struct_v;
	__le64 server_challenge;
} __attribute__ ((packed));

struct ceph_x_authorize_reply {
	__u8 struct_v;