{
	if (vga_video_type < VIDEO_TYPE_EGAM)
		return -EINVAL;

	font->width = VGA_FONTWIDTH;
	font->height = c->vc_font.height;
	font->charcount = vga_512_chars ? 512 : 256;
	if (!font->data)
		return 0;
	return vgacon_do_font_op(&vgastate, font->data, 0, vga_512_chars);
}

static int vgacon_resize(struct vc_data *c, unsigned int width,
			 unsigned int height, unsigned int user)
{
	if (width % 2 || width > screen_info.orig_video_cols ||
	    height > (screen_info.orig_video_lines * vga_default_font_height)/
	    c->vc_font.height)
		/* let svgatextmode tinker with video timings and
		   return success */
		return (user) ? 0 : -EINVAL;

	if (con_is_visible(c) && !vga_is_gfx) /* who knows */
		vgacon_doresize(c, width, height);
	return 0;
}