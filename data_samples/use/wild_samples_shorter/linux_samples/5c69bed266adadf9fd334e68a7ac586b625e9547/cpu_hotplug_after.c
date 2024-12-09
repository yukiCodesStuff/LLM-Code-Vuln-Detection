static int vcpu_online(unsigned int cpu)
{
	int err;
	char dir[16], state[16];

	sprintf(dir, "cpu/%u", cpu);
	err = xenbus_scanf(XBT_NIL, dir, "availability", "%15s", state);
	if (err != 1) {
		if (!xen_initial_domain())
			printk(KERN_ERR "XENBUS: Unable to read cpu state\n");
		return err;