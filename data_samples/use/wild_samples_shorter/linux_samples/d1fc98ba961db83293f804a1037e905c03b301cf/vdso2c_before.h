 * are built for 32-bit userspace.
 */

static void GOFUNC(void *addr, size_t len, FILE *outfile, const char *name)
{
	int found_load = 0;
	unsigned long load_size = -1;  /* Work around bogus warning */
	unsigned long data_size;
	Elf_Ehdr *hdr = (Elf_Ehdr *)addr;
	int i;
	unsigned long j;
	Elf_Shdr *symtab_hdr = NULL, *strtab_hdr, *secstrings_hdr,
		*alt_sec = NULL;
	Elf_Dyn *dyn = 0, *dyn_end = 0;
	const char *secstrings;
	uint64_t syms[NSYMS] = {};

	uint64_t fake_sections_value = 0, fake_sections_size = 0;

	Elf_Phdr *pt = (Elf_Phdr *)(addr + GET_LE(&hdr->e_phoff));

	/* Walk the segment table. */
	for (i = 0; i < GET_LE(&hdr->e_phnum); i++) {
		if (GET_LE(&pt[i].p_type) == PT_LOAD) {
	for (i = 0; dyn + i < dyn_end &&
		     GET_LE(&dyn[i].d_tag) != DT_NULL; i++) {
		typeof(dyn[i].d_tag) tag = GET_LE(&dyn[i].d_tag);
		if (tag == DT_REL || tag == DT_RELSZ ||
		    tag == DT_RELENT || tag == DT_TEXTREL)
			fail("vdso image contains dynamic relocations\n");
	}

		GET_LE(&hdr->e_shentsize)*GET_LE(&hdr->e_shstrndx);
	secstrings = addr + GET_LE(&secstrings_hdr->sh_offset);
	for (i = 0; i < GET_LE(&hdr->e_shnum); i++) {
		Elf_Shdr *sh = addr + GET_LE(&hdr->e_shoff) +
			GET_LE(&hdr->e_shentsize) * i;
		if (GET_LE(&sh->sh_type) == SHT_SYMTAB)
			symtab_hdr = sh;

	     i < GET_LE(&symtab_hdr->sh_size) / GET_LE(&symtab_hdr->sh_entsize);
	     i++) {
		int k;
		Elf_Sym *sym = addr + GET_LE(&symtab_hdr->sh_offset) +
			GET_LE(&symtab_hdr->sh_entsize) * i;
		const char *name = addr + GET_LE(&strtab_hdr->sh_offset) +
			GET_LE(&sym->st_name);

		for (k = 0; k < NSYMS; k++) {
			if (!strcmp(name, required_syms[k])) {
				if (syms[k]) {
					fail("duplicate symbol %s\n",
					     required_syms[k]);
				}
				syms[k] = GET_LE(&sym->st_value);
			}
		}

		if (!strcmp(name, "vdso_fake_sections")) {
			if (fake_sections_value)
				fail("duplicate vdso_fake_sections\n");
			fake_sections_value = GET_LE(&sym->st_value);
			fake_sections_size = GET_LE(&sym->st_size);
		}
	}

	/* Validate mapping addresses. */
	for (i = 0; i < sizeof(special_pages) / sizeof(special_pages[0]); i++) {
		if (!syms[i])
			continue;  /* The mapping isn't used; ignore it. */

		if (syms[i] % 4096)
			fail("%s must be a multiple of 4096\n",
			     required_syms[i]);
		if (syms[i] < data_size)
			fail("%s must be after the text mapping\n",
			     required_syms[i]);
		if (syms[sym_end_mapping] < syms[i] + 4096)
			fail("%s overruns end_mapping\n", required_syms[i]);
	}
	if (syms[sym_end_mapping] % 4096)
		fail("end_mapping must be a multiple of 4096\n");

	/* Remove sections or use fakes */
	if (fake_sections_size % sizeof(Elf_Shdr))
		fail("vdso_fake_sections size is not a multiple of %ld\n",
		     (long)sizeof(Elf_Shdr));
	PUT_LE(&hdr->e_shoff, fake_sections_value);
	PUT_LE(&hdr->e_shentsize, fake_sections_value ? sizeof(Elf_Shdr) : 0);
	PUT_LE(&hdr->e_shnum, fake_sections_size / sizeof(Elf_Shdr));
	PUT_LE(&hdr->e_shstrndx, SHN_UNDEF);

	if (!name) {
		fwrite(addr, load_size, 1, outfile);
		return;
	}
			(unsigned long)GET_LE(&alt_sec->sh_size));
	}
	for (i = 0; i < NSYMS; i++) {
		if (syms[i])
			fprintf(outfile, "\t.sym_%s = 0x%" PRIx64 ",\n",
				required_syms[i], syms[i]);
	}
	fprintf(outfile, "};\n");
}