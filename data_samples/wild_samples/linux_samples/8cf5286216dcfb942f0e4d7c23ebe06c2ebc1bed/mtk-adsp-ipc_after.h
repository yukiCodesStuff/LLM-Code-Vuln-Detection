struct mtk_adsp_ipc {
	struct mtk_adsp_chan chans[MTK_ADSP_MBOX_NUM];
	struct device *dev;
	struct mtk_adsp_ipc_ops *ops;
	void *private_data;
};

static inline void mtk_adsp_ipc_set_data(struct mtk_adsp_ipc *ipc, void *data)
{
	ipc->private_data = data;
}