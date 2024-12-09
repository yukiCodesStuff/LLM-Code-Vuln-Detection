
               logic [511:0] bigData;
               ...
               
               hmac hmac(
                  
                  .clk_i(clk_i),
                  .rst_ni(rst_ni && ~rst_4),
                  .init_i(startHash && ~startHash_r),
                  .key_i(key),
                  .ikey_hash_i(ikey_hash), 
                  .okey_hash_i(okey_hash), 
                  .key_hash_bypass_i(key_hash_bypass),
                  .message_i(bigData),
                  .hash_o(hash),
                  .ready_o(ready),
                  .hash_valid_o(hashValid)
                  
               
			   