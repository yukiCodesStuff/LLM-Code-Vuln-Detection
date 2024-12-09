{
	struct i2c_client *client = data->client;
	int error;

	if (data->core_reg)
		regulator_enable(data->core_reg);
	if (data->io_reg)
		regulator_enable(data->io_reg);
	mdelay(MMS114_POWERON_DELAY);

	error = mms114_setup_regs(data);
	if (error < 0)
		return error;

	if (data->pdata->cfg_pin)
		data->pdata->cfg_pin(true);

	enable_irq(client->irq);

	return 0;
}

static void mms114_stop(struct mms114_data *data)
{
	struct i2c_client *client = data->client;

	disable_irq(client->irq);

	if (data->pdata->cfg_pin)
		data->pdata->cfg_pin(false);

	if (data->io_reg)
		regulator_disable(data->io_reg);
	if (data->core_reg)
		regulator_disable(data->core_reg);
}

static int mms114_input_open(struct input_dev *dev)
{
	struct mms114_data *data = input_get_drvdata(dev);

	return mms114_start(data);
}

static void mms114_input_close(struct input_dev *dev)
{
	struct mms114_data *data = input_get_drvdata(dev);

	mms114_stop(data);
}

#ifdef CONFIG_OF
static struct mms114_platform_data *mms114_parse_dt(struct device *dev)
{
	struct mms114_platform_data *pdata;
	struct device_node *np = dev->of_node;

	if (!np)
		return NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(dev, "failed to allocate platform data\n");
		return NULL;
	}
{
	struct i2c_client *client = data->client;

	disable_irq(client->irq);

	if (data->pdata->cfg_pin)
		data->pdata->cfg_pin(false);

	if (data->io_reg)
		regulator_disable(data->io_reg);
	if (data->core_reg)
		regulator_disable(data->core_reg);
}