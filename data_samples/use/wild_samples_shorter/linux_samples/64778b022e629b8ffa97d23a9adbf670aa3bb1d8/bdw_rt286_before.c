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