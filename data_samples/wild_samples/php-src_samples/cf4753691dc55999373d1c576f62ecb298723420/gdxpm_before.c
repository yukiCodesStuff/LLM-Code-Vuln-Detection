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
				buf[1] = '\0';
				buf[0] = image.colorTable[i].c_color[1];
				red = strtol(buf, NULL, 16);

				buf[0] = image.colorTable[i].c_color[2];
				green = strtol(buf, NULL, 16);

				buf[0] = image.colorTable[i].c_color[3];
				blue = strtol(buf, NULL, 16);
				break;

			case 7:
				buf[2] = '\0';
				buf[0] = image.colorTable[i].c_color[1];
				buf[1] = image.colorTable[i].c_color[2];
				red = strtol(buf, NULL, 16);

				buf[0] = image.colorTable[i].c_color[3];
				buf[1] = image.colorTable[i].c_color[4];
				green = strtol(buf, NULL, 16);

				buf[0] = image.colorTable[i].c_color[5];
				buf[1] = image.colorTable[i].c_color[6];
				blue = strtol(buf, NULL, 16);
				break;

			case 10:
				buf[3] = '\0';
				buf[0] = image.colorTable[i].c_color[1];
				buf[1] = image.colorTable[i].c_color[2];
				buf[2] = image.colorTable[i].c_color[3];
				red = strtol(buf, NULL, 16);
				red /= 64;

				buf[0] = image.colorTable[i].c_color[4];
				buf[1] = image.colorTable[i].c_color[5];
				buf[2] = image.colorTable[i].c_color[6];
				green = strtol(buf, NULL, 16);
				green /= 64;

				buf[0] = image.colorTable[i].c_color[7];
				buf[1] = image.colorTable[i].c_color[8];
				buf[2] = image.colorTable[i].c_color[9];
				blue = strtol(buf, NULL, 16);
				blue /= 64;
				break;

			case 13:
				buf[4] = '\0';
				buf[0] = image.colorTable[i].c_color[1];
				buf[1] = image.colorTable[i].c_color[2];
				buf[2] = image.colorTable[i].c_color[3];
				buf[3] = image.colorTable[i].c_color[4];
				red = strtol(buf, NULL, 16);
				red /= 256;

				buf[0] = image.colorTable[i].c_color[5];
				buf[1] = image.colorTable[i].c_color[6];
				buf[2] = image.colorTable[i].c_color[7];
				buf[3] = image.colorTable[i].c_color[8];
				green = strtol(buf, NULL, 16);
				green /= 256;

				buf[0] = image.colorTable[i].c_color[9];
				buf[1] = image.colorTable[i].c_color[10];
				buf[2] = image.colorTable[i].c_color[11];
				buf[3] = image.colorTable[i].c_color[12];
				blue = strtol(buf, NULL, 16);
				blue /= 256;
				break;
		}