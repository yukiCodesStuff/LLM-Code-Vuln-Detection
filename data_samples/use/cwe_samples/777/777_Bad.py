
		
		  import subprocess
		  import re
		  
		  def validate_ip_regex(ip: str):
		  
		    ip_validator = re.compile(r"((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}")
		    if ip_validator.match(ip):
		    
		      return ip
		    
		    else:
		    
		      raise ValueError("IP address does not match valid pattern.")
		    
		  
		  
		  def run_ping_regex(ip: str):
		  
		    validated = validate_ip_regex(ip)
		    # The ping command treats zero-prepended IP addresses as octal
		    result = subprocess.call(["ping", validated])
		    print(result)
		  
		
	      
	      