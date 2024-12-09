		    PROM_ERROR) {
			u32 width, height, pitch, addr;

			prom_printf("Setting btext !\n");
			prom_getprop(node, "width", &width, 4);
			prom_getprop(node, "height", &height, 4);
			prom_getprop(node, "linebytes", &pitch, 4);
			prom_getprop(node, "address", &addr, 4);
			prom_printf("W=%d H=%d LB=%d addr=0x%x\n",
				    width, height, pitch, addr);
			btext_setup_display(width, height, 8, pitch, addr);
		}
#endif /* CONFIG_PPC_EARLY_DEBUG_BOOTX */
	}
}


/* Return (relocated) pointer to this much memory: moves initrd if reqd. */
static void __init *make_room(unsigned long *mem_start, unsigned long *mem_end,
			      unsigned long needed, unsigned long align)
{
	void *ret;

	*mem_start = _ALIGN(*mem_start, align);
	while ((*mem_start + needed) > *mem_end) {
		unsigned long room, chunk;

		prom_debug("Chunk exhausted, claiming more at %lx...\n",
			   alloc_bottom);
		room = alloc_top - alloc_bottom;
		if (room > DEVTREE_CHUNK_SIZE)
			room = DEVTREE_CHUNK_SIZE;
		if (room < PAGE_SIZE)
			prom_panic("No memory for flatten_device_tree "
				   "(no room)\n");
		chunk = alloc_up(room, 0);
		if (chunk == 0)
			prom_panic("No memory for flatten_device_tree "
				   "(claim failed)\n");
		*mem_end = chunk + room;
	}

	ret = (void *)*mem_start;
	*mem_start += needed;

	return ret;
}

#define dt_push_token(token, mem_start, mem_end) do { 			\
		void *room = make_room(mem_start, mem_end, 4, 4);	\
		*(__be32 *)room = cpu_to_be32(token);			\
	} while(0)

static unsigned long __init dt_find_string(char *str)
{
	char *s, *os;

	s = os = (char *)dt_string_start;
	s += 4;
	while (s <  (char *)dt_string_end) {
		if (prom_strcmp(s, str) == 0)
			return s - os;
		s += prom_strlen(s) + 1;
	}
	return 0;
}

/*
 * The Open Firmware 1275 specification states properties must be 31 bytes or
 * less, however not all firmwares obey this. Make it 64 bytes to be safe.
 */
#define MAX_PROPERTY_NAME 64

static void __init scan_dt_build_strings(phandle node,
					 unsigned long *mem_start,
					 unsigned long *mem_end)
{
	char *prev_name, *namep, *sstart;
	unsigned long soff;
	phandle child;

	sstart =  (char *)dt_string_start;

	/* get and store all property names */
	prev_name = "";
	for (;;) {
		/* 64 is max len of name including nul. */
		namep = make_room(mem_start, mem_end, MAX_PROPERTY_NAME, 1);
		if (call_prom("nextprop", 3, 1, node, prev_name, namep) != 1) {
			/* No more nodes: unwind alloc */
			*mem_start = (unsigned long)namep;
			break;
		}

 		/* skip "name" */
		if (prom_strcmp(namep, "name") == 0) {
 			*mem_start = (unsigned long)namep;
 			prev_name = "name";
 			continue;
 		}
		/* get/create string entry */
		soff = dt_find_string(namep);
		if (soff != 0) {
			*mem_start = (unsigned long)namep;
			namep = sstart + soff;
		} else {
			/* Trim off some if we can */
			*mem_start = (unsigned long)namep + prom_strlen(namep) + 1;
			dt_string_end = *mem_start;
		}