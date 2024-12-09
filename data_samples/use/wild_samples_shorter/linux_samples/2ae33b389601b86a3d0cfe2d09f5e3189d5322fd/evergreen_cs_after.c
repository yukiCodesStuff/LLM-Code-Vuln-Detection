			 __func__, __LINE__, toffset, surf.base_align);
		return -EINVAL;
	}
	if (surf.nsamples <= 1 && moffset & (surf.base_align - 1)) {
		dev_warn(p->dev, "%s:%d mipmap bo base %ld not aligned with %ld\n",
			 __func__, __LINE__, moffset, surf.base_align);
		return -EINVAL;
	}