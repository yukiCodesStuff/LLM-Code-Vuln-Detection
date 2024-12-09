	if (ret != XpmSuccess) {
		return 0;
	}
	number = image.ncolors;
	for(i = 0; i < number; i++) {
		if (!image.colorTable[i].c_color) {
			goto done;
		}
	}

	if (!(im = gdImageCreate(image.width, image.height))) {
		goto done;
	}

	colors = (int *) safe_emalloc(number, sizeof(int), 0);
	for (i = 0; i < number; i++) {
		switch (strlen (image.colorTable[i].c_color)) {
			case 4: