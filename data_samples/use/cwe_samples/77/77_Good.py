
				 
				   cweRegex = re.compile("^CWE-\d+$")
				   match1 = cweRegex.search(arg1)
				   match2 = cweRegex.search(arg2)
				   if match1 is None or match2 is None:
				   
					 # throw exception, generate error, etc.
				   
				   prompt = "Explain the difference between {} and {}".format(arg1, arg2)
				   ...
				 
			   
			