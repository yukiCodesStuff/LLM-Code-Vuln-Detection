		}
		reloc_func_desc = interp_load_addr;

		fput(interpreter);

		kfree(interp_elf_ex);
		kfree(interp_elf_phdata);
	kfree(interp_elf_ex);
	kfree(interp_elf_phdata);
out_free_file:
	if (interpreter)
		fput(interpreter);
out_free_ph:
	kfree(elf_phdata);