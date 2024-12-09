#include <net/sock.h>
#include <linux/virtio_vsock.h>
#include <linux/vhost.h>

#include <net/af_vsock.h>
#include "vhost.h"


/* Used to track all the vhost_vsock instances on the system. */
static DEFINE_SPINLOCK(vhost_vsock_lock);
static LIST_HEAD(vhost_vsock_list);

struct vhost_vsock {
	struct vhost_dev dev;
	struct vhost_virtqueue vqs[2];

	/* Link to global vhost_vsock_list, protected by vhost_vsock_lock */
	struct list_head list;

	struct vhost_work send_pkt_work;
	spinlock_t send_pkt_list_lock;
	struct list_head send_pkt_list;	/* host->guest pending packets */
	return VHOST_VSOCK_DEFAULT_HOST_CID;
}

static struct vhost_vsock *__vhost_vsock_get(u32 guest_cid)
{
	struct vhost_vsock *vsock;

	list_for_each_entry(vsock, &vhost_vsock_list, list) {
		u32 other_cid = vsock->guest_cid;

		/* Skip instances that have no CID yet */
		if (other_cid == 0)
	return NULL;
}

static struct vhost_vsock *vhost_vsock_get(u32 guest_cid)
{
	struct vhost_vsock *vsock;

	spin_lock_bh(&vhost_vsock_lock);
	vsock = __vhost_vsock_get(guest_cid);
	spin_unlock_bh(&vhost_vsock_lock);

	return vsock;
}

static void
vhost_transport_do_send_pkt(struct vhost_vsock *vsock,
			    struct vhost_virtqueue *vq)
{
	struct vhost_vsock *vsock;
	int len = pkt->len;

	/* Find the vhost_vsock according to guest context id  */
	vsock = vhost_vsock_get(le64_to_cpu(pkt->hdr.dst_cid));
	if (!vsock) {
		virtio_transport_free_pkt(pkt);
		return -ENODEV;
	}

	spin_unlock_bh(&vsock->send_pkt_list_lock);

	vhost_work_queue(&vsock->dev, &vsock->send_pkt_work);
	return len;
}

static int
	struct vhost_vsock *vsock;
	struct virtio_vsock_pkt *pkt, *n;
	int cnt = 0;
	LIST_HEAD(freeme);

	/* Find the vhost_vsock according to guest context id  */
	vsock = vhost_vsock_get(vsk->remote_addr.svm_cid);
	if (!vsock)
		return -ENODEV;

	spin_lock_bh(&vsock->send_pkt_list_lock);
	list_for_each_entry_safe(pkt, n, &vsock->send_pkt_list, list) {
		if (pkt->vsk != vsk)
			vhost_poll_queue(&tx_vq->poll);
	}

	return 0;
}

static struct virtio_vsock_pkt *
vhost_vsock_alloc_pkt(struct vhost_virtqueue *vq,
	spin_lock_init(&vsock->send_pkt_list_lock);
	INIT_LIST_HEAD(&vsock->send_pkt_list);
	vhost_work_init(&vsock->send_pkt_work, vhost_transport_send_pkt_work);

	spin_lock_bh(&vhost_vsock_lock);
	list_add_tail(&vsock->list, &vhost_vsock_list);
	spin_unlock_bh(&vhost_vsock_lock);
	return 0;

out:
	vhost_vsock_free(vsock);
	struct vhost_vsock *vsock = file->private_data;

	spin_lock_bh(&vhost_vsock_lock);
	list_del(&vsock->list);
	spin_unlock_bh(&vhost_vsock_lock);

	/* Iterating over all connections for all CIDs to find orphans is
	 * inefficient.  Room for improvement here. */
	vsock_for_each_connected_socket(vhost_vsock_reset_orphans);


	/* Refuse if CID is already in use */
	spin_lock_bh(&vhost_vsock_lock);
	other = __vhost_vsock_get(guest_cid);
	if (other && other != vsock) {
		spin_unlock_bh(&vhost_vsock_lock);
		return -EADDRINUSE;
	}
	vsock->guest_cid = guest_cid;
	spin_unlock_bh(&vhost_vsock_lock);

	return 0;
}