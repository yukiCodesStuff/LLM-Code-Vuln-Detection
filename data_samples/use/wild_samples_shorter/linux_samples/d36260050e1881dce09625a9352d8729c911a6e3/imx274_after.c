
	err = imx274_write_reg(priv, IMX274_VFLIP_REG, val);
	if (err) {
		dev_err(&priv->client->dev, "VFLIP control error\n");
		return err;
	}

	dev_dbg(&priv->client->dev,