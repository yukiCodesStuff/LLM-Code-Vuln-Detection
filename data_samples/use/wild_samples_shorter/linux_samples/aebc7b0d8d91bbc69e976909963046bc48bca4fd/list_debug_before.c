 * Copyright 2006, Red Hat, Inc., Dave Jones
 * Released under the General Public License (GPL).
 *
 * This file contains the linked list validation for DEBUG_LIST.
 */

#include <linux/export.h>
#include <linux/list.h>
 * attempt).
 */

bool __list_add_valid_or_report(struct list_head *new, struct list_head *prev,
				struct list_head *next)
{
	if (CHECK_DATA_CORRUPTION(prev == NULL,
}
EXPORT_SYMBOL(__list_add_valid_or_report);

bool __list_del_entry_valid_or_report(struct list_head *entry)
{
	struct list_head *prev, *next;
