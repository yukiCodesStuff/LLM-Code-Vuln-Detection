		if (IS_ERR(tcon->sclk0)) {
			dev_err(dev, "Couldn't get the TCON channel 0 clock\n");
			return PTR_ERR(tcon->sclk0);
		}
	}

	if (tcon->quirks->has_channel_1) {
		tcon->sclk1 = devm_clk_get(dev, "tcon-ch1");
		if (IS_ERR(tcon->sclk1)) {
			dev_err(dev, "Couldn't get the TCON channel 1 clock\n");
			return PTR_ERR(tcon->sclk1);
		}
	}

	return 0;
}

static void sun4i_tcon_free_clocks(struct sun4i_tcon *tcon)
{
	clk_disable_unprepare(tcon->clk);
}

static int sun4i_tcon_init_irq(struct device *dev,
			       struct sun4i_tcon *tcon)
{
	struct platform_device *pdev = to_platform_device(dev);
	int irq, ret;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "Couldn't retrieve the TCON interrupt\n");
		return irq;
	}

	ret = devm_request_irq(dev, irq, sun4i_tcon_handler, 0,
			       dev_name(dev), tcon);
	if (ret) {
		dev_err(dev, "Couldn't request the IRQ\n");
		return ret;
	}

	return 0;
}

static struct regmap_config sun4i_tcon_regmap_config = {
	.reg_bits	= 32,
	.val_bits	= 32,
	.reg_stride	= 4,
	.max_register	= 0x800,
};

static int sun4i_tcon_init_regmap(struct device *dev,
				  struct sun4i_tcon *tcon)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct resource *res;
	void __iomem *regs;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	tcon->regs = devm_regmap_init_mmio(dev, regs,
					   &sun4i_tcon_regmap_config);
	if (IS_ERR(tcon->regs)) {
		dev_err(dev, "Couldn't create the TCON regmap\n");
		return PTR_ERR(tcon->regs);
	}

	/* Make sure the TCON is disabled and all IRQs are off */
	regmap_write(tcon->regs, SUN4I_TCON_GCTL_REG, 0);
	regmap_write(tcon->regs, SUN4I_TCON_GINT0_REG, 0);
	regmap_write(tcon->regs, SUN4I_TCON_GINT1_REG, 0);

	/* Disable IO lines and set them to tristate */
	regmap_write(tcon->regs, SUN4I_TCON0_IO_TRI_REG, ~0);
	regmap_write(tcon->regs, SUN4I_TCON1_IO_TRI_REG, ~0);

	return 0;
}

/*
 * On SoCs with the old display pipeline design (Display Engine 1.0),
 * the TCON is always tied to just one backend. Hence we can traverse
 * the of_graph upwards to find the backend our tcon is connected to,
 * and take its ID as our own.
 *
 * We can either identify backends from their compatible strings, which
 * means maintaining a large list of them. Or, since the backend is
 * registered and binded before the TCON, we can just go through the
 * list of registered backends and compare the device node.
 *
 * As the structures now store engines instead of backends, here this
 * function in fact searches the corresponding engine, and the ID is
 * requested via the get_id function of the engine.
 */
