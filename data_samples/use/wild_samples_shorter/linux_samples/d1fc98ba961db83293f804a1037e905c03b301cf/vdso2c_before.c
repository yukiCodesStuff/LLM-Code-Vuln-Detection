	sym_vvar_page,
	sym_hpet_page,
	sym_end_mapping,
};

const int special_pages[] = {
	sym_vvar_page,
	sym_hpet_page,
};

char const * const required_syms[] = {
	[sym_vvar_page] = "vvar_page",
	[sym_hpet_page] = "hpet_page",
	[sym_end_mapping] = "end_mapping",
	"VDSO32_NOTE_MASK",
	"VDSO32_SYSENTER_RETURN",
	"__kernel_vsyscall",
	"__kernel_sigreturn",
	"__kernel_rt_sigreturn",
};

__attribute__((format(printf, 1, 2))) __attribute__((noreturn))
static void fail(const char *format, ...)

#define NSYMS (sizeof(required_syms) / sizeof(required_syms[0]))

#define BITS 64
#define GOFUNC go64
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Phdr Elf64_Phdr
#define Elf_Sym Elf64_Sym
#define Elf_Dyn Elf64_Dyn
#include "vdso2c.h"
#undef BITS
#undef GOFUNC
#undef Elf_Ehdr
#undef Elf_Shdr
#undef Elf_Phdr
#undef Elf_Sym
#undef Elf_Dyn

#define BITS 32
#define GOFUNC go32
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Phdr Elf32_Phdr
#define Elf_Sym Elf32_Sym
#define Elf_Dyn Elf32_Dyn
#include "vdso2c.h"
#undef BITS
#undef GOFUNC
#undef Elf_Ehdr
#undef Elf_Shdr
#undef Elf_Phdr
#undef Elf_Sym
#undef Elf_Dyn

static void go(void *addr, size_t len, FILE *outfile, const char *name)
{
	Elf64_Ehdr *hdr = (Elf64_Ehdr *)addr;