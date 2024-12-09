{
	void *p;

	if (!vgacon_scrollback_cur->data || !vgacon_scrollback_cur->size ||
	    c->vc_num != fg_console)
		return;

	p = (void *) (c->vc_origin + t * c->vc_size_row);

	while (count--) {
		if ((vgacon_scrollback_cur->tail + c->vc_size_row) >
		    vgacon_scrollback_cur->size)
			vgacon_scrollback_cur->tail = 0;

		scr_memcpyw(vgacon_scrollback_cur->data +
			    vgacon_scrollback_cur->tail,
			    p, c->vc_size_row);

		vgacon_scrollback_cur->cnt++;
		p += c->vc_size_row;
		vgacon_scrollback_cur->tail += c->vc_size_row;

		if (vgacon_scrollback_cur->tail >= vgacon_scrollback_cur->size)
			vgacon_scrollback_cur->tail = 0;

		if (vgacon_scrollback_cur->cnt > vgacon_scrollback_cur->rows)
			vgacon_scrollback_cur->cnt = vgacon_scrollback_cur->rows;

		vgacon_scrollback_cur->cur = vgacon_scrollback_cur->cnt;
	}