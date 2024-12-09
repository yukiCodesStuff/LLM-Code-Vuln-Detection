#include <linux/uidgid.h>
#include <keys/system_keyring.h>
#include "blacklist.h"

static struct key *blacklist_keyring;

/*
 * The description must be a type prefix, a colon and then an even number of
 * hex digits.  The hash is kept in the description.
 */
}
EXPORT_SYMBOL_GPL(is_binary_blacklisted);

/*
 * Initialise the blacklist
 */
static int __init blacklist_init(void)
 * Must be initialised before we try and load the keys into the keyring.
 */
device_initcall(blacklist_init);