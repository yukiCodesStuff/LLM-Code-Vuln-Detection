#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <linux/stat.h>
#include <linux/init.h>
#endif
#ifdef CONFIG_ECONET_AUNUDP
	struct msghdr udpmsg;
	struct iovec iov[2];
	struct aunhdr ah;
	struct sockaddr_in udpdest;
	__kernel_size_t size;
	mm_segment_t oldfs;
	char *userbuf;
#endif

	/*
	 *	Check the flags.
		}
	}

	if (dev->type == ARPHRD_ECONET) {
		/* Real hardware Econet.  We're not worthy etc. */
#ifdef CONFIG_ECONET_NATIVE
		unsigned short proto = 0;
		int res;

		if (len + 15 > dev->mtu) {
			mutex_unlock(&econet_mutex);
			return -EMSGSIZE;
		}

		dev_hold(dev);

		skb = sock_alloc_send_skb(sk, len+LL_ALLOCATED_SPACE(dev),
					  msg->msg_flags & MSG_DONTWAIT, &err);
		return -ENETDOWN;		/* No socket - can't send */
	}

	if (len > 32768) {
		err = -E2BIG;
		goto error;
	}

	/* Make up a UDP datagram and hand it off to some higher intellect. */

	memset(&udpdest, 0, sizeof(udpdest));
	udpdest.sin_family = AF_INET;

	/* tack our header on the front of the iovec */
	size = sizeof(struct aunhdr);
	iov[0].iov_base = (void *)&ah;
	iov[0].iov_len = size;

	userbuf = vmalloc(len);
	if (userbuf == NULL) {
		err = -ENOMEM;
		goto error;
	}

	iov[1].iov_base = userbuf;
	iov[1].iov_len = len;
	err = memcpy_fromiovec(userbuf, msg->msg_iov, len);
	if (err)
		goto error_free_buf;

	/* Get a skbuff (no data, just holds our cb information) */
	if ((skb = sock_alloc_send_skb(sk, 0,
				       msg->msg_flags & MSG_DONTWAIT,
				       &err)) == NULL)
		goto error_free_buf;

	eb = (struct ec_cb *)&skb->cb;

	eb->cookie = saddr->cookie;
	udpmsg.msg_name = (void *)&udpdest;
	udpmsg.msg_namelen = sizeof(udpdest);
	udpmsg.msg_iov = &iov[0];
	udpmsg.msg_iovlen = 2;
	udpmsg.msg_control = NULL;
	udpmsg.msg_controllen = 0;
	udpmsg.msg_flags=0;

	oldfs = get_fs(); set_fs(KERNEL_DS);	/* More privs :-) */
	err = sock_sendmsg(udpsock, &udpmsg, size);
	set_fs(oldfs);

error_free_buf:
	vfree(userbuf);
#else
	err = -EPROTOTYPE;
#endif
	error:
	mutex_unlock(&econet_mutex);

	return err;
}