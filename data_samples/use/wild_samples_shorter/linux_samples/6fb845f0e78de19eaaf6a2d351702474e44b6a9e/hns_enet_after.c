out_notify_fail:
	(void)cancel_work_sync(&priv->service_task);
out_read_prop_fail:
	/* safe for ACPI FW */
	of_node_put(to_of_node(priv->fwnode));
	free_netdev(ndev);
	return ret;
}

	set_bit(NIC_STATE_REMOVING, &priv->state);
	(void)cancel_work_sync(&priv->service_task);

	/* safe for ACPI FW */
	of_node_put(to_of_node(priv->fwnode));

	free_netdev(ndev);
	return 0;
}
