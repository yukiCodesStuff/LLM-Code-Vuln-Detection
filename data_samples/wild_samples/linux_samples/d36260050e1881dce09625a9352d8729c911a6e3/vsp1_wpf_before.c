	for (i = 0; i < vsp1->info->rpf_count; ++i) {
		struct vsp1_rwpf *input = pipe->inputs[i];

		if (!input)
			continue;

		srcrpf |= (!pipe->bru && pipe->num_inputs == 1)
			? VI6_WPF_SRCRPF_RPF_ACT_MST(input->entity.index)
			: VI6_WPF_SRCRPF_RPF_ACT_SUB(input->entity.index);
	}

	if (pipe->bru || pipe->num_inputs > 1)
		srcrpf |= pipe->bru->type == VSP1_ENTITY_BRU
			? VI6_WPF_SRCRPF_VIRACT_MST
			: VI6_WPF_SRCRPF_VIRACT2_MST;

	vsp1_wpf_write(wpf, dl, VI6_WPF_SRCRPF, srcrpf);

	/* Enable interrupts */
	vsp1_dl_list_write(dl, VI6_WPF_IRQ_STA(wpf->entity.index), 0);
	vsp1_dl_list_write(dl, VI6_WPF_IRQ_ENB(wpf->entity.index),
			   VI6_WFP_IRQ_ENB_DFEE);
}

static unsigned int wpf_max_width(struct vsp1_entity *entity,
				  struct vsp1_pipeline *pipe)
{
	struct vsp1_rwpf *wpf = to_rwpf(&entity->subdev);

	return wpf->flip.rotate ? 256 : wpf->max_width;
}

static void wpf_partition(struct vsp1_entity *entity,
			  struct vsp1_pipeline *pipe,
			  struct vsp1_partition *partition,
			  unsigned int partition_idx,
			  struct vsp1_partition_window *window)
{
	partition->wpf = *window;
}

static const struct vsp1_entity_operations wpf_entity_ops = {
	.destroy = vsp1_wpf_destroy,
	.configure = wpf_configure,
	.max_width = wpf_max_width,
	.partition = wpf_partition,
};

/* -----------------------------------------------------------------------------
 * Initialization and Cleanup
 */

struct vsp1_rwpf *vsp1_wpf_create(struct vsp1_device *vsp1, unsigned int index)
{
	struct vsp1_rwpf *wpf;
	char name[6];
	int ret;

	wpf = devm_kzalloc(vsp1->dev, sizeof(*wpf), GFP_KERNEL);
	if (wpf == NULL)
		return ERR_PTR(-ENOMEM);

	if (vsp1->info->gen == 2) {
		wpf->max_width = WPF_GEN2_MAX_WIDTH;
		wpf->max_height = WPF_GEN2_MAX_HEIGHT;
	} else {
		wpf->max_width = WPF_GEN3_MAX_WIDTH;
		wpf->max_height = WPF_GEN3_MAX_HEIGHT;
	}

	wpf->entity.ops = &wpf_entity_ops;
	wpf->entity.type = VSP1_ENTITY_WPF;
	wpf->entity.index = index;

	sprintf(name, "wpf.%u", index);
	ret = vsp1_entity_init(vsp1, &wpf->entity, name, 2, &wpf_ops,
			       MEDIA_ENT_F_PROC_VIDEO_PIXEL_FORMATTER);
	if (ret < 0)
		return ERR_PTR(ret);

	/* Initialize the display list manager. */
	wpf->dlm = vsp1_dlm_create(vsp1, index, 64);
	if (!wpf->dlm) {
		ret = -ENOMEM;
		goto error;
	}

	/* Initialize the control handler. */
	ret = wpf_init_controls(wpf);
	if (ret < 0) {
		dev_err(vsp1->dev, "wpf%u: failed to initialize controls\n",
			index);
		goto error;
	}

	v4l2_ctrl_handler_setup(&wpf->ctrls);

	return wpf;

error:
	vsp1_entity_destroy(&wpf->entity);
	return ERR_PTR(ret);
}