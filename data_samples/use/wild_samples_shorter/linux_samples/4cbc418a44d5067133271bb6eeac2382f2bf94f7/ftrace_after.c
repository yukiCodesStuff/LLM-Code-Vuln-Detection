	}

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	avg = div64_ul(rec->time, rec->counter);
	if (tracing_thresh && (avg < tracing_thresh))
		goto out;
#endif

		 * Divide only 1000 for ns^2 -> us^2 conversion.
		 * trace_print_graph_duration will divide 1000 again.
		 */
		stddev = div64_ul(stddev,
				  rec->counter * (rec->counter - 1) * 1000);
	}

	trace_seq_init(&s);
	trace_print_graph_duration(rec->time, &s);