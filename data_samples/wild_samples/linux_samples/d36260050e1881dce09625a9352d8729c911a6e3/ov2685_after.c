	.driver = {
		.name = "ov2685",
		.pm = &ov2685_pm_ops,
		.of_match_table = of_match_ptr(ov2685_of_match),
	},
	.probe		= &ov2685_probe,
	.remove		= &ov2685_remove,
};

module_i2c_driver(ov2685_i2c_driver);

MODULE_DESCRIPTION("OmniVision ov2685 sensor driver");
MODULE_LICENSE("GPL v2");