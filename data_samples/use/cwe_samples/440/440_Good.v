
               module csr_regfile #(...)(...);
               ...
               // ---------------------------
               // CSR Write and update logic
               // ---------------------------
               ...
                  
                  if (csr_we) begin
                     
                     unique case (csr_addr.address)
                     ...
                        
                        riscv::CSR_SIE: begin
                              
                              // the mideleg makes sure only delegate-able register
                              //(and therefore also only implemented registers) are written
                              mie_d = (mie_q & ~mideleg_q) | (csr_wdata & mideleg_q);
                              
                        end
                        ...
                        
                     endcase
                     
                  end
                  
               endmodule
               
            