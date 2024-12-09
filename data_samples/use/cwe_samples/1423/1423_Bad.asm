
					
						
						adc edi,dword ptr [ebx+edx+13BE13BDh]
						adc dl,byte ptr [edi]
						...
						
					indirect_branch_site:
						
						jmp dword ptr [rsi]   # at this point attacker knows edx, controls edi and ebx
						
					
					