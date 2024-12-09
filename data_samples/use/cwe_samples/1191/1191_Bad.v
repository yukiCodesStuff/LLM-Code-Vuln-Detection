
			...
			PassChkValid: begin
				if(hashValid) begin
					if(exp_hash == pass_hash) begin
						pass_check = 1'b1;
						end else begin
						pass_check = 1'b0;
						end
						state_d = Idle;
						
					end else begin
						state_d = PassChkValid;
					end
				end
			...
		
		