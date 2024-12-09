#include <linux/dmi.h>
#include <linux/kthread.h>
#include <linux/acpi.h>
#include <linux/ctype.h>

#define PFX "ipmi_ssif: "
#define DEVICE_NAME "ipmi_ssif"


		do_gettimeofday(&t);
		pr_info("**Enqueue %02x %02x: %ld.%6.6ld\n",
		       msg->data[0], msg->data[1],
		       (long) t.tv_sec, (long) t.tv_usec);
	}
}

static int get_smi_info(void *send_info, struct ipmi_smi_info *data)