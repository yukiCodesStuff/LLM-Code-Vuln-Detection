{
	uint8_t pipe_num = 0;
	unsigned int thread_id;

	assert(pipeline != NULL);
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
	      "ia_css_pipeline_start() enter: pipe_id=%d, pipeline=%p\n",
	      pipe_id, pipeline);
	pipeline->pipe_id = pipe_id;
	sh_css_sp_init_pipeline(pipeline, pipe_id, pipe_num,
				false, false, false, true, SH_CSS_BDS_FACTOR_1_00,
				SH_CSS_PIPE_CONFIG_OVRD_NO_OVRD,
#ifndef ISP2401
				IA_CSS_INPUT_MODE_MEMORY, NULL, NULL
#else
				IA_CSS_INPUT_MODE_MEMORY, NULL, NULL,
#endif
#if !defined(HAS_NO_INPUT_SYSTEM)
#ifndef ISP2401
				, (mipi_port_ID_t) 0
#else
				(mipi_port_ID_t) 0,
#endif
#endif
#ifndef ISP2401
				);
#else
				NULL, NULL);
#endif
	ia_css_pipeline_get_sp_thread_id(pipe_num, &thread_id);
	if (!sh_css_sp_is_running()) {
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_pipeline_start() error,leaving\n");
		/* queues are invalid*/
		return;
	}
		    PIPELINE_SP_THREAD_EMPTY_TOKEN) {
			pipeline_sp_thread_list[i] =
			    PIPELINE_SP_THREAD_RESERVED_TOKEN;
			pipeline_num_to_sp_thread_map[pipe_num] = i;
			found_sp_thread = true;
			break;
		}
	}

	/* Make sure a mapping is found */
	/* I could do:
		assert(i < SH_CSS_MAX_SP_THREADS);

		But the below is more descriptive.
	*/
	assert(found_sp_thread != false);
}

static void pipeline_unmap_num_to_sp_thread(unsigned int pipe_num)
{
	unsigned int thread_id;
	assert(pipeline_num_to_sp_thread_map[pipe_num]
		!= (unsigned)PIPELINE_NUM_UNMAPPED);

	thread_id = pipeline_num_to_sp_thread_map[pipe_num];
	pipeline_num_to_sp_thread_map[pipe_num] = PIPELINE_NUM_UNMAPPED;
	pipeline_sp_thread_list[thread_id] = PIPELINE_SP_THREAD_EMPTY_TOKEN;
}

static enum ia_css_err pipeline_stage_create(
	struct ia_css_pipeline_stage_desc *stage_desc,
	struct ia_css_pipeline_stage **new_stage)
{
	enum ia_css_err err = IA_CSS_SUCCESS;
	struct ia_css_pipeline_stage *stage = NULL;
	struct ia_css_binary *binary;
	struct ia_css_frame *vf_frame;
	struct ia_css_frame *out_frame[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	const struct ia_css_fw_info *firmware;
	unsigned int i;

	/* Verify input parameters*/
	if (!(stage_desc->in_frame) && !(stage_desc->firmware)
	    && (stage_desc->binary) && !(stage_desc->binary->online)) {
		err = IA_CSS_ERR_INTERNAL_ERROR;
		goto ERR;
	}