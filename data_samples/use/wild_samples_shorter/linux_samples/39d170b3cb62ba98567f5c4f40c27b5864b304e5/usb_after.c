	struct ath6kl_urb_context *urb_context = NULL;
	unsigned long flags;

	/* bail if this pipe is not initialized */
	if (!pipe->ar_usb)
		return NULL;

	spin_lock_irqsave(&pipe->ar_usb->cs_lock, flags);
	if (!list_empty(&pipe->urb_list_head)) {
		urb_context =
		    list_first_entry(&pipe->urb_list_head,
{
	unsigned long flags;

	/* bail if this pipe is not initialized */
	if (!pipe->ar_usb)
		return;

	spin_lock_irqsave(&pipe->ar_usb->cs_lock, flags);
	pipe->urb_cnt++;

	list_add(&urb_context->link, &pipe->urb_list_head);