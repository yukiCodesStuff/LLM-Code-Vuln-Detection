{
	struct snd_soc_dai *codec_dai = snd_soc_card_get_codec_dai(card, "rt286-aif1");

	return snd_soc_component_set_jack(codec_dai->component, NULL, NULL);
}

static int card_resume_post(struct snd_soc_card *card)
{
	struct snd_soc_dai *codec_dai = snd_soc_card_get_codec_dai(card, "rt286-aif1");

	return snd_soc_component_set_jack(codec_dai->component, &card_headset, NULL);
}

static struct snd_soc_card bdw_rt286_card = {
	.owner = THIS_MODULE,
	.suspend_pre = card_suspend_pre,
	.resume_post = card_resume_post,
	.dai_link = card_dai_links,
	.num_links = ARRAY_SIZE(card_dai_links),
	.controls = card_controls,
	.num_controls = ARRAY_SIZE(card_controls),
	.dapm_widgets = card_widgets,
	.num_dapm_widgets = ARRAY_SIZE(card_widgets),
	.dapm_routes = card_routes,
	.num_dapm_routes = ARRAY_SIZE(card_routes),
	.fully_routed = true,
};

/* Use space before codec name to simplify card ID, and simplify driver name. */
#define SOF_CARD_NAME "bdw rt286" /* card name will be 'sof-bdw rt286' */
#define SOF_DRIVER_NAME "SOF"

#define CARD_NAME "broadwell-rt286"

static int bdw_rt286_probe(struct platform_device *pdev)
{
	struct snd_soc_acpi_mach *mach;
	struct device *dev = &pdev->dev;
	int ret;

	bdw_rt286_card.dev = dev;
	mach = dev_get_platdata(dev);

	ret = snd_soc_fixup_dai_links_platform_name(&bdw_rt286_card, mach->mach_params.platform);
	if (ret)
		return ret;

	if (snd_soc_acpi_sof_parent(dev)) {
		bdw_rt286_card.name = SOF_CARD_NAME;
		bdw_rt286_card.driver_name = SOF_DRIVER_NAME;
	} else {
		bdw_rt286_card.name = CARD_NAME;
	}

	return devm_snd_soc_register_card(dev, &bdw_rt286_card);
}

static struct platform_driver bdw_rt286_driver = {
	.probe = bdw_rt286_probe,
	.driver = {
		.name = "bdw_rt286",
		.pm = &snd_soc_pm_ops
	},
};

module_platform_driver(bdw_rt286_driver)

MODULE_AUTHOR("Liam Girdwood, Xingchao Wang");
MODULE_DESCRIPTION("Sound card driver for Intel Broadwell Wildcat Point with Realtek 286");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bdw_rt286");