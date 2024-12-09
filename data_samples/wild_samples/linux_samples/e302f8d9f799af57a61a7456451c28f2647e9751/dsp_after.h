struct imx_dsp_ipc {
	/* Host <-> DSP communication uses 2 txdb and 2 rxdb channels */
	struct imx_dsp_chan chans[DSP_MU_CHAN_NUM];
	struct device *dev;
	struct imx_dsp_ops *ops;
	void *private_data;
};

static inline void imx_dsp_set_data(struct imx_dsp_ipc *ipc, void *data)
{
	ipc->private_data = data;
}