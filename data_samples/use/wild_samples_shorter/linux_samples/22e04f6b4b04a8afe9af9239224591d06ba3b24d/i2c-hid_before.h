 * @hid_descriptor_address: i2c register where the HID descriptor is stored.
 *
 * Note that it is the responsibility of the platform driver (or the acpi 5.0
 * driver) to setup the irq related to the gpio in the struct i2c_board_info.
 * The platform driver should also setup the gpio according to the device:
 *
 * A typical example is the following:
 *	irq = gpio_to_irq(intr_gpio);