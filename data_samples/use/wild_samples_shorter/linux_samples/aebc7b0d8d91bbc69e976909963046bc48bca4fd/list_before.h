	WRITE_ONCE(list->prev, list);
}

#ifdef CONFIG_DEBUG_LIST
/*
 * Performs the full set of list corruption checks before __list_add().
 * On list corruption reports a warning, and returns false.
 */
extern bool __list_add_valid_or_report(struct list_head *new,
				       struct list_head *prev,
				       struct list_head *next);

/*
 * Performs list corruption checks before __list_add(). Returns false if a
 * corruption is detected, true otherwise.
 */
static __always_inline bool __list_add_valid(struct list_head *new,
					     struct list_head *prev,
					     struct list_head *next)
{
	return __list_add_valid_or_report(new, prev, next);
}

/*
 * Performs the full set of list corruption checks before __list_del_entry().
 * On list corruption reports a warning, and returns false.
 */
extern bool __list_del_entry_valid_or_report(struct list_head *entry);

/*
 * Performs list corruption checks before __list_del_entry(). Returns false if a
 * corruption is detected, true otherwise.
 */
static __always_inline bool __list_del_entry_valid(struct list_head *entry)
{
	return __list_del_entry_valid_or_report(entry);
}
#else
static inline bool __list_add_valid(struct list_head *new,
				struct list_head *prev,