	struct vhost_msg_node *node = kmalloc(sizeof *node, GFP_KERNEL);
	if (!node)
		return NULL;
	node->vq = vq;
	node->msg.type = type;
	return node;
}