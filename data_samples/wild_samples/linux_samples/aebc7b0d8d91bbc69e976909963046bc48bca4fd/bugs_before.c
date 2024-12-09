	else {
		pr_err("list_add() corruption not detected!\n");
		pr_expected_config(CONFIG_DEBUG_LIST);
	}
	else {
		pr_err("list_del() corruption not detected!\n");
		pr_expected_config(CONFIG_DEBUG_LIST);
	}