{
	int err;
	char dir[32], state[32];

	sprintf(dir, "cpu/%u", cpu);
	err = xenbus_scanf(XBT_NIL, dir, "availability", "%s", state);
	if (err != 1) {
		if (!xen_initial_domain())
			printk(KERN_ERR "XENBUS: Unable to read cpu state\n");
		return err;
	}

	if (strcmp(state, "online") == 0)
		return 1;
	else if (strcmp(state, "offline") == 0)
		return 0;

	printk(KERN_ERR "XENBUS: unknown state(%s) on CPU%d\n", state, cpu);
	return -EINVAL;
}