
			...
			case (state_q)
				Idle: begin
				...
					else if ( (dm::dtm_op_e'(dmi.op) == dm::DTM_PASS) && (miss_pass_check_cnt_q != 2'b11) )
					begin
						state_d = Write;pass_mode = 1'b1;
					end
				...
				end
				...
				PassChkValid: begin
					if(hashValid) begin
						if(exp_hash == pass_hash) begin
							pass_check = 1'b1;
							end else begin
							pass_check = 1'b0;
							miss_pass_check_cnt_d = miss_pass_check_cnt_q + 1
							
							end
							state_d = Idle;
							
						end else begin
							state_d = PassChkValid;
						end
					end
				...
		
	