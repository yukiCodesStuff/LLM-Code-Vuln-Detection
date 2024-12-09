{
	unsigned long freeptr_addr = (unsigned long)object + s->offset;

	*(void **)freeptr_addr = freelist_ptr(s, fp, freeptr_addr);
}

/* Loop over all objects in a slab */
#define for_each_object(__p, __s, __addr, __objects) \
	for (__p = fixup_red_left(__s, __addr); \
		__p < (__addr) + (__objects) * (__s)->size; \
		__p += (__s)->size)

#define for_each_object_idx(__p, __idx, __s, __addr, __objects) \
	for (__p = fixup_red_left(__s, __addr), __idx = 1; \
		__idx <= __objects; \
		__p += (__s)->size, __idx++)

/* Determine object index from a given position */
static inline int slab_index(void *p, struct kmem_cache *s, void *addr)
{
	return (p - addr) / s->size;
}

static inline int order_objects(int order, unsigned long size, int reserved)
{
	return ((PAGE_SIZE << order) - reserved) / size;
}

static inline struct kmem_cache_order_objects oo_make(int order,
		unsigned long size, int reserved)
{
	struct kmem_cache_order_objects x = {
		(order << OO_SHIFT) + order_objects(order, size, reserved)
	};

	return x;
}