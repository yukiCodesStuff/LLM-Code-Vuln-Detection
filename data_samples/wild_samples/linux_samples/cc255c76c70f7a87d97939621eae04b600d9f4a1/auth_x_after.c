// SPDX-License-Identifier: GPL-2.0

#include <linux/ceph/ceph_debug.h>

#include <linux/err.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>

#include <linux/ceph/decode.h>
#include <linux/ceph/auth.h>
#include <linux/ceph/ceph_features.h>
#include <linux/ceph/libceph.h>
#include <linux/ceph/messenger.h>

#include "crypto.h"
#include "auth_x.h"
#include "auth_x_protocol.h"

static void ceph_x_validate_tickets(struct ceph_auth_client *ac, int *pneed);

static int ceph_x_is_authenticated(struct ceph_auth_client *ac)
{
	struct ceph_x_info *xi = ac->private;
	int need;

	ceph_x_validate_tickets(ac, &need);
	dout("ceph_x_is_authenticated want=%d need=%d have=%d\n",
	     ac->want_keys, need, xi->have_keys);
	return (ac->want_keys & xi->have_keys) == ac->want_keys;
}
{
	void *enc_buf = au->enc_buf;
	int ret;

	if (!CEPH_HAVE_FEATURE(msg->con->peer_features, CEPHX_V2)) {
		struct {
			__le32 len;
			__le32 header_crc;
			__le32 front_crc;
			__le32 middle_crc;
			__le32 data_crc;
		} __packed *sigblock = enc_buf + ceph_x_encrypt_offset();

		sigblock->len = cpu_to_le32(4*sizeof(u32));
		sigblock->header_crc = msg->hdr.crc;
		sigblock->front_crc = msg->footer.front_crc;
		sigblock->middle_crc = msg->footer.middle_crc;
		sigblock->data_crc =  msg->footer.data_crc;

		ret = ceph_x_encrypt(&au->session_key, enc_buf,
				     CEPHX_AU_ENC_BUF_LEN, sizeof(*sigblock));
		if (ret < 0)
			return ret;

		*psig = *(__le64 *)(enc_buf + sizeof(u32));
	} else {
		struct {
			__le32 header_crc;
			__le32 front_crc;
			__le32 front_len;
			__le32 middle_crc;
			__le32 middle_len;
			__le32 data_crc;
			__le32 data_len;
			__le32 seq_lower_word;
		} __packed *sigblock = enc_buf;
		struct {
			__le64 a, b, c, d;
		} __packed *penc = enc_buf;
		int ciphertext_len;

		sigblock->header_crc = msg->hdr.crc;
		sigblock->front_crc = msg->footer.front_crc;
		sigblock->front_len = msg->hdr.front_len;
		sigblock->middle_crc = msg->footer.middle_crc;
		sigblock->middle_len = msg->hdr.middle_len;
		sigblock->data_crc =  msg->footer.data_crc;
		sigblock->data_len = msg->hdr.data_len;
		sigblock->seq_lower_word = *(__le32 *)&msg->hdr.seq;

		/* no leading len, no ceph_x_encrypt_header */
		ret = ceph_crypt(&au->session_key, true, enc_buf,
				 CEPHX_AU_ENC_BUF_LEN, sizeof(*sigblock),
				 &ciphertext_len);
		if (ret)
			return ret;

		*psig = penc->a ^ penc->b ^ penc->c ^ penc->d;
	}

	return 0;
}

static int ceph_x_sign_message(struct ceph_auth_handshake *auth,
			       struct ceph_msg *msg)
{
	__le64 sig;
	int ret;

	if (ceph_test_opt(from_msgr(msg->con->msgr), NOMSGSIGN))
		return 0;

	ret = calc_signature((struct ceph_x_authorizer *)auth->authorizer,
			     msg, &sig);
	if (ret)
		return ret;

	msg->footer.sig = sig;
	msg->footer.flags |= CEPH_MSG_FOOTER_SIGNED;
	return 0;
}

static int ceph_x_check_message_signature(struct ceph_auth_handshake *auth,
					  struct ceph_msg *msg)
{
	__le64 sig_check;
	int ret;

	if (ceph_test_opt(from_msgr(msg->con->msgr), NOMSGSIGN))
		return 0;

	ret = calc_signature((struct ceph_x_authorizer *)auth->authorizer,
			     msg, &sig_check);
	if (ret)
		return ret;
	if (sig_check == msg->footer.sig)
		return 0;
	if (msg->footer.flags & CEPH_MSG_FOOTER_SIGNED)
		dout("ceph_x_check_message_signature %p has signature %llx "
		     "expect %llx\n", msg, msg->footer.sig, sig_check);
	else
		dout("ceph_x_check_message_signature %p sender did not set "
		     "CEPH_MSG_FOOTER_SIGNED\n", msg);
	return -EBADMSG;
}

static const struct ceph_auth_client_ops ceph_x_ops = {
	.name = "x",
	.is_authenticated = ceph_x_is_authenticated,
	.should_authenticate = ceph_x_should_authenticate,
	.build_request = ceph_x_build_request,
	.handle_reply = ceph_x_handle_reply,
	.create_authorizer = ceph_x_create_authorizer,
	.update_authorizer = ceph_x_update_authorizer,
	.add_authorizer_challenge = ceph_x_add_authorizer_challenge,
	.verify_authorizer_reply = ceph_x_verify_authorizer_reply,
	.invalidate_authorizer = ceph_x_invalidate_authorizer,
	.reset =  ceph_x_reset,
	.destroy = ceph_x_destroy,
	.sign_message = ceph_x_sign_message,
	.check_message_signature = ceph_x_check_message_signature,
};


int ceph_x_init(struct ceph_auth_client *ac)
{
	struct ceph_x_info *xi;
	int ret;

	dout("ceph_x_init %p\n", ac);
	ret = -ENOMEM;
	xi = kzalloc(sizeof(*xi), GFP_NOFS);
	if (!xi)
		goto out;

	ret = -EINVAL;
	if (!ac->key) {
		pr_err("no secret set (for auth_x protocol)\n");
		goto out_nomem;
	}

	ret = ceph_crypto_key_clone(&xi->secret, ac->key);
	if (ret < 0) {
		pr_err("cannot clone key: %d\n", ret);
		goto out_nomem;
	}

	xi->starting = true;
	xi->ticket_handlers = RB_ROOT;

	ac->protocol = CEPH_AUTH_CEPHX;
	ac->private = xi;
	ac->ops = &ceph_x_ops;
	return 0;

out_nomem:
	kfree(xi);
out:
	return ret;
}