	p = (void *) (c->vc_origin + t * c->vc_size_row);

	while (count--) {
		scr_memcpyw(vgacon_scrollback_cur->data +
			    vgacon_scrollback_cur->tail,
			    p, c->vc_size_row);
