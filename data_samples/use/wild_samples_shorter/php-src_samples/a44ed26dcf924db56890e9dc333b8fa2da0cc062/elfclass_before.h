/*
 * Copyright (c) Christos Zoulas 2008.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
	swap = (u.c[sizeof(int32_t) - 1] + 1) != elfhdr.e_ident[EI_DATA];

	type = elf_getu16(swap, elfhdr.e_type);
	switch (type) {
#ifdef ELFCORE
	case ET_CORE:
		flags |= FLAGS_IS_CORE;
		if (dophn_core(ms, clazz, swap, fd,
		    (zend_off_t)elf_getu(swap, elfhdr.e_phoff),
		    elf_getu16(swap, elfhdr.e_phnum),
		    (size_t)elf_getu16(swap, elfhdr.e_phentsize),
		    fsize, &flags) == -1)
			return -1;
		break;
#endif
	case ET_EXEC:
	case ET_DYN:
		if (dophn_exec(ms, clazz, swap, fd,
		    (zend_off_t)elf_getu(swap, elfhdr.e_phoff),
		    elf_getu16(swap, elfhdr.e_phnum),
		    (size_t)elf_getu16(swap, elfhdr.e_phentsize),
		    fsize, &flags, elf_getu16(swap, elfhdr.e_shnum))
		    == -1)
			return -1;
		/*FALLTHROUGH*/
	case ET_REL:
		if (doshn(ms, clazz, swap, fd,
		    (zend_off_t)elf_getu(swap, elfhdr.e_shoff),
		    elf_getu16(swap, elfhdr.e_shnum),
		    (size_t)elf_getu16(swap, elfhdr.e_shentsize),
		    fsize, &flags, elf_getu16(swap, elfhdr.e_machine),
		    (int)elf_getu16(swap, elfhdr.e_shstrndx)) == -1)
			return -1;
		break;

	default:
		break;
	}
	return 1;