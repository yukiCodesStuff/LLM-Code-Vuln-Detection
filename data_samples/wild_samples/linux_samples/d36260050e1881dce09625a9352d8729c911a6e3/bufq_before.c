struct sh_css_queues {
	/* Host2SP buffer queue */
	ia_css_queue_t host2sp_buffer_queue_handles
		[SH_CSS_MAX_SP_THREADS][SH_CSS_MAX_NUM_QUEUES];
	/* SP2Host buffer queue */
	ia_css_queue_t sp2host_buffer_queue_handles
		[SH_CSS_MAX_NUM_QUEUES];

	/* Host2SP event queue */
	ia_css_queue_t host2sp_psys_event_queue_handle;

	/* SP2Host event queue */
	ia_css_queue_t sp2host_psys_event_queue_handle;

#if !defined(HAS_NO_INPUT_SYSTEM)
	/* Host2SP ISYS event queue */
	ia_css_queue_t host2sp_isys_event_queue_handle;

	/* SP2Host ISYS event queue */
	ia_css_queue_t sp2host_isys_event_queue_handle;

	/* Tagger command queue */
	ia_css_queue_t host2sp_tag_cmd_queue_handle;
#endif
};

#endif

struct sh_css_queues  css_queues;


/*******************************************************
*** Static variables
********************************************************/
static int buffer_type_to_queue_id_map[SH_CSS_MAX_SP_THREADS][IA_CSS_NUM_DYNAMIC_BUFFER_TYPE];
static bool queue_availability[SH_CSS_MAX_SP_THREADS][SH_CSS_MAX_NUM_QUEUES];

/*******************************************************
*** Static functions
********************************************************/
static void map_buffer_type_to_queue_id(
	unsigned int thread_id,
	enum ia_css_buffer_type buf_type
	);
static void unmap_buffer_type_to_queue_id(
	unsigned int thread_id,
	enum ia_css_buffer_type buf_type
	);

static ia_css_queue_t *bufq_get_qhandle(
	enum sh_css_queue_type type,
	enum sh_css_queue_id id,
	int thread
	);

/*******************************************************
*** Public functions
********************************************************/
void ia_css_queue_map_init(void)
{
	unsigned int i, j;

	for (i = 0; i < SH_CSS_MAX_SP_THREADS; i++) {
		for (j = 0; j < SH_CSS_MAX_NUM_QUEUES; j++)
			queue_availability[i][j] = true;
	}

	for (i = 0; i < SH_CSS_MAX_SP_THREADS; i++) {
		for (j = 0; j < IA_CSS_NUM_DYNAMIC_BUFFER_TYPE; j++)
			buffer_type_to_queue_id_map[i][j] = SH_CSS_INVALID_QUEUE_ID;
	}
}
	if (buf_type == IA_CSS_BUFFER_TYPE_PER_FRAME_PARAMETER_SET) {
		assert(queue_availability[thread_id][IA_CSS_PER_FRAME_PARAMETER_SET_QUEUE_ID]);
		queue_availability[thread_id][IA_CSS_PER_FRAME_PARAMETER_SET_QUEUE_ID] = false;
		buffer_type_to_queue_id_map[thread_id][buf_type] = IA_CSS_PER_FRAME_PARAMETER_SET_QUEUE_ID;
		return;
	}

	for (i = SH_CSS_QUEUE_C_ID; i < SH_CSS_MAX_NUM_QUEUES; i++) {
		if (queue_availability[thread_id][i] == true) {
			queue_availability[thread_id][i] = false;
			buffer_type_to_queue_id_map[thread_id][buf_type] = i;
			break;
		}
	}
	switch (type) {
	case sh_css_host2sp_buffer_queue:
		if ((thread >= SH_CSS_MAX_SP_THREADS) || (thread < 0) ||
			(id == SH_CSS_INVALID_QUEUE_ID))
			break;
		q = &css_queues.host2sp_buffer_queue_handles[thread][id];
		break;
	case sh_css_sp2host_buffer_queue:
		if (id == SH_CSS_INVALID_QUEUE_ID)
			break;
		q = &css_queues.sp2host_buffer_queue_handles[id];
		break;
	case sh_css_host2sp_psys_event_queue:
		q = &css_queues.host2sp_psys_event_queue_handle;
		break;
	case sh_css_sp2host_psys_event_queue:
		q = &css_queues.sp2host_psys_event_queue_handle;
		break;
#if !defined(HAS_NO_INPUT_SYSTEM)
	case sh_css_host2sp_isys_event_queue:
		q = &css_queues.host2sp_isys_event_queue_handle;
		break;
	case sh_css_sp2host_isys_event_queue:
		q = &css_queues.sp2host_isys_event_queue_handle;
		break;
#endif		
	case sh_css_host2sp_tag_cmd_queue:
		q = &css_queues.host2sp_tag_cmd_queue_handle;
		break;
	default:
		break;
	}

	return q;
}

