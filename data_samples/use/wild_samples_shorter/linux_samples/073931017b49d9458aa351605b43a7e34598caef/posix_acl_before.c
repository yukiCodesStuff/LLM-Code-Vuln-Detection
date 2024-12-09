}
EXPORT_SYMBOL_GPL(posix_acl_create);

/*
 * Fix up the uids and gids in posix acl extended attributes in place.
 */
static void posix_acl_fix_xattr_userns(