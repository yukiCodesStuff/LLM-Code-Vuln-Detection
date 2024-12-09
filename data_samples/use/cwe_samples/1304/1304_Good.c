
						void save_config_state()
						{
						
							void* cfg;
							void* sha;
							
							cfg = get_config_state();
							save_config_state(cfg);
							
							// save hash(cfg) to trusted location
							sha = get_hash_of_config_state(cfg);
							save_hash(sha); 
							
							go_to_sleep();
						
						}
						
						void restore_config_state()
						{
						
							void* cfg;
							void* sha_1, sha_2;
							
							cfg = get_config_file();
							// restore hash of config from trusted memory
							sha_1 = get_persisted_sha_value();
							
							sha_2 = get_hash_of_config_state(cfg);
							if (sha_1 != sha_2)
							
								assert_error_and_halt();
							
							
							load_config_file(cfg);
						
						}
                    
                    