
/* offsets in shared sync bo of various structures */
#define EVO_SYNC(c, o) ((c) * 0x0100 + (o))
#define EVO_MAST_NTFY     EVO_SYNC(      0, 0x00)
#define EVO_FLIP_SEM0(c)  EVO_SYNC((c) + 1, 0x00)
#define EVO_FLIP_SEM1(c)  EVO_SYNC((c) + 1, 0x10)

#define EVO_CORE_HANDLE      (0xd1500000)
#define EVO_CHAN_HANDLE(t,i) (0xd15c0000 | (((t) & 0x00ff) << 8) | (i))
#define EVO_CHAN_OCLASS(t,c) ((nv_hclass(c) & 0xff00) | ((t) & 0x00ff))

struct nv50_sync {
	struct nv50_dmac base;
	u32 addr;
	u32 data;
};

struct nv50_ovly {
	struct nv50_dmac base;
	return nv50_disp(dev)->sync;
}

struct nv50_display_flip {
	struct nv50_disp *disp;
	struct nv50_sync *chan;
};

static bool
nv50_display_flip_wait(void *data)
{
	struct nv50_display_flip *flip = data;
	if (nouveau_bo_rd32(flip->disp->sync, flip->chan->addr / 4) ==
					      flip->chan->data);
		return true;
	usleep_range(1, 2);
	return false;
}

void
nv50_display_flip_stop(struct drm_crtc *crtc)
{
	struct nouveau_device *device = nouveau_dev(crtc->dev);
	struct nv50_display_flip flip = {
		.disp = nv50_disp(crtc->dev),
		.chan = nv50_sync(crtc),
	};
	u32 *push;

	push = evo_wait(flip.chan, 8);
	if (push) {
		evo_mthd(push, 0x0084, 1);
		evo_data(push, 0x00000000);
		evo_mthd(push, 0x0094, 1);
		evo_data(push, 0x00000000);
		evo_mthd(push, 0x0080, 1);
		evo_data(push, 0x00000000);
		evo_kick(push, flip.chan);
	}

	nv_wait_cb(device, nv50_display_flip_wait, &flip);
}

int
nv50_display_flip_next(struct drm_crtc *crtc, struct drm_framebuffer *fb,
		       struct nouveau_channel *chan, u32 swap_interval)
{
	struct nouveau_framebuffer *nv_fb = nouveau_framebuffer(fb);
	struct nouveau_crtc *nv_crtc = nouveau_crtc(crtc);
	struct nv50_sync *sync = nv50_sync(crtc);
	int head = nv_crtc->index, ret;
	u32 *push;

	swap_interval <<= 4;
	if (swap_interval == 0)
		swap_interval |= 0x100;
	if (unlikely(push == NULL))
		return -EBUSY;

	if (chan && nv_mclass(chan->object) < NV84_CHANNEL_IND_CLASS) {
		ret = RING_SPACE(chan, 8);
		if (ret)
			return ret;

		BEGIN_NV04(chan, 0, NV11_SUBCHAN_DMA_SEMAPHORE, 2);
		OUT_RING  (chan, NvEvoSema0 + head);
		OUT_RING  (chan, sync->addr ^ 0x10);
		BEGIN_NV04(chan, 0, NV11_SUBCHAN_SEMAPHORE_RELEASE, 1);
		OUT_RING  (chan, sync->data + 1);
		BEGIN_NV04(chan, 0, NV11_SUBCHAN_SEMAPHORE_OFFSET, 2);
		OUT_RING  (chan, sync->addr);
		OUT_RING  (chan, sync->data);
	} else
	if (chan && nv_mclass(chan->object) < NVC0_CHANNEL_IND_CLASS) {
		u64 addr = nv84_fence_crtc(chan, head) + sync->addr;
		ret = RING_SPACE(chan, 12);
		if (ret)
			return ret;

		BEGIN_NV04(chan, 0, NV11_SUBCHAN_DMA_SEMAPHORE, 1);
		OUT_RING  (chan, chan->vram);
		BEGIN_NV04(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
		OUT_RING  (chan, upper_32_bits(addr ^ 0x10));
		OUT_RING  (chan, lower_32_bits(addr ^ 0x10));
		OUT_RING  (chan, sync->data + 1);
		OUT_RING  (chan, NV84_SUBCHAN_SEMAPHORE_TRIGGER_WRITE_LONG);
		BEGIN_NV04(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
		OUT_RING  (chan, upper_32_bits(addr));
		OUT_RING  (chan, lower_32_bits(addr));
		OUT_RING  (chan, sync->data);
		OUT_RING  (chan, NV84_SUBCHAN_SEMAPHORE_TRIGGER_ACQUIRE_EQUAL);
	} else
	if (chan) {
		u64 addr = nv84_fence_crtc(chan, head) + sync->addr;
		ret = RING_SPACE(chan, 10);
		if (ret)
			return ret;

		BEGIN_NVC0(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
		OUT_RING  (chan, upper_32_bits(addr ^ 0x10));
		OUT_RING  (chan, lower_32_bits(addr ^ 0x10));
		OUT_RING  (chan, sync->data + 1);
		OUT_RING  (chan, NV84_SUBCHAN_SEMAPHORE_TRIGGER_WRITE_LONG |
				 NVC0_SUBCHAN_SEMAPHORE_TRIGGER_YIELD);
		BEGIN_NVC0(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
		OUT_RING  (chan, upper_32_bits(addr));
		OUT_RING  (chan, lower_32_bits(addr));
		OUT_RING  (chan, sync->data);
		OUT_RING  (chan, NV84_SUBCHAN_SEMAPHORE_TRIGGER_ACQUIRE_EQUAL |
				 NVC0_SUBCHAN_SEMAPHORE_TRIGGER_YIELD);
	}

	if (chan) {
		sync->addr ^= 0x10;
		sync->data++;
		FIRE_RING (chan);
	} else {
		evo_sync(crtc->dev);
	}

	/* queue the flip */
		evo_data(push, 0x40000000);
	}
	evo_mthd(push, 0x0088, 4);
	evo_data(push, sync->addr);
	evo_data(push, sync->data++);
	evo_data(push, sync->data);
	evo_data(push, NvEvoSync);
	evo_mthd(push, 0x00a0, 2);
	evo_data(push, 0x00000000);
	evo_data(push, 0x00000000);
	evo_mthd(push, 0x0080, 1);
	evo_data(push, 0x00000000);
	evo_kick(push, sync);
	return 0;
}

/******************************************************************************
	if (ret)
		goto out;

	head->sync.addr = EVO_FLIP_SEM0(index);
	head->sync.data = 0x00000000;

	/* allocate overlay resources */
	ret = nv50_pioc_create(disp->core, NV50_DISP_OIMM_CLASS, index,
			      &(struct nv50_display_oimm_class) {
int
nv50_display_init(struct drm_device *dev)
{
	struct nv50_disp *disp = nv50_disp(dev);
	struct drm_crtc *crtc;
	u32 *push;

	push = evo_wait(nv50_mast(dev), 32);
	if (!push)
		return -EBUSY;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct nv50_sync *sync = nv50_sync(crtc);
		nouveau_bo_wr32(disp->sync, sync->addr / 4, sync->data);
	}

	evo_mthd(push, 0x0088, 1);
	evo_data(push, NvEvoSync);
	evo_kick(push, nv50_mast(dev));
	return 0;
}

void
nv50_display_destroy(struct drm_device *dev)
			NV_WARN(drm, "failed to create encoder %d/%d/%d: %d\n",
				     dcbe->location, dcbe->type,
				     ffs(dcbe->or) - 1, ret);
			ret = 0;
		}
	}

	/* cull any connectors we created that don't have an encoder */