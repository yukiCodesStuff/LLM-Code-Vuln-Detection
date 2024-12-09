	if (ret != XpmSuccess) {
		return 0;
	}

	if (!(im = gdImageCreate(image.width, image.height))) {
		goto done;
	}

	number = image.ncolors;
	colors = (int *) safe_emalloc(number, sizeof(int), 0);
	for (i = 0; i < number; i++) {
		switch (strlen (image.colorTable[i].c_color)) {
			case 4: