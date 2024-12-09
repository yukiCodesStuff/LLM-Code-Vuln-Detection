	int                    guid_set;
	char                   name[16];
	struct kref	       usecount;
};
#define to_bmc_device(x) container_of((x), struct bmc_device, pdev.dev)

/*

	return snprintf(buf, 10, "%u\n", bmc->id.device_id);
}
static DEVICE_ATTR(device_id, S_IRUGO, device_id_show, NULL);

static ssize_t provides_device_sdrs_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
	return snprintf(buf, 10, "%u\n",
			(bmc->id.device_revision & 0x80) >> 7);
}
static DEVICE_ATTR(provides_device_sdrs, S_IRUGO, provides_device_sdrs_show,
		   NULL);

static ssize_t revision_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	return snprintf(buf, 20, "%u\n",
			bmc->id.device_revision & 0x0F);
}
static DEVICE_ATTR(revision, S_IRUGO, revision_show, NULL);

static ssize_t firmware_revision_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
	return snprintf(buf, 20, "%u.%x\n", bmc->id.firmware_revision_1,
			bmc->id.firmware_revision_2);
}
static DEVICE_ATTR(firmware_revision, S_IRUGO, firmware_revision_show, NULL);

static ssize_t ipmi_version_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
			ipmi_version_major(&bmc->id),
			ipmi_version_minor(&bmc->id));
}
static DEVICE_ATTR(ipmi_version, S_IRUGO, ipmi_version_show, NULL);

static ssize_t add_dev_support_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
	return snprintf(buf, 10, "0x%02x\n",
			bmc->id.additional_device_support);
}
static DEVICE_ATTR(additional_device_support, S_IRUGO, add_dev_support_show,
		   NULL);

static ssize_t manufacturer_id_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)

	return snprintf(buf, 20, "0x%6.6x\n", bmc->id.manufacturer_id);
}
static DEVICE_ATTR(manufacturer_id, S_IRUGO, manufacturer_id_show, NULL);

static ssize_t product_id_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)

	return snprintf(buf, 10, "0x%4.4x\n", bmc->id.product_id);
}
static DEVICE_ATTR(product_id, S_IRUGO, product_id_show, NULL);

static ssize_t aux_firmware_rev_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
			bmc->id.aux_firmware_revision[1],
			bmc->id.aux_firmware_revision[0]);
}
static DEVICE_ATTR(aux_firmware_revision, S_IRUGO, aux_firmware_rev_show, NULL);

static ssize_t guid_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
			(long long) bmc->guid[0],
			(long long) bmc->guid[8]);
}
static DEVICE_ATTR(guid, S_IRUGO, guid_show, NULL);

static struct attribute *bmc_dev_attrs[] = {
	&dev_attr_device_id.attr,
	&dev_attr_provides_device_sdrs.attr,

	if (bmc->id.aux_firmware_revision_set)
		device_remove_file(&bmc->pdev.dev,
				   &dev_attr_aux_firmware_revision);
	if (bmc->guid_set)
		device_remove_file(&bmc->pdev.dev,
				   &dev_attr_guid);

	platform_device_unregister(&bmc->pdev);
}

	int err;

	if (bmc->id.aux_firmware_revision_set) {
		err = device_create_file(&bmc->pdev.dev,
					 &dev_attr_aux_firmware_revision);
		if (err)
			goto out;
	}
	if (bmc->guid_set) {
		err = device_create_file(&bmc->pdev.dev,
					 &dev_attr_guid);
		if (err)
			goto out_aux_firm;
	}

out_aux_firm:
	if (bmc->id.aux_firmware_revision_set)
		device_remove_file(&bmc->pdev.dev,
				   &dev_attr_aux_firmware_revision);
out:
	return err;
}
