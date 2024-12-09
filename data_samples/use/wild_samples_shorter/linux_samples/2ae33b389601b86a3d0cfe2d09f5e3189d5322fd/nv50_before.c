static void
nv50_disp_base_vblank_enable(struct nouveau_event *event, int head)
{
	nv_mask(event->priv, 0x61002c, (1 << head), (1 << head));
}

static void
nv50_disp_base_vblank_disable(struct nouveau_event *event, int head)
{
	nv_mask(event->priv, 0x61002c, (1 << head), (0 << head));
}

static int
nv50_disp_base_ctor(struct nouveau_object *parent,