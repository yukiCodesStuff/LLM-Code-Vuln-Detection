	sym_vvar_page,
	sym_hpet_page,
	sym_end_mapping,
	sym_VDSO_FAKE_SECTION_TABLE_START,
	sym_VDSO_FAKE_SECTION_TABLE_END,
};

const int special_pages[] = {
	sym_vvar_page,
	sym_hpet_page,
};

struct vdso_sym {
	const char *name;
	bool export;
};

struct vdso_sym required_syms[] = {
	[sym_vvar_page] = {"vvar_page", true},
	[sym_hpet_page] = {"hpet_page", true},
	[sym_end_mapping] = {"end_mapping", true},
	[sym_VDSO_FAKE_SECTION_TABLE_START] = {
		"VDSO_FAKE_SECTION_TABLE_START", false
	},
	[sym_VDSO_FAKE_SECTION_TABLE_END] = {
		"VDSO_FAKE_SECTION_TABLE_END", false
	},
	{"VDSO32_NOTE_MASK", true},
	{"VDSO32_SYSENTER_RETURN", true},
	{"__kernel_vsyscall", true},
	{"__kernel_sigreturn", true},
	{"__kernel_rt_sigreturn", true},
};

__attribute__((format(printf, 1, 2))) __attribute__((noreturn))
static void fail(const char *format, ...)

#define NSYMS (sizeof(required_syms) / sizeof(required_syms[0]))

#define BITSFUNC3(name, bits) name##bits
#define BITSFUNC2(name, bits) BITSFUNC3(name, bits)
#define BITSFUNC(name) BITSFUNC2(name, ELF_BITS)

#define ELF_BITS_XFORM2(bits, x) Elf##bits##_##x
#define ELF_BITS_XFORM(bits, x) ELF_BITS_XFORM2(bits, x)
#define ELF(x) ELF_BITS_XFORM(ELF_BITS, x)

#define ELF_BITS 64
#include "vdso2c.h"
#undef ELF_BITS

#define ELF_BITS 32
#include "vdso2c.h"
#undef ELF_BITS

static void go(void *addr, size_t len, FILE *outfile, const char *name)
{
	Elf64_Ehdr *hdr = (Elf64_Ehdr *)addr;