static struct sunxi_engine *
sun4i_tcon_find_engine_traverse(struct sun4i_drv *drv,
				struct device_node *node,
				u32 port_id)
{
	struct device_node *port, *ep, *remote;
	struct sunxi_engine *engine = ERR_PTR(-EINVAL);
	u32 reg = 0;

	port = of_graph_get_port_by_id(node, port_id);
	if (!port)
		return ERR_PTR(-EINVAL);

	/*
	 * This only works if there is only one path from the TCON
	 * to any display engine. Otherwise the probe order of the
	 * TCONs and display engines is not guaranteed. They may
	 * either bind to the wrong one, or worse, bind to the same
	 * one if additional checks are not done.
	 *
	 * Bail out if there are multiple input connections.
	 */
	if (of_get_available_child_count(port) != 1)
		goto out_put_port;

	/* Get the first connection without specifying an ID */
	ep = of_get_next_available_child(port, NULL);
	if (!ep)
		goto out_put_port;

	remote = of_graph_get_remote_port_parent(ep);
	if (!remote)
		goto out_put_ep;

	/* does this node match any registered engines? */
	list_for_each_entry(engine, &drv->engine_list, list)
		if (remote == engine->node)
			goto out_put_remote;

	/*
	 * According to device tree binding input ports have even id
	 * number and output ports have odd id. Since component with
	 * more than one input and one output (TCON TOP) exits, correct
	 * remote input id has to be calculated by subtracting 1 from
	 * remote output id. If this for some reason can't be done, 0
	 * is used as input port id.
	 */
	of_node_put(port);
	port = of_graph_get_remote_port(ep);
	if (!of_property_read_u32(port, "reg", &reg) && reg > 0)
		reg -= 1;

	/* keep looking through upstream ports */
	engine = sun4i_tcon_find_engine_traverse(drv, remote, reg);

out_put_remote:
	of_node_put(remote);
out_put_ep:
	of_node_put(ep);
out_put_port:
	of_node_put(port);

	return engine;
}

/*
 * The device tree binding says that the remote endpoint ID of any
 * connection between components, up to and including the TCON, of
 * the display pipeline should be equal to the actual ID of the local
 * component. Thus we can look at any one of the input connections of
 * the TCONs, and use that connection's remote endpoint ID as our own.
 *
 * Since the user of this function already finds the input port,
 * the port is passed in directly without further checks.
 */
static int sun4i_tcon_of_get_id_from_port(struct device_node *port)
{
	struct device_node *ep;
	int ret = -EINVAL;

	/* try finding an upstream endpoint */
	for_each_available_child_of_node(port, ep) {
		struct device_node *remote;
		u32 reg;

		remote = of_graph_get_remote_endpoint(ep);
		if (!remote)
			continue;

		ret = of_property_read_u32(remote, "reg", &reg);
		if (ret)
			continue;

		ret = reg;
	}

	return ret;
}

/*
 * Once we know the TCON's id, we can look through the list of
 * engines to find a matching one. We assume all engines have
 * been probed and added to the list.
 */
static struct sunxi_engine *sun4i_tcon_get_engine_by_id(struct sun4i_drv *drv,
							int id)
{
	struct sunxi_engine *engine;

	list_for_each_entry(engine, &drv->engine_list, list)
		if (engine->id == id)
			return engine;

	return ERR_PTR(-EINVAL);
}

static bool sun4i_tcon_connected_to_tcon_top(struct device_node *node)
{
	struct device_node *remote;
	bool ret = false;

	remote = of_graph_get_remote_node(node, 0, -1);
	if (remote) {
		ret = !!(IS_ENABLED(CONFIG_DRM_SUN8I_TCON_TOP) &&
			 of_match_node(sun8i_tcon_top_of_table, remote));
		of_node_put(remote);
	}

	return ret;
}

static int sun4i_tcon_get_index(struct sun4i_drv *drv)
{
	struct list_head *pos;
	int size = 0;

	/*
	 * Because TCON is added to the list at the end of the probe
	 * (after this function is called), index of the current TCON
	 * will be same as current TCON list size.
	 */
	list_for_each(pos, &drv->tcon_list)
		++size;

	return size;
}

/*
 * On SoCs with the old display pipeline design (Display Engine 1.0),
 * we assumed the TCON was always tied to just one backend. However
 * this proved not to be the case. On the A31, the TCON can select
 * either backend as its source. On the A20 (and likely on the A10),
 * the backend can choose which TCON to output to.
 *
 * The device tree binding says that the remote endpoint ID of any
 * connection between components, up to and including the TCON, of
 * the display pipeline should be equal to the actual ID of the local
 * component. Thus we should be able to look at any one of the input
 * connections of the TCONs, and use that connection's remote endpoint
 * ID as our own.
 *
 * However  the connections between the backend and TCON were assumed
 * to be always singular, and their endpoit IDs were all incorrectly
 * set to 0. This means for these old device trees, we cannot just look
 * up the remote endpoint ID of a TCON input endpoint. TCON1 would be
 * incorrectly identified as TCON0.
 *
 * This function first checks if the TCON node has 2 input endpoints.
 * If so, then the device tree is a corrected version, and it will use
 * sun4i_tcon_of_get_id() and sun4i_tcon_get_engine_by_id() from above
 * to fetch the ID and engine directly. If not, then it is likely an
 * old device trees, where the endpoint IDs were incorrect, but did not
 * have endpoint connections between the backend and TCON across
 * different display pipelines. It will fall back to the old method of
 * traversing the  of_graph to try and find a matching engine by device
 * node.
 *
 * In the case of single display pipeline device trees, either method
 * works.
 */
