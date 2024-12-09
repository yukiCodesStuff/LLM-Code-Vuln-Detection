#include <asm/trace.h>
#include <asm/firmware.h>
#include <asm/plpar_wrappers.h>
#include <asm/kexec.h>
#include <asm/fadump.h>

#include "pseries.h"

		 * out to the user, but at least this will stop us from
		 * continuing on further and creating an even more
		 * difficult to debug situation.
		 *
		 * There is a known problem when kdump'ing, if cpus are offline
		 * the above call will fail. Rather than panicking again, keep
		 * going and hope the kdump kernel is also little endian, which
		 * it usually is.
		 */
		if (rc && !kdump_in_progress())
			panic("Could not enable big endian exceptions");
	}
#endif
}