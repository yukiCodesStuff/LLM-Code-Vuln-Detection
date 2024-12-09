				  unsigned long output_len,
				  unsigned long run_size)
{
	real_mode = rmode;

	sanitize_boot_params(real_mode);

	debug_putstr("\nDecompressing Linux... ");
	decompress(input_data, input_len, NULL, NULL, output, NULL, error);
	parse_elf(output);
	handle_relocations(output, output_len);
	debug_putstr("done.\nBooting the kernel.\n");
	return output;
}