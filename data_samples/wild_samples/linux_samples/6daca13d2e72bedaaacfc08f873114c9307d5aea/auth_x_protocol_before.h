struct ceph_x_authorize_b {
	__u8 struct_v;
	__le64 nonce;
} __attribute__ ((packed));

struct ceph_x_authorize_reply {
	__u8 struct_v;
	__le64 nonce_plus_one;
} __attribute__ ((packed));


/*
 * encyption bundle
 */
#define CEPHX_ENC_MAGIC 0xff009cad8826aa55ull

struct ceph_x_encrypt_header {
	__u8 struct_v;
	__le64 magic;
} __attribute__ ((packed));

#endif