		vsize,
		mm ? get_mm_rss(mm) : 0,
		rsslim,
		mm ? (permitted ? mm->start_code : 1) : 0,
		mm ? (permitted ? mm->end_code : 1) : 0,
		(permitted && mm) ? mm->start_stack : 0,
		esp,
		eip,
		/* The signal information here is obsolete.