
static inline void imx_dsp_set_data(struct imx_dsp_ipc *ipc, void *data)
{
	if (!ipc)
		return;

	ipc->private_data = data;
}

static inline void *imx_dsp_get_data(struct imx_dsp_ipc *ipc)
{
	if (!ipc)
		return NULL;

	return ipc->private_data;
}

#if IS_ENABLED(CONFIG_IMX_DSP)