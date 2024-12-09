{
	return sprintf(buf, "Not affected\n");
}

static DEVICE_ATTR(meltdown, 0444, cpu_show_meltdown, NULL);
static DEVICE_ATTR(spectre_v1, 0444, cpu_show_spectre_v1, NULL);
static DEVICE_ATTR(spectre_v2, 0444, cpu_show_spectre_v2, NULL);
static DEVICE_ATTR(spec_store_bypass, 0444, cpu_show_spec_store_bypass, NULL);
static DEVICE_ATTR(l1tf, 0444, cpu_show_l1tf, NULL);

static struct attribute *cpu_root_vulnerabilities_attrs[] = {
	&dev_attr_meltdown.attr,
	&dev_attr_spectre_v1.attr,
	&dev_attr_spectre_v2.attr,
	&dev_attr_spec_store_bypass.attr,
	&dev_attr_l1tf.attr,
	NULL
};

static const struct attribute_group cpu_root_vulnerabilities_group = {
	.name  = "vulnerabilities",
	.attrs = cpu_root_vulnerabilities_attrs,
};

static void __init cpu_register_vulnerabilities(void)
{
	if (sysfs_create_group(&cpu_subsys.dev_root->kobj,
			       &cpu_root_vulnerabilities_group))
		pr_err("Unable to register CPU vulnerabilities\n");
}

#else
static inline void cpu_register_vulnerabilities(void) { }