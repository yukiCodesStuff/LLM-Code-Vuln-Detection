{
	struct ath6kl_urb_context *urb_context = NULL;
	unsigned long flags;

	spin_lock_irqsave(&pipe->ar_usb->cs_lock, flags);
	if (!list_empty(&pipe->urb_list_head)) {
		urb_context =
		    list_first_entry(&pipe->urb_list_head,
				     struct ath6kl_urb_context, link);
		list_del(&urb_context->link);
		pipe->urb_cnt--;
	}
{
	unsigned long flags;

	spin_lock_irqsave(&pipe->ar_usb->cs_lock, flags);
	pipe->urb_cnt++;

	list_add(&urb_context->link, &pipe->urb_list_head);
	spin_unlock_irqrestore(&pipe->ar_usb->cs_lock, flags);
}

static void ath6kl_usb_cleanup_recv_urb(struct ath6kl_urb_context *urb_context)
{
	dev_kfree_skb(urb_context->skb);
	urb_context->skb = NULL;

	ath6kl_usb_free_urb_to_pipe(urb_context->pipe, urb_context);
}

static inline struct ath6kl_usb *ath6kl_usb_priv(struct ath6kl *ar)
{
	return ar->hif_priv;
}

/* pipe resource allocation/cleanup */
static int ath6kl_usb_alloc_pipe_resources(struct ath6kl_usb_pipe *pipe,
					   int urb_cnt)
{
	struct ath6kl_urb_context *urb_context;
	int status = 0, i;

	INIT_LIST_HEAD(&pipe->urb_list_head);
	init_usb_anchor(&pipe->urb_submitted);

	for (i = 0; i < urb_cnt; i++) {
		urb_context = kzalloc(sizeof(struct ath6kl_urb_context),
				      GFP_KERNEL);
		if (urb_context == NULL) {
			status = -ENOMEM;
			goto fail_alloc_pipe_resources;
		}

		urb_context->pipe = pipe;

		/*
		 * we are only allocate the urb contexts here, the actual URB
		 * is allocated from the kernel as needed to do a transaction
		 */
		pipe->urb_alloc++;
		ath6kl_usb_free_urb_to_pipe(pipe, urb_context);
	}