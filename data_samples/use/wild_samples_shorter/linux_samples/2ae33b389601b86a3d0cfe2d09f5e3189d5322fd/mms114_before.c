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