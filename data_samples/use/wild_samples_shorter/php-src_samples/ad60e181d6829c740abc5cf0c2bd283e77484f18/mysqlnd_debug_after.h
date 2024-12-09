					if ((dbg_obj)) { \
						dbg_skip_trace = !(dbg_obj)->m->func_enter((dbg_obj), __LINE__, __FILE__, func_name, strlen(func_name)); \
					} \
					if (dbg_skip_trace); /* shut compiler's mouth */\ 
					do { \
						if ((dbg_obj) && (dbg_obj)->flags & MYSQLND_DEBUG_PROFILE_CALLS) { \
							DBG_PROFILE_START_TIME(); \
						} \