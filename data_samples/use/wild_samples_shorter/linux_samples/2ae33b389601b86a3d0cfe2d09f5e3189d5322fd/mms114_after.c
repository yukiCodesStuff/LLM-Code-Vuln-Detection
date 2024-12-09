	struct i2c_client *client = data->client;
	int error;

	error = regulator_enable(data->core_reg);
	if (error) {
		dev_err(&client->dev, "Failed to enable avdd: %d\n", error);
		return error;
	}

	error = regulator_enable(data->io_reg);
	if (error) {
		dev_err(&client->dev, "Failed to enable vdd: %d\n", error);
		regulator_disable(data->core_reg);
		return error;
	}

	mdelay(MMS114_POWERON_DELAY);

	error = mms114_setup_regs(data);
	if (error < 0) {
		regulator_disable(data->io_reg);
		regulator_disable(data->core_reg);
		return error;
	}

	if (data->pdata->cfg_pin)
		data->pdata->cfg_pin(true);

static void mms114_stop(struct mms114_data *data)
{
	struct i2c_client *client = data->client;
	int error;

	disable_irq(client->irq);

	if (data->pdata->cfg_pin)
		data->pdata->cfg_pin(false);

	error = regulator_disable(data->io_reg);
	if (error)
		dev_warn(&client->dev, "Failed to disable vdd: %d\n", error);

	error = regulator_disable(data->core_reg);
	if (error)
		dev_warn(&client->dev, "Failed to disable avdd: %d\n", error);
}

static int mms114_input_open(struct input_dev *dev)
{