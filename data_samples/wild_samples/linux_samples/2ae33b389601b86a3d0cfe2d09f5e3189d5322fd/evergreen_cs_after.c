	if (toffset & (surf.base_align - 1)) {
		dev_warn(p->dev, "%s:%d texture bo base %ld not aligned with %ld\n",
			 __func__, __LINE__, toffset, surf.base_align);
		return -EINVAL;
	}
	if (surf.nsamples <= 1 && moffset & (surf.base_align - 1)) {
		dev_warn(p->dev, "%s:%d mipmap bo base %ld not aligned with %ld\n",
			 __func__, __LINE__, moffset, surf.base_align);
		return -EINVAL;
	}
	if (dim == SQ_TEX_DIM_3D) {
		toffset += surf.layer_size * depth;
	} else {
		toffset += surf.layer_size * mslice;
	}