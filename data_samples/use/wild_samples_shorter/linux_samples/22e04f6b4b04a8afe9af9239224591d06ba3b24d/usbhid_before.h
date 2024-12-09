	unsigned int retry_delay;                                       /* Delay length in ms */
	struct work_struct reset_work;                                  /* Task context for resets */
	wait_queue_head_t wait;						/* For sleeping */
	int ledcount;							/* counting the number of active leds */

	struct work_struct led_work;					/* Task context for setting LEDs */
};

#define	hid_to_usb_dev(hid_dev) \
	container_of(hid_dev->dev.parent->parent, struct usb_device, dev)