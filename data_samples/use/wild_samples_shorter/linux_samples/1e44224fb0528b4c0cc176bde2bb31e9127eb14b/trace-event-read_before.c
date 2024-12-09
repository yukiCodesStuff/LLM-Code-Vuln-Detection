		for (x=0; x < count; x++) {
			size = read8(pevent);
			ret = read_event_file(pevent, sys, size);
			if (ret)
				return ret;
		}
	}
	return 0;
}
