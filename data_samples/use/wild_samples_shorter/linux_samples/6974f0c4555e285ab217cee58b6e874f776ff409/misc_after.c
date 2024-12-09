	debug_putstr("done.\nBooting the kernel.\n");
	return output;
}

void fortify_panic(const char *name)
{
	error("detected buffer overflow");
}