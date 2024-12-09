	pti_init();
}

void __init __weak arch_call_rest_init(void)
{
	rest_init();
}