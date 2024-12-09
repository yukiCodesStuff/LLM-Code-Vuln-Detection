
#include <linux/ceph/decode.h>
#include <linux/ceph/auth.h>
#include <linux/ceph/libceph.h>
#include <linux/ceph/messenger.h>

#include "crypto.h"
			  __le64 *psig)
{
	void *enc_buf = au->enc_buf;
	struct {
		__le32 len;
		__le32 header_crc;
		__le32 front_crc;
		__le32 middle_crc;
		__le32 data_crc;
	} __packed *sigblock = enc_buf + ceph_x_encrypt_offset();
	int ret;

	sigblock->len = cpu_to_le32(4*sizeof(u32));
	sigblock->header_crc = msg->hdr.crc;
	sigblock->front_crc = msg->footer.front_crc;
	sigblock->middle_crc = msg->footer.middle_crc;
	sigblock->data_crc =  msg->footer.data_crc;
	ret = ceph_x_encrypt(&au->session_key, enc_buf, CEPHX_AU_ENC_BUF_LEN,
			     sizeof(*sigblock));
	if (ret < 0)
		return ret;

	*psig = *(__le64 *)(enc_buf + sizeof(u32));
	return 0;
}

static int ceph_x_sign_message(struct ceph_auth_handshake *auth,