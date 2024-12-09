	WRITE_ONCE(list->prev, list);
}

#ifdef CONFIG_LIST_HARDENED

#ifdef CONFIG_DEBUG_LIST
# define __list_valid_slowpath
#else
# define __list_valid_slowpath __cold __preserve_most
#endif

/*
 * Performs the full set of list corruption checks before __list_add().
 * On list corruption reports a warning, and returns false.
 */
extern bool __list_valid_slowpath __list_add_valid_or_report(struct list_head *new,
							     struct list_head *prev,
							     struct list_head *next);

/*
 * Performs list corruption checks before __list_add(). Returns false if a
 * corruption is detected, true otherwise.
 *
 * With CONFIG_LIST_HARDENED only, performs minimal list integrity checking
 * inline to catch non-faulting corruptions, and only if a corruption is
 * detected calls the reporting function __list_add_valid_or_report().
 */
static __always_inline bool __list_add_valid(struct list_head *new,
					     struct list_head *prev,
					     struct list_head *next)
{
	bool ret = true;

	if (!IS_ENABLED(CONFIG_DEBUG_LIST)) {
		/*
		 * With the hardening version, elide checking if next and prev
		 * are NULL, since the immediate dereference of them below would
		 * result in a fault if NULL.
		 *
		 * With the reduced set of checks, we can afford to inline the
		 * checks, which also gives the compiler a chance to elide some
		 * of them completely if they can be proven at compile-time. If
		 * one of the pre-conditions does not hold, the slow-path will
		 * show a report which pre-condition failed.
		 */
		if (likely(next->prev == prev && prev->next == next && new != prev && new != next))
			return true;
		ret = false;
	}

	ret &= __list_add_valid_or_report(new, prev, next);
	return ret;
}

/*
 * Performs the full set of list corruption checks before __list_del_entry().
 * On list corruption reports a warning, and returns false.
 */
extern bool __list_valid_slowpath __list_del_entry_valid_or_report(struct list_head *entry);

/*
 * Performs list corruption checks before __list_del_entry(). Returns false if a
 * corruption is detected, true otherwise.
 *
 * With CONFIG_LIST_HARDENED only, performs minimal list integrity checking
 * inline to catch non-faulting corruptions, and only if a corruption is
 * detected calls the reporting function __list_del_entry_valid_or_report().
 */
static __always_inline bool __list_del_entry_valid(struct list_head *entry)
{
	bool ret = true;

	if (!IS_ENABLED(CONFIG_DEBUG_LIST)) {
		struct list_head *prev = entry->prev;
		struct list_head *next = entry->next;

		/*
		 * With the hardening version, elide checking if next and prev
		 * are NULL, LIST_POISON1 or LIST_POISON2, since the immediate
		 * dereference of them below would result in a fault.
		 */
		if (likely(prev->next == entry && next->prev == entry))
			return true;
		ret = false;
	}

	ret &= __list_del_entry_valid_or_report(entry);
	return ret;
}
#else
static inline bool __list_add_valid(struct list_head *new,
				struct list_head *prev,