 * This script controls its layout.
 */

#if defined(BUILD_VDSO64)
# define SHDR_SIZE 64
#elif defined(BUILD_VDSO32) || defined(BUILD_VDSOX32)
# define SHDR_SIZE 40
#else
# error unknown VDSO target
#endif

#define NUM_FAKE_SHDRS 13

SECTIONS
{
	. = SIZEOF_HEADERS;

	.gnu.version_d	: { *(.gnu.version_d) }
	.gnu.version_r	: { *(.gnu.version_r) }

	.dynamic	: { *(.dynamic) }		:text	:dynamic

	.rodata		: {
		*(.rodata*)
		*(.data*)
		*(.sdata*)
		*(.got.plt) *(.got)
		*(.gnu.linkonce.d.*)
		*(.bss*)
		*(.dynbss*)
		*(.gnu.linkonce.b.*)

		/*
		 * Ideally this would live in a C file, but that won't
		 * work cleanly for x32 until we start building the x32
		 * C code using an x32 toolchain.
		 */
		VDSO_FAKE_SECTION_TABLE_START = .;
		. = . + NUM_FAKE_SHDRS * SHDR_SIZE;
		VDSO_FAKE_SECTION_TABLE_END = .;
	}						:text

	.fake_shstrtab	: { *(.fake_shstrtab) }		:text


	.note		: { *(.note.*) }		:text	:note

	.eh_frame_hdr	: { *(.eh_frame_hdr) }		:text	:eh_frame_hdr
	.eh_frame	: { KEEP (*(.eh_frame)) }	:text


	/*
	 * Text is well-separated from actual data: there's plenty of
	 * stuff that isn't used at runtime in between.
	 */

	.text		: { *(.text*) }			:text	=0x90909090,

	/*
	 * At the end so that eu-elflint stays happy when vdso2c strips
	 * these.  A better implementation would avoid allocating space
	 * for these.
	 */
	.altinstructions	: { *(.altinstructions) }	:text
	.altinstr_replacement	: { *(.altinstr_replacement) }	:text

	/*
	 * The remainder of the vDSO consists of special pages that are
	 * shared between the kernel and userspace.  It needs to be at the
	 * end so that it doesn't overlap the mapping of the actual
	/DISCARD/ : {
		*(.discard)
		*(.discard.*)
		*(__bug_table)
	}
}

/*