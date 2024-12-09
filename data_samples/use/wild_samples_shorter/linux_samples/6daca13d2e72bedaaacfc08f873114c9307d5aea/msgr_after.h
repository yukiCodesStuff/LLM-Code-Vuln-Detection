#define CEPH_MSGR_TAG_SEQ           13 /* 64-bit int follows with seen seq number */
#define CEPH_MSGR_TAG_KEEPALIVE2    14 /* keepalive2 byte + ceph_timespec */
#define CEPH_MSGR_TAG_KEEPALIVE2_ACK 15 /* keepalive2 reply */
#define CEPH_MSGR_TAG_CHALLENGE_AUTHORIZER 16  /* cephx v2 doing server challenge */

/*
 * connection negotiation
 */