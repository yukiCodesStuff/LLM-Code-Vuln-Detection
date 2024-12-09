	return crashing_cpu >= 0;
}

#ifdef CONFIG_KEXEC_FILE
extern const struct kexec_file_ops kexec_elf64_ops;

#ifdef CONFIG_IMA_KEXEC