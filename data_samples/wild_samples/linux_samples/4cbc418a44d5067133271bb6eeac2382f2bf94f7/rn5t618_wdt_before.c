	.driver = {
		.name	= DRIVER_NAME,
	},
};

module_platform_driver(rn5t618_wdt_driver);

MODULE_AUTHOR("Beniamino Galvani <b.galvani@gmail.com>");
MODULE_DESCRIPTION("RN5T618 watchdog driver");
MODULE_LICENSE("GPL v2");