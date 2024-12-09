 * This script controls its layout.
 */

SECTIONS
{
	. = SIZEOF_HEADERS;

	.gnu.version_d	: { *(.gnu.version_d) }
	.gnu.version_r	: { *(.gnu.version_r) }

	.note		: { *(.note.*) }		:text	:note

	.eh_frame_hdr	: { *(.eh_frame_hdr) }		:text	:eh_frame_hdr
	.eh_frame	: { KEEP (*(.eh_frame)) }	:text

	.dynamic	: { *(.dynamic) }		:text	:dynamic

	.rodata		: { *(.rodata*) }		:text
	.data		: {
	      *(.data*)
	      *(.sdata*)
	      *(.got.plt) *(.got)
	      *(.gnu.linkonce.d.*)
	      *(.bss*)
	      *(.dynbss*)
	      *(.gnu.linkonce.b.*)
	}

	.altinstructions	: { *(.altinstructions) }
	.altinstr_replacement	: { *(.altinstr_replacement) }

	/*
	 * Align the actual code well away from the non-instruction data.
	 * This is the best thing for the I-cache.
	 */
	. = ALIGN(0x100);

	.text		: { *(.text*) }			:text	=0x90909090,

	/*
	 * The remainder of the vDSO consists of special pages that are
	 * shared between the kernel and userspace.  It needs to be at the
	 * end so that it doesn't overlap the mapping of the actual
	/DISCARD/ : {
		*(.discard)
		*(.discard.*)
	}
}

/*