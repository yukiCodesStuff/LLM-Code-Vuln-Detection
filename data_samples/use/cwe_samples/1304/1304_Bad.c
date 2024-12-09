
						void save_config_state()
						{
						
							void* cfg;
							
							cfg = get_config_state();
							save_config_state(cfg);
							
							go_to_sleep();
						
						}
						
						void restore_config_state()
						{
						
							void* cfg;
							cfg = get_config_file();
							load_config_file(cfg);
						
						}
					
					