	if (ret)
		return ret;

	return sysfs_emit(buf, "0x%X\n", le32_to_cpu(mesh_access.data[0]));
}

/**
 * anycast_mask_store - Set function for sysfs attribute anycast_mask
		return ret;

	retry_limit = le32_to_cpu(mesh_access.data[1]);
	return sysfs_emit(buf, "%d\n", retry_limit);
}

/**
 * prb_rsp_limit_store - Set function for sysfs attribute prb_rsp_limit
			     struct device_attribute *attr, char *buf)
{
	struct lbs_private *priv = to_net_dev(dev)->ml_priv;
	return sysfs_emit(buf, "0x%X\n", !!priv->mesh_dev);
}

/**
 * lbs_mesh_store - Set function for sysfs attribute mesh
	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", le32_to_cpu(defs.bootflag));
}

/**
 * bootflag_store - Set function for sysfs attribute bootflag
	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", defs.boottime);
}

/**
 * boottime_store - Set function for sysfs attribute boottime
	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", le16_to_cpu(defs.channel));
}

/**
 * channel_store - Set function for sysfs attribute channel
	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", defs.meshie.val.active_protocol_id);
}

/**
 * protocol_id_store - Set function for sysfs attribute protocol_id
	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", defs.meshie.val.active_metric_id);
}

/**
 * metric_id_store - Set function for sysfs attribute metric_id
	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", defs.meshie.val.mesh_capability);
}

/**
 * capability_store - Set function for sysfs attribute capability