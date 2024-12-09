
	       module ocp2ahb
	      ( 
	      
	        ahb_hprot, 
	        ocp_mreqinfo 
	      
	      ); 
	      
	      output [1:0] ahb_hprot;        // output is 2 bit signal for AHB HPROT
	      input [4:0] ocp_mreqinfo;      // input is 5 bit signal from OCP MReqInfo
	      wire [6:0] p0_mreqinfo_o_temp; // OCP signal that transmits hardware identity of bus controller
	       
	      wire y;
	      
	      reg [1:0] ahb_hprot;
	      
	      // hardware identity of bus controller is in bits 5:1 of p0_mreqinfo_o_temp signal
	      assign p0_mreqinfo_o_temp[6:0] = {1'b0, ocp_mreqinfo[4:0], y};
	      
	      always @*
	      begin
	      
	        case (p0_mreqinfo_o_temp[4:2])
		
	          000: ahb_hprot = 2'b11;    // OCP MReqInfo to AHB HPROT mapping
	          001: ahb_hprot = 2'b00;
	          010: ahb_hprot = 2'b00;
	          011: ahb_hprot = 2'b01;
	          100: ahb_hprot = 2'b00;
	          101: ahb_hprot = 2'b00;
	          110: ahb_hprot = 2'b10;
	          111: ahb_hprot = 2'b00;
		
	        endcase
	      
	      end
	      endmodule
	     
	     