/* Local function to initialize a buffer queue. This reduces
 * the chances of copy-paste errors or typos.
 */
static inline void
init_bufq(unsigned int desc_offset,
	  unsigned int elems_offset,
	  ia_css_queue_t *handle)
{
	const struct ia_css_fw_info *fw;
	unsigned int q_base_addr;
	ia_css_queue_remote_t remoteq;

	fw = &sh_css_sp_fw;
	q_base_addr = fw->info.sp.host_sp_queue;

	/* Setup queue location as SP and proc id as SP0_ID */
	remoteq.location = IA_CSS_QUEUE_LOC_SP;
	remoteq.proc_id = SP0_ID;
	remoteq.cb_desc_addr = q_base_addr + desc_offset;
	remoteq.cb_elems_addr = q_base_addr + elems_offset;
	/* Initialize the queue instance and obtain handle */
	ia_css_queue_remote_init(handle, &remoteq);
}

void ia_css_bufq_init(void)
{
	int i, j;

	IA_CSS_ENTER_PRIVATE("");

	/* Setup all the local queue descriptors for Host2SP Buffer Queues */
	for (i = 0; i < SH_CSS_MAX_SP_THREADS; i++)
		for (j = 0; j < SH_CSS_MAX_NUM_QUEUES; j++) {
			init_bufq((uint32_t)offsetof(struct host_sp_queues, host2sp_buffer_queues_desc[i][j]),
				  (uint32_t)offsetof(struct host_sp_queues, host2sp_buffer_queues_elems[i][j]),
				  &css_queues.host2sp_buffer_queue_handles[i][j]);
		}

	/* Setup all the local queue descriptors for SP2Host Buffer Queues */
	for (i = 0; i < SH_CSS_MAX_NUM_QUEUES; i++) {
		init_bufq(offsetof(struct host_sp_queues, sp2host_buffer_queues_desc[i]),
			  offsetof(struct host_sp_queues, sp2host_buffer_queues_elems[i]),
			  &css_queues.sp2host_buffer_queue_handles[i]);
	}

	/* Host2SP event queue*/
	init_bufq((uint32_t)offsetof(struct host_sp_queues, host2sp_psys_event_queue_desc),
		  (uint32_t)offsetof(struct host_sp_queues, host2sp_psys_event_queue_elems),
		  &css_queues.host2sp_psys_event_queue_handle);

	/* SP2Host event queue */
	init_bufq((uint32_t)offsetof(struct host_sp_queues, sp2host_psys_event_queue_desc),
		  (uint32_t)offsetof(struct host_sp_queues, sp2host_psys_event_queue_elems),
		  &css_queues.sp2host_psys_event_queue_handle);

#if !defined(HAS_NO_INPUT_SYSTEM)
	/* Host2SP ISYS event queue */
	init_bufq((uint32_t)offsetof(struct host_sp_queues, host2sp_isys_event_queue_desc),
		  (uint32_t)offsetof(struct host_sp_queues, host2sp_isys_event_queue_elems),
		  &css_queues.host2sp_isys_event_queue_handle);

	/* SP2Host ISYS event queue*/
	init_bufq((uint32_t)offsetof(struct host_sp_queues, sp2host_isys_event_queue_desc),
		  (uint32_t)offsetof(struct host_sp_queues, sp2host_isys_event_queue_elems),
		  &css_queues.sp2host_isys_event_queue_handle);

	/* Host2SP tagger command queue */
	init_bufq((uint32_t)offsetof(struct host_sp_queues, host2sp_tag_cmd_queue_desc),
		  (uint32_t)offsetof(struct host_sp_queues, host2sp_tag_cmd_queue_elems),
		  &css_queues.host2sp_tag_cmd_queue_handle);
#endif

	IA_CSS_LEAVE_PRIVATE("");
}

enum ia_css_err ia_css_bufq_enqueue_buffer(
	int thread_index,
	int queue_id,
	uint32_t item)
{
	enum ia_css_err return_err = IA_CSS_SUCCESS;
	ia_css_queue_t *q;
	int error;

	IA_CSS_ENTER_PRIVATE("queue_id=%d", queue_id);
	if ((thread_index >= SH_CSS_MAX_SP_THREADS) || (thread_index < 0) ||
			(queue_id == SH_CSS_INVALID_QUEUE_ID))
		return IA_CSS_ERR_INVALID_ARGUMENTS;

	/* Get the queue for communication */
	q = bufq_get_qhandle(sh_css_host2sp_buffer_queue,
		queue_id,
		thread_index);
	if (q != NULL) {
		error = ia_css_queue_enqueue(q, item);
		return_err = ia_css_convert_errno(error);
	} else {
		IA_CSS_ERROR("queue is not initialized");
		return_err = IA_CSS_ERR_RESOURCE_NOT_AVAILABLE;
	}