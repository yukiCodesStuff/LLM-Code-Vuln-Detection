
/* offsets in shared sync bo of various structures */
#define EVO_SYNC(c, o) ((c) * 0x0100 + (o))
#define EVO_MAST_NTFY     EVO_SYNC(  0, 0x00)
#define EVO_FLIP_SEM0(c)  EVO_SYNC((c), 0x00)
#define EVO_FLIP_SEM1(c)  EVO_SYNC((c), 0x10)

#define EVO_CORE_HANDLE      (0xd1500000)
#define EVO_CHAN_HANDLE(t,i) (0xd15c0000 | (((t) & 0x00ff) << 8) | (i))
#define EVO_CHAN_OCLASS(t,c) ((nv_hclass(c) & 0xff00) | ((t) & 0x00ff))

struct nv50_sync {
	struct nv50_dmac base;
	struct {
		u32 offset;
		u16 value;
	} sem;
};

struct nv50_ovly {
	struct nv50_dmac base;
	return nv50_disp(dev)->sync;
}

void
nv50_display_flip_stop(struct drm_crtc *crtc)
{
	struct nv50_sync *sync = nv50_sync(crtc);
	u32 *push;

	push = evo_wait(sync, 8);
	if (push) {
		evo_mthd(push, 0x0084, 1);
		evo_data(push, 0x00000000);
		evo_mthd(push, 0x0094, 1);
		evo_data(push, 0x00000000);
		evo_mthd(push, 0x0080, 1);
		evo_data(push, 0x00000000);
		evo_kick(push, sync);
	}
}

int
nv50_display_flip_next(struct drm_crtc *crtc, struct drm_framebuffer *fb,
		       struct nouveau_channel *chan, u32 swap_interval)
{
	struct nouveau_framebuffer *nv_fb = nouveau_framebuffer(fb);
	struct nv50_disp *disp = nv50_disp(crtc->dev);
	struct nouveau_crtc *nv_crtc = nouveau_crtc(crtc);
	struct nv50_sync *sync = nv50_sync(crtc);
	u32 *push;
	int ret;

	swap_interval <<= 4;
	if (swap_interval == 0)
		swap_interval |= 0x100;
	if (unlikely(push == NULL))
		return -EBUSY;

	/* synchronise with the rendering channel, if necessary */
	if (likely(chan)) {
		ret = RING_SPACE(chan, 10);
		if (ret)
			return ret;

		if (nv_mclass(chan->object) < NV84_CHANNEL_IND_CLASS) {
			BEGIN_NV04(chan, 0, NV11_SUBCHAN_DMA_SEMAPHORE, 2);
			OUT_RING  (chan, NvEvoSema0 + nv_crtc->index);
			OUT_RING  (chan, sync->sem.offset);
			BEGIN_NV04(chan, 0, NV11_SUBCHAN_SEMAPHORE_RELEASE, 1);
			OUT_RING  (chan, 0xf00d0000 | sync->sem.value);
			BEGIN_NV04(chan, 0, NV11_SUBCHAN_SEMAPHORE_OFFSET, 2);
			OUT_RING  (chan, sync->sem.offset ^ 0x10);
			OUT_RING  (chan, 0x74b1e000);
			BEGIN_NV04(chan, 0, NV11_SUBCHAN_DMA_SEMAPHORE, 1);
			OUT_RING  (chan, NvSema);
		} else
		if (nv_mclass(chan->object) < NVC0_CHANNEL_IND_CLASS) {
			u64 offset = nv84_fence_crtc(chan, nv_crtc->index);
			offset += sync->sem.offset;

			BEGIN_NV04(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
			OUT_RING  (chan, upper_32_bits(offset));
			OUT_RING  (chan, lower_32_bits(offset));
			OUT_RING  (chan, 0xf00d0000 | sync->sem.value);
			OUT_RING  (chan, 0x00000002);
			BEGIN_NV04(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
			OUT_RING  (chan, upper_32_bits(offset));
			OUT_RING  (chan, lower_32_bits(offset ^ 0x10));
			OUT_RING  (chan, 0x74b1e000);
			OUT_RING  (chan, 0x00000001);
		} else {
			u64 offset = nv84_fence_crtc(chan, nv_crtc->index);
			offset += sync->sem.offset;

			BEGIN_NVC0(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
			OUT_RING  (chan, upper_32_bits(offset));
			OUT_RING  (chan, lower_32_bits(offset));
			OUT_RING  (chan, 0xf00d0000 | sync->sem.value);
			OUT_RING  (chan, 0x00001002);
			BEGIN_NVC0(chan, 0, NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH, 4);
			OUT_RING  (chan, upper_32_bits(offset));
			OUT_RING  (chan, lower_32_bits(offset ^ 0x10));
			OUT_RING  (chan, 0x74b1e000);
			OUT_RING  (chan, 0x00001001);
		}

		FIRE_RING (chan);
	} else {
		nouveau_bo_wr32(disp->sync, sync->sem.offset / 4,
				0xf00d0000 | sync->sem.value);
		evo_sync(crtc->dev);
	}

	/* queue the flip */
		evo_data(push, 0x40000000);
	}
	evo_mthd(push, 0x0088, 4);
	evo_data(push, sync->sem.offset);
	evo_data(push, 0xf00d0000 | sync->sem.value);
	evo_data(push, 0x74b1e000);
	evo_data(push, NvEvoSync);
	evo_mthd(push, 0x00a0, 2);
	evo_data(push, 0x00000000);
	evo_data(push, 0x00000000);
	evo_mthd(push, 0x0080, 1);
	evo_data(push, 0x00000000);
	evo_kick(push, sync);

	sync->sem.offset ^= 0x10;
	sync->sem.value++;
	return 0;
}

/******************************************************************************
	if (ret)
		goto out;

	head->sync.sem.offset = EVO_SYNC(1 + index, 0x00);

	/* allocate overlay resources */
	ret = nv50_pioc_create(disp->core, NV50_DISP_OIMM_CLASS, index,
			      &(struct nv50_display_oimm_class) {
int
nv50_display_init(struct drm_device *dev)
{
	u32 *push = evo_wait(nv50_mast(dev), 32);
	if (push) {
		evo_mthd(push, 0x0088, 1);
		evo_data(push, NvEvoSync);
		evo_kick(push, nv50_mast(dev));
		return 0;
	}

	return -EBUSY;
}

void
nv50_display_destroy(struct drm_device *dev)
			NV_WARN(drm, "failed to create encoder %d/%d/%d: %d\n",
				     dcbe->location, dcbe->type,
				     ffs(dcbe->or) - 1, ret);
		}
	}

	/* cull any connectors we created that don't have an encoder */