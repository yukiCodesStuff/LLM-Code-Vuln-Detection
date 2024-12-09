	if (ret != XpmSuccess) {
		return 0;
	}
	number = image.ncolors;
	for(i = 0; i < number; i++) {
		if (!image.colorTable[i].c_color) {
			goto done;
		}