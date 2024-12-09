// SPDX-License-Identifier: GPL-2.0-or-later
/* System hash blacklist.
 *
 * Copyright (C) 2016 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#define pr_fmt(fmt) "blacklist: "fmt
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/key.h>
#include <linux/key-type.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/seq_file.h>
#include <linux/uidgid.h>
#include <keys/system_keyring.h>
#include "blacklist.h"
#include "common.h"

static struct key *blacklist_keyring;

#ifdef CONFIG_SYSTEM_REVOCATION_LIST
extern __initconst const u8 revocation_certificate_list[];
extern __initconst const unsigned long revocation_certificate_list_size;
#endif

/*
 * The description must be a type prefix, a colon and then an even number of
 * hex digits.  The hash is kept in the description.
 */
static int blacklist_vet_description(const char *desc)
{
	int n = 0;

	if (*desc == ':')
		return -EINVAL;
	for (; *desc; desc++)
		if (*desc == ':')
			goto found_colon;
	return -EINVAL;

found_colon:
	desc++;
	for (; *desc; desc++) {
		if (!isxdigit(*desc) || isupper(*desc))
			return -EINVAL;
		n++;
	}

	if (n == 0 || n & 1)
		return -EINVAL;
	return 0;
}
{
	if (is_hash_blacklisted(hash, hash_len, "bin") == -EKEYREJECTED)
		return -EPERM;

	return 0;
}
EXPORT_SYMBOL_GPL(is_binary_blacklisted);

#ifdef CONFIG_SYSTEM_REVOCATION_LIST
/**
 * add_key_to_revocation_list - Add a revocation certificate to the blacklist
 * @data: The data blob containing the certificate
 * @size: The size of data blob
 */
int add_key_to_revocation_list(const char *data, size_t size)
{
	key_ref_t key;

	key = key_create_or_update(make_key_ref(blacklist_keyring, true),
				   "asymmetric",
				   NULL,
				   data,
				   size,
				   ((KEY_POS_ALL & ~KEY_POS_SETATTR) | KEY_USR_VIEW),
				   KEY_ALLOC_NOT_IN_QUOTA | KEY_ALLOC_BUILT_IN);

	if (IS_ERR(key)) {
		pr_err("Problem with revocation key (%ld)\n", PTR_ERR(key));
		return PTR_ERR(key);
	}

	return 0;
}
{
	const char *const *bl;

	if (register_key_type(&key_type_blacklist) < 0)
		panic("Can't allocate system blacklist key type\n");

	blacklist_keyring =
		keyring_alloc(".blacklist",
			      GLOBAL_ROOT_UID, GLOBAL_ROOT_GID, current_cred(),
			      (KEY_POS_ALL & ~KEY_POS_SETATTR) |
			      KEY_USR_VIEW | KEY_USR_READ |
			      KEY_USR_SEARCH,
			      KEY_ALLOC_NOT_IN_QUOTA |
			      KEY_ALLOC_SET_KEEP,
			      NULL, NULL);
	if (IS_ERR(blacklist_keyring))
		panic("Can't allocate system blacklist keyring\n");

	for (bl = blacklist_hashes; *bl; bl++)
		if (mark_hash_blacklisted(*bl) < 0)
			pr_err("- blacklisting failed\n");
	return 0;
}

/*
 * Must be initialised before we try and load the keys into the keyring.
 */
device_initcall(blacklist_init);

#ifdef CONFIG_SYSTEM_REVOCATION_LIST
/*
 * Load the compiled-in list of revocation X.509 certificates.
 */
static __init int load_revocation_certificate_list(void)
{
	if (revocation_certificate_list_size)
		pr_notice("Loading compiled-in revocation X.509 certificates\n");

	return load_certificate_list(revocation_certificate_list, revocation_certificate_list_size,
				     blacklist_keyring);
}