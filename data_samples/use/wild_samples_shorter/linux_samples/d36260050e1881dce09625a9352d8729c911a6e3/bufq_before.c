
#endif

struct sh_css_queues  css_queues;


/*******************************************************
*** Static variables
********************************************************/
static int buffer_type_to_queue_id_map[SH_CSS_MAX_SP_THREADS][IA_CSS_NUM_DYNAMIC_BUFFER_TYPE];
static bool queue_availability[SH_CSS_MAX_SP_THREADS][SH_CSS_MAX_NUM_QUEUES];

/*******************************************************
	}

	for (i = SH_CSS_QUEUE_C_ID; i < SH_CSS_MAX_NUM_QUEUES; i++) {
		if (queue_availability[thread_id][i] == true) {
			queue_availability[thread_id][i] = false;
			buffer_type_to_queue_id_map[thread_id][buf_type] = i;
			break;
		}
	case sh_css_sp2host_isys_event_queue:
		q = &css_queues.sp2host_isys_event_queue_handle;
		break;
#endif		
	case sh_css_host2sp_tag_cmd_queue:
		q = &css_queues.host2sp_tag_cmd_queue_handle;
		break;
	default: