#include <net/sock.h>
#include <linux/virtio_vsock.h>
#include <linux/vhost.h>
#include <linux/hashtable.h>

#include <net/af_vsock.h>
#include "vhost.h"


/* Used to track all the vhost_vsock instances on the system. */
static DEFINE_SPINLOCK(vhost_vsock_lock);
static DEFINE_READ_MOSTLY_HASHTABLE(vhost_vsock_hash, 8);

struct vhost_vsock {
	struct vhost_dev dev;
	struct vhost_virtqueue vqs[2];

	/* Link to global vhost_vsock_hash, writes use vhost_vsock_lock */
	struct hlist_node hash;

	struct vhost_work send_pkt_work;
	spinlock_t send_pkt_list_lock;
	struct list_head send_pkt_list;	/* host->guest pending packets */
	return VHOST_VSOCK_DEFAULT_HOST_CID;
}

/* Callers that dereference the return value must hold vhost_vsock_lock or the
 * RCU read lock.
 */
static struct vhost_vsock *vhost_vsock_get(u32 guest_cid)
{
	struct vhost_vsock *vsock;

	hash_for_each_possible_rcu(vhost_vsock_hash, vsock, hash, guest_cid) {
		u32 other_cid = vsock->guest_cid;

		/* Skip instances that have no CID yet */
		if (other_cid == 0)
	return NULL;
}

static void
vhost_transport_do_send_pkt(struct vhost_vsock *vsock,
			    struct vhost_virtqueue *vq)
{
	struct vhost_vsock *vsock;
	int len = pkt->len;

	rcu_read_lock();

	/* Find the vhost_vsock according to guest context id  */
	vsock = vhost_vsock_get(le64_to_cpu(pkt->hdr.dst_cid));
	if (!vsock) {
		rcu_read_unlock();
		virtio_transport_free_pkt(pkt);
		return -ENODEV;
	}

	spin_unlock_bh(&vsock->send_pkt_list_lock);

	vhost_work_queue(&vsock->dev, &vsock->send_pkt_work);

	rcu_read_unlock();
	return len;
}

static int
	struct vhost_vsock *vsock;
	struct virtio_vsock_pkt *pkt, *n;
	int cnt = 0;
	int ret = -ENODEV;
	LIST_HEAD(freeme);

	rcu_read_lock();

	/* Find the vhost_vsock according to guest context id  */
	vsock = vhost_vsock_get(vsk->remote_addr.svm_cid);
	if (!vsock)
		goto out;

	spin_lock_bh(&vsock->send_pkt_list_lock);
	list_for_each_entry_safe(pkt, n, &vsock->send_pkt_list, list) {
		if (pkt->vsk != vsk)
			vhost_poll_queue(&tx_vq->poll);
	}

	ret = 0;
out:
	rcu_read_unlock();
	return ret;
}

static struct virtio_vsock_pkt *
vhost_vsock_alloc_pkt(struct vhost_virtqueue *vq,
	spin_lock_init(&vsock->send_pkt_list_lock);
	INIT_LIST_HEAD(&vsock->send_pkt_list);
	vhost_work_init(&vsock->send_pkt_work, vhost_transport_send_pkt_work);
	return 0;

out:
	vhost_vsock_free(vsock);
	struct vhost_vsock *vsock = file->private_data;

	spin_lock_bh(&vhost_vsock_lock);
	if (vsock->guest_cid)
		hash_del_rcu(&vsock->hash);
	spin_unlock_bh(&vhost_vsock_lock);

	/* Wait for other CPUs to finish using vsock */
	synchronize_rcu();

	/* Iterating over all connections for all CIDs to find orphans is
	 * inefficient.  Room for improvement here. */
	vsock_for_each_connected_socket(vhost_vsock_reset_orphans);


	/* Refuse if CID is already in use */
	spin_lock_bh(&vhost_vsock_lock);
	other = vhost_vsock_get(guest_cid);
	if (other && other != vsock) {
		spin_unlock_bh(&vhost_vsock_lock);
		return -EADDRINUSE;
	}

	if (vsock->guest_cid)
		hash_del_rcu(&vsock->hash);

	vsock->guest_cid = guest_cid;
	hash_add_rcu(vhost_vsock_hash, &vsock->hash, guest_cid);
	spin_unlock_bh(&vhost_vsock_lock);

	return 0;
}