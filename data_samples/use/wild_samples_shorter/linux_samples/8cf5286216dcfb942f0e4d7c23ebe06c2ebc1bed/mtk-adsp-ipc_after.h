
static inline void mtk_adsp_ipc_set_data(struct mtk_adsp_ipc *ipc, void *data)
{
	ipc->private_data = data;
}

static inline void *mtk_adsp_ipc_get_data(struct mtk_adsp_ipc *ipc)
{
	return ipc->private_data;
}

int mtk_adsp_ipc_send(struct mtk_adsp_ipc *ipc, unsigned int idx, uint32_t op);