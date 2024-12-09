static struct i2c_driver ov2685_i2c_driver = {
	.driver = {
		.name = "ov2685",
		.owner = THIS_MODULE,
		.pm = &ov2685_pm_ops,
		.of_match_table = of_match_ptr(ov2685_of_match),
	},
	.probe		= &ov2685_probe,