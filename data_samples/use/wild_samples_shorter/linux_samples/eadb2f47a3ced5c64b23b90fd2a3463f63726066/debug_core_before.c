#include <linux/vmacache.h>
#include <linux/rcupdate.h>
#include <linux/irq.h>

#include <asm/cacheflush.h>
#include <asm/byteorder.h>
#include <linux/atomic.h>
				continue;
			kgdb_connected = 0;
		} else {
			error = gdb_serial_stub(ks);
		}

		if (error == DBG_PASS_EVENT) {