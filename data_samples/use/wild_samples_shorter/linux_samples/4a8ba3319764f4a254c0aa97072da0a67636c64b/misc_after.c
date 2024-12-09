				  unsigned long output_len,
				  unsigned long run_size)
{
	unsigned char *output_orig = output;

	real_mode = rmode;

	sanitize_boot_params(real_mode);

	debug_putstr("\nDecompressing Linux... ");
	decompress(input_data, input_len, NULL, NULL, output, NULL, error);
	parse_elf(output);
	/*
	 * 32-bit always performs relocations. 64-bit relocations are only
	 * needed if kASLR has chosen a different load address.
	 */
	if (!IS_ENABLED(CONFIG_X86_64) || output != output_orig)
		handle_relocations(output, output_len);
	debug_putstr("done.\nBooting the kernel.\n");
	return output;
}