	if (pb == NULL) {
		ret_val = -ENOMEM;
		goto err_parent_bus;
	}

	pb->switch_data = data;
	pb->switch_fn = switch_fn;
	pb->current_child = -1;
	pb->parent_id = parent_count++;
	pb->mii_bus = parent_bus;

	ret_val = -ENODEV;
	for_each_child_of_node(dev->of_node, child_bus_node) {
		u32 v;

		r = of_property_read_u32(child_bus_node, "reg", &v);
		if (r)
			continue;

		cb = devm_kzalloc(dev, sizeof(*cb), GFP_KERNEL);
		if (cb == NULL) {
			dev_err(dev,
				"Error: Failed to allocate memory for child\n");
			ret_val = -ENOMEM;
			break;
		}
		cb->bus_number = v;
		cb->parent = pb;
		cb->mii_bus = mdiobus_alloc();
		cb->mii_bus->priv = cb;

		cb->mii_bus->irq = cb->phy_irq;
		cb->mii_bus->name = "mdio_mux";
		snprintf(cb->mii_bus->id, MII_BUS_ID_SIZE, "%x.%x",
			 pb->parent_id, v);
		cb->mii_bus->parent = dev;
		cb->mii_bus->read = mdio_mux_read;
		cb->mii_bus->write = mdio_mux_write;
		r = of_mdiobus_register(cb->mii_bus, child_bus_node);
		if (r) {
			mdiobus_free(cb->mii_bus);
			devm_kfree(dev, cb);
		} else {
			of_node_get(child_bus_node);
			cb->next = pb->children;
			pb->children = cb;
		}
	}
	if (pb->children) {
		*mux_handle = pb;
		dev_info(dev, "Version " DRV_VERSION "\n");
		return 0;
	}
err_parent_bus:
	of_node_put(parent_bus_node);
	return ret_val;
}