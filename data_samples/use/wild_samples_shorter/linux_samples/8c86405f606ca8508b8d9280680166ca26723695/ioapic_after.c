#include <linux/io.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/nospec.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/current.h>
#include <trace/events/kvm.h>
	default:
		{
			u32 redir_index = (ioapic->ioregsel - 0x10) >> 1;
			u64 redir_content = ~0ULL;

			if (redir_index < IOAPIC_NUM_PINS) {
				u32 index = array_index_nospec(
					redir_index, IOAPIC_NUM_PINS);

				redir_content = ioapic->redirtbl[index].bits;
			}

			result = (ioapic->ioregsel & 0x1) ?
			    (redir_content >> 32) & 0xffffffff :
			    redir_content & 0xffffffff;