static struct sunxi_engine *sun4i_tcon_find_engine(struct sun4i_drv *drv,
						   struct device_node *node)
{
	struct device_node *port;
	struct sunxi_engine *engine;

	port = of_graph_get_port_by_id(node, 0);
	if (!port)
		return ERR_PTR(-EINVAL);

	/*
	 * Is this a corrected device tree with cross pipeline
	 * connections between the backend and TCON?
	 */
	if (of_get_child_count(port) > 1) {
		int id;

		/*
		 * When pipeline has the same number of TCONs and engines which
		 * are represented by frontends/backends (DE1) or mixers (DE2),
		 * we match them by their respective IDs. However, if pipeline
		 * contains TCON TOP, chances are that there are either more
		 * TCONs than engines (R40) or TCONs with non-consecutive ids.
		 * (H6). In that case it's easier just use TCON index in list
		 * as an id. That means that on R40, any 2 TCONs can be enabled
		 * in DT out of 4 (there are 2 mixers). Due to the design of
		 * TCON TOP, remaining 2 TCONs can't be connected to anything
		 * anyway.
		 */
		if (sun4i_tcon_connected_to_tcon_top(node))
			id = sun4i_tcon_get_index(drv);
		else
			id = sun4i_tcon_of_get_id_from_port(port);

		/* Get our engine by matching our ID */
		engine = sun4i_tcon_get_engine_by_id(drv, id);

		of_node_put(port);
		return engine;
	}

	/* Fallback to old method by traversing input endpoints */
	of_node_put(port);
	return sun4i_tcon_find_engine_traverse(drv, node, 0);
}

static int sun4i_tcon_bind(struct device *dev, struct device *master,
			   void *data)
{
	struct drm_device *drm = data;
	struct sun4i_drv *drv = drm->dev_private;
	struct sunxi_engine *engine;
	struct device_node *remote;
	struct sun4i_tcon *tcon;
	struct reset_control *edp_rstc;
	bool has_lvds_rst, has_lvds_alt, can_lvds;
	int ret;

	engine = sun4i_tcon_find_engine(drv, dev->of_node);
	if (IS_ERR(engine)) {
		dev_err(dev, "Couldn't find matching engine\n");
		return -EPROBE_DEFER;
	}

	tcon = devm_kzalloc(dev, sizeof(*tcon), GFP_KERNEL);
	if (!tcon)
		return -ENOMEM;
	dev_set_drvdata(dev, tcon);
	tcon->drm = drm;
	tcon->dev = dev;
	tcon->id = engine->id;
	tcon->quirks = of_device_get_match_data(dev);

	tcon->lcd_rst = devm_reset_control_get(dev, "lcd");
	if (IS_ERR(tcon->lcd_rst)) {
		dev_err(dev, "Couldn't get our reset line\n");
		return PTR_ERR(tcon->lcd_rst);
	}

	if (tcon->quirks->needs_edp_reset) {
		edp_rstc = devm_reset_control_get_shared(dev, "edp");
		if (IS_ERR(edp_rstc)) {
			dev_err(dev, "Couldn't get edp reset line\n");
			return PTR_ERR(edp_rstc);
		}
		if (IS_ERR(tcon->sclk1)) {
			dev_err(dev, "Couldn't get the TCON channel 1 clock\n");
			return PTR_ERR(tcon->sclk1);
		}
	}

	return 0;
}

static void sun4i_tcon_free_clocks(struct sun4i_tcon *tcon)
{
	clk_disable_unprepare(tcon->clk);
}