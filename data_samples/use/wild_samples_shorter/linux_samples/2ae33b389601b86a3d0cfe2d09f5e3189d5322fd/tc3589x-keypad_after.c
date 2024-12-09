#define TC3589x_EVT_INT_CLR	0x2
#define TC3589x_KBD_INT_CLR	0x1

/**
 * struct tc_keypad - data structure used by keypad driver
 * @tc3589x:    pointer to tc35893
 * @input:      pointer to input device object
	const struct tc3589x_keypad_platform_data *board;
	unsigned int krow;
	unsigned int kcol;
	unsigned short *keymap;
	bool keypad_stopped;
};

static int tc3589x_keypad_init_key_hardware(struct tc_keypad *keypad)

	error = matrix_keypad_build_keymap(plat->keymap_data, NULL,
					   TC3589x_MAX_KPROW, TC3589x_MAX_KPCOL,
					   NULL, input);
	if (error) {
		dev_err(&pdev->dev, "Failed to build keymap\n");
		goto err_free_mem;
	}

	keypad->keymap = input->keycode;

	input_set_capability(input, EV_MSC, MSC_SCAN);
	if (!plat->no_autorepeat)
		__set_bit(EV_REP, input->evbit);
