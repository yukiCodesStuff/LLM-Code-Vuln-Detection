	struct vhost_msg_node *node = kmalloc(sizeof *node, GFP_KERNEL);
	if (!node)
		return NULL;

	/* Make sure all padding within the structure is initialized. */
	memset(&node->msg, 0, sizeof node->msg);
	node->vq = vq;
	node->msg.type = type;
	return node;
}