	return crashing_cpu >= 0;
}

void relocate_new_kernel(unsigned long indirection_page, unsigned long reboot_code_buffer,
			 unsigned long start_address) __noreturn;

#ifdef CONFIG_KEXEC_FILE
extern const struct kexec_file_ops kexec_elf64_ops;

#ifdef CONFIG_IMA_KEXEC