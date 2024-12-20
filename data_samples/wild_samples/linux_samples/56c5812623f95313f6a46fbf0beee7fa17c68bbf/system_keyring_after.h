/* SPDX-License-Identifier: GPL-2.0-or-later */
/* System keyring containing trusted public keys.
 *
 * Copyright (C) 2013 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#ifndef _KEYS_SYSTEM_KEYRING_H
#define _KEYS_SYSTEM_KEYRING_H

#include <linux/key.h>

#ifdef CONFIG_SYSTEM_TRUSTED_KEYRING

extern int restrict_link_by_builtin_trusted(struct key *keyring,
					    const struct key_type *type,
					    const union key_payload *payload,
					    struct key *restriction_key);

#else
#define restrict_link_by_builtin_trusted restrict_link_reject
#endif

#ifdef CONFIG_SECONDARY_TRUSTED_KEYRING
extern int restrict_link_by_builtin_and_secondary_trusted(
	struct key *keyring,
	const struct key_type *type,
	const union key_payload *payload,
	struct key *restriction_key);
#else
#define restrict_link_by_builtin_and_secondary_trusted restrict_link_by_builtin_trusted
#endif

extern struct pkcs7_message *pkcs7;
#ifdef CONFIG_SYSTEM_BLACKLIST_KEYRING
extern int mark_hash_blacklisted(const char *hash);
extern int is_hash_blacklisted(const u8 *hash, size_t hash_len,
			       const char *type);
extern int is_binary_blacklisted(const u8 *hash, size_t hash_len);
#else
static inline int is_hash_blacklisted(const u8 *hash, size_t hash_len,
				      const char *type)
{
	return 0;
}
{
	return 0;
}
#endif

#ifdef CONFIG_SYSTEM_REVOCATION_LIST
extern int add_key_to_revocation_list(const char *data, size_t size);
extern int is_key_on_revocation_list(struct pkcs7_message *pkcs7);
#else
static inline int add_key_to_revocation_list(const char *data, size_t size)
{
	return 0;
}