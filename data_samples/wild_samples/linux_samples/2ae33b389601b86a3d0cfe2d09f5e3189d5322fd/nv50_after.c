nv50_disp_curs_ofuncs = {
	.ctor = nv50_disp_curs_ctor,
	.dtor = nv50_disp_pioc_dtor,
	.init = nv50_disp_pioc_init,
	.fini = nv50_disp_pioc_fini,
	.rd32 = nv50_disp_chan_rd32,
	.wr32 = nv50_disp_chan_wr32,
};

/*******************************************************************************
 * Base display object
 ******************************************************************************/

static void
nv50_disp_base_vblank_enable(struct nouveau_event *event, int head)
{
	nv_mask(event->priv, 0x61002c, (4 << head), (4 << head));
}