#include <linux/stacktrace.h>
#include <linux/prefetch.h>
#include <linux/memcontrol.h>

#include <trace/events/kmem.h>

#include "internal.h"
 * 			Core slab cache functions
 *******************************************************************/

static inline void *get_freepointer(struct kmem_cache *s, void *object)
{
	return *(void **)(object + s->offset);
}

static void prefetch_freepointer(const struct kmem_cache *s, void *object)
{
	prefetch(object + s->offset);
}

static inline void *get_freepointer_safe(struct kmem_cache *s, void *object)
{
	void *p;

	if (!debug_pagealloc_enabled())
		return get_freepointer(s, object);

	probe_kernel_read(&p, (void **)(object + s->offset), sizeof(p));
	return p;
}

static inline void set_freepointer(struct kmem_cache *s, void *object, void *fp)
{
	*(void **)(object + s->offset) = fp;
}

/* Loop over all objects in a slab */
#define for_each_object(__p, __s, __addr, __objects) \
{
	s->flags = kmem_cache_flags(s->size, flags, s->name, s->ctor);
	s->reserved = 0;

	if (need_reserve_slab_rcu && (s->flags & SLAB_TYPESAFE_BY_RCU))
		s->reserved = sizeof(struct rcu_head);
