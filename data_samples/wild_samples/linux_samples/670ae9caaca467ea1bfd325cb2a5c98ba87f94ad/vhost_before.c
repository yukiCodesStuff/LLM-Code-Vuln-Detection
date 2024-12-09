{
	struct vhost_msg_node *node = kmalloc(sizeof *node, GFP_KERNEL);
	if (!node)
		return NULL;
	node->vq = vq;
	node->msg.type = type;
	return node;
}
EXPORT_SYMBOL_GPL(vhost_new_msg);

void vhost_enqueue_msg(struct vhost_dev *dev, struct list_head *head,
		       struct vhost_msg_node *node)
{
	spin_lock(&dev->iotlb_lock);
	list_add_tail(&node->node, head);
	spin_unlock(&dev->iotlb_lock);

	wake_up_interruptible_poll(&dev->wait, EPOLLIN | EPOLLRDNORM);
}
EXPORT_SYMBOL_GPL(vhost_enqueue_msg);

struct vhost_msg_node *vhost_dequeue_msg(struct vhost_dev *dev,
					 struct list_head *head)
{
	struct vhost_msg_node *node = NULL;

	spin_lock(&dev->iotlb_lock);
	if (!list_empty(head)) {
		node = list_first_entry(head, struct vhost_msg_node,
					node);
		list_del(&node->node);
	}