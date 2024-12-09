
					
					  import subprocess
					  
					  def validate_ip(ip: str):
					  
					    split_ip = ip.split('.')
						if len(split_ip) > 4 or len(split_ip) == 0:
						
						  raise ValueError("Invalid IP length")
						
						
						for octet in split_ip:
						
						  try:
						  
						    int(octet, 10)
						  
						  except ValueError as e:
						  
						    raise ValueError(f"Cannot convert IP octet to int - {e}")
						  
						
						
						# Returns original IP after ensuring no exceptions are raised
						return ip

					  
					  
					  def run_ping(ip: str):
					  
					    validated = validate_ip(ip)
						# The ping command treats zero-prepended IP addresses as octal
						result = subprocess.call(["ping", validated])
						print(result)
					  
					
					
					