#include <linux/stacktrace.h>
#include <linux/prefetch.h>
#include <linux/memcontrol.h>
#include <linux/random.h>

#include <trace/events/kmem.h>

#include "internal.h"
 * 			Core slab cache functions
 *******************************************************************/

/*
 * Returns freelist pointer (ptr). With hardening, this is obfuscated
 * with an XOR of the address where the pointer is held and a per-cache
 * random number.
 */
static inline void *freelist_ptr(const struct kmem_cache *s, void *ptr,
				 unsigned long ptr_addr)
{
#ifdef CONFIG_SLAB_FREELIST_HARDENED
	return (void *)((unsigned long)ptr ^ s->random ^ ptr_addr);
#else
	return ptr;
#endif
}

/* Returns the freelist pointer recorded at location ptr_addr. */
static inline void *freelist_dereference(const struct kmem_cache *s,
					 void *ptr_addr)
{
	return freelist_ptr(s, (void *)*(unsigned long *)(ptr_addr),
			    (unsigned long)ptr_addr);
}

static inline void *get_freepointer(struct kmem_cache *s, void *object)
{
	return freelist_dereference(s, object + s->offset);
}

static void prefetch_freepointer(const struct kmem_cache *s, void *object)
{
	if (object)
		prefetch(freelist_dereference(s, object + s->offset));
}

static inline void *get_freepointer_safe(struct kmem_cache *s, void *object)
{
	unsigned long freepointer_addr;
	void *p;

	if (!debug_pagealloc_enabled())
		return get_freepointer(s, object);

	freepointer_addr = (unsigned long)object + s->offset;
	probe_kernel_read(&p, (void **)freepointer_addr, sizeof(p));
	return freelist_ptr(s, p, freepointer_addr);
}

static inline void set_freepointer(struct kmem_cache *s, void *object, void *fp)
{
	unsigned long freeptr_addr = (unsigned long)object + s->offset;

	*(void **)freeptr_addr = freelist_ptr(s, fp, freeptr_addr);
}

/* Loop over all objects in a slab */
#define for_each_object(__p, __s, __addr, __objects) \
{
	s->flags = kmem_cache_flags(s->size, flags, s->name, s->ctor);
	s->reserved = 0;
#ifdef CONFIG_SLAB_FREELIST_HARDENED
	s->random = get_random_long();
#endif

	if (need_reserve_slab_rcu && (s->flags & SLAB_TYPESAFE_BY_RCU))
		s->reserved = sizeof(struct rcu_head);
