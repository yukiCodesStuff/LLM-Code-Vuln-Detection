	struct list_head read_list;
	struct list_head pending_list;
	wait_queue_head_t wait;
	int iov_limit;
};

void vhost_dev_init(struct vhost_dev *, struct vhost_virtqueue **vqs,
		    int nvqs, int iov_limit);
long vhost_dev_set_owner(struct vhost_dev *dev);
bool vhost_dev_has_owner(struct vhost_dev *dev);
long vhost_dev_check_owner(struct vhost_dev *);
struct vhost_umem *vhost_dev_reset_owner_prepare(void);