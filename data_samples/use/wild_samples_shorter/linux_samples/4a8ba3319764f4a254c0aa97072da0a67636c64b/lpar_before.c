#include <asm/trace.h>
#include <asm/firmware.h>
#include <asm/plpar_wrappers.h>
#include <asm/fadump.h>

#include "pseries.h"

		 * out to the user, but at least this will stop us from
		 * continuing on further and creating an even more
		 * difficult to debug situation.
		 */
		if (rc)
			panic("Could not enable big endian exceptions");
	}
#endif
}