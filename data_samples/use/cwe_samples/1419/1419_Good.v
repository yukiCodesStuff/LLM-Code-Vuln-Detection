
                  register_example #( 
				  
					.REGISTER_WIDTH (32), 
					.REGISTER_DEFAULT (1225) // Correct default value set, to enable Secure_mode 
				  
				  ) Secure_Device_ID_example ( 
				  
					.Data_in (Data_in), 
					.Data_out (Secure_reg), 
					.Clk (Clk), 
					.resetn (resetn), 
					.write (write) 
				  
				  );
                
              