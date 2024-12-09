#define restrict_link_by_builtin_and_secondary_trusted restrict_link_by_builtin_trusted
#endif

#ifdef CONFIG_SYSTEM_BLACKLIST_KEYRING
extern int mark_hash_blacklisted(const char *hash);
extern int is_hash_blacklisted(const u8 *hash, size_t hash_len,
			       const char *type);
}
#endif

#ifdef CONFIG_IMA_BLACKLIST_KEYRING
extern struct key *ima_blacklist_keyring;

static inline struct key *get_ima_blacklist_keyring(void)