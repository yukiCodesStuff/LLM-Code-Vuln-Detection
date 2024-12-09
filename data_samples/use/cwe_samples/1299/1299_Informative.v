
                        module foo_bar(data_out, data_in, incoming_id, address, clk, rst_n);
                        output [31:0] data_out;
                        input [31:0] data_in, incoming_id, address;
                        input clk, rst_n;
                        wire write_auth, addr_auth;
                        reg [31:0] data_out, acl_oh_allowlist, q;
                        assign write_auth = | (incoming_id & acl_oh_allowlist) ? 1 : 0; 
                        always @*
                        
                            acl_oh_allowlist <= 32'h8312; 
                        
                        assign addr_auth = (address == 32'hF00) ? 1: 0;
                        always @ (posedge clk or negedge rst_n)
                        
                            if (!rst_n)
                            
                                begin
                                
                                    q <= 32'h0;
                                    data_out <= 32'h0;
                                
                                end
                            
                            else
                            
                                begin
                                
                                    q <= (addr_auth & write_auth) ? data_in: q;
                                    data_out <= q;
                                
                                end
                            
                            end
                        
                        endmodule
                    
					