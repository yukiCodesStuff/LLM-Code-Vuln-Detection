		unsigned char __user *ured;
		unsigned char __user *ugreen;
		unsigned char __user *ublue;
		unsigned int index, count, i;

		if (get_user(index, &c->index) ||
		    __get_user(count, &c->count) ||
		    __get_user(ured, &c->red) ||
		unsigned char __user *ugreen;
		unsigned char __user *ublue;
		struct fb_cmap *cmap = &info->cmap;
		unsigned int index, count, i;
		u8 red, green, blue;

		if (get_user(index, &c->index) ||
		    __get_user(count, &c->count) ||