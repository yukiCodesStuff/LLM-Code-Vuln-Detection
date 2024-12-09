	pb->mii_bus = parent_bus;

	ret_val = -ENODEV;
	for_each_available_child_of_node(dev->of_node, child_bus_node) {
		u32 v;

		r = of_property_read_u32(child_bus_node, "reg", &v);
		if (r)