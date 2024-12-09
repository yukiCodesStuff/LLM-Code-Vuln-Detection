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

		But the below is more descriptive.
	*/
	assert(found_sp_thread != false);
}

static void pipeline_unmap_num_to_sp_thread(unsigned int pipe_num)
{