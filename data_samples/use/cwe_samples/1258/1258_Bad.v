
                  module aes1_wrapper #(
                  ...
                  
                     assign core_key0 = debug_mode_i ? 'b0 : { 
                     
                        key_reg0[7],
                        key_reg0[6],
                        key_reg0[5],
                        key_reg0[4],
                        key_reg0[3],
                        key_reg0[2],
                        key_reg0[1],
                        key_reg0[0]};
                     
                     assign core_key1 = { 
                     
                        key_reg1[7],
                        key_reg1[6],
                        key_reg1[5],
                        key_reg1[4],
                        key_reg1[3],
                        key_reg1[2],
                        key_reg1[1],
                        key_reg1[0]};
                     
                  
                  ...
                  endmodule
               
               