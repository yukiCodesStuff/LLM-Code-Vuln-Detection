#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <linux/stat.h>
#include <linux/init.h>
#endif
#ifdef CONFIG_ECONET_AUNUDP
	struct msghdr udpmsg;
	struct iovec iov[msg->msg_iovlen+1];
	struct aunhdr ah;
	struct sockaddr_in udpdest;
	__kernel_size_t size;
	int i;
	mm_segment_t oldfs;
#endif

	/*
	 *	Check the flags.
		}
	}

	if (len + 15 > dev->mtu) {
		mutex_unlock(&econet_mutex);
		return -EMSGSIZE;
	}

	if (dev->type == ARPHRD_ECONET) {
		/* Real hardware Econet.  We're not worthy etc. */
#ifdef CONFIG_ECONET_NATIVE
		unsigned short proto = 0;
		int res;

		dev_hold(dev);

		skb = sock_alloc_send_skb(sk, len+LL_ALLOCATED_SPACE(dev),
					  msg->msg_flags & MSG_DONTWAIT, &err);
		return -ENETDOWN;		/* No socket - can't send */
	}

	/* Make up a UDP datagram and hand it off to some higher intellect. */

	memset(&udpdest, 0, sizeof(udpdest));
	udpdest.sin_family = AF_INET;

	/* tack our header on the front of the iovec */
	size = sizeof(struct aunhdr);
	/*
	 * XXX: that is b0rken.  We can't mix userland and kernel pointers
	 * in iovec, since on a lot of platforms copy_from_user() will
	 * *not* work with the kernel and userland ones at the same time,
	 * regardless of what we do with set_fs().  And we are talking about
	 * econet-over-ethernet here, so "it's only ARM anyway" doesn't
	 * apply.  Any suggestions on fixing that code?		-- AV
	 */
	iov[0].iov_base = (void *)&ah;
	iov[0].iov_len = size;
	for (i = 0; i < msg->msg_iovlen; i++) {
		void __user *base = msg->msg_iov[i].iov_base;
		size_t iov_len = msg->msg_iov[i].iov_len;
		/* Check it now since we switch to KERNEL_DS later. */
		if (!access_ok(VERIFY_READ, base, iov_len)) {
			mutex_unlock(&econet_mutex);
			return -EFAULT;
		}
		iov[i+1].iov_base = base;
		iov[i+1].iov_len = iov_len;
		size += iov_len;
	}

	/* Get a skbuff (no data, just holds our cb information) */
	if ((skb = sock_alloc_send_skb(sk, 0,
				       msg->msg_flags & MSG_DONTWAIT,
				       &err)) == NULL) {
		mutex_unlock(&econet_mutex);
		return err;
	}

	eb = (struct ec_cb *)&skb->cb;

	eb->cookie = saddr->cookie;
	udpmsg.msg_name = (void *)&udpdest;
	udpmsg.msg_namelen = sizeof(udpdest);
	udpmsg.msg_iov = &iov[0];
	udpmsg.msg_iovlen = msg->msg_iovlen + 1;
	udpmsg.msg_control = NULL;
	udpmsg.msg_controllen = 0;
	udpmsg.msg_flags=0;

	oldfs = get_fs(); set_fs(KERNEL_DS);	/* More privs :-) */
	err = sock_sendmsg(udpsock, &udpmsg, size);
	set_fs(oldfs);
#else
	err = -EPROTOTYPE;
#endif
	mutex_unlock(&econet_mutex);

	return err